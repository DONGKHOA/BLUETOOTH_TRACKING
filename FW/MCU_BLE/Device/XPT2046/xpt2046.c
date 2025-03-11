/******************************************************************************
 *      INCLUDES
 ******************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/gpio.h"

#include "xpt2046.h"

#include "spi.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "XPT2046"

#define CMD_X_READ  0XD0
#define CMD_Y_READ  0X90
#define CMD_Z1_READ 0b10110000
#define CMD_Z2_READ 0b11000000

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef enum
{
  TOUCH_NOT_DETECTED = 0,
  TOUCH_DETECTED     = 1,
} xpt2046_touch_detect_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void                   xpt2046_corr(int16_t *x, int16_t *y);
static void                   xpt2046_avg(int16_t *x, int16_t *y);
static int16_t                xpt2046_cmd(uint8_t cmd);
static xpt2046_touch_detect_t xpt2048_is_touch_detected();
static void tp_spi_read_reg(uint8_t reg, uint8_t *data, uint8_t byte_count);

/******************************************************************************
 *  PRIVATE DATA
 *****************************************************************************/

static xpt2046_handle_t xpt2046_handle;
static int16_t          avg_buf_x[XPT2046_AVG];
static int16_t          avg_buf_y[XPT2046_AVG];
static uint8_t          avg_last;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
DEV_XPT2046_Init (xpt2046_handle_t *p_xpt2046_handle)
{
  xpt2046_handle = *p_xpt2046_handle;

  gpio_config_t irq_config = {
    .pin_bit_mask = BIT64(xpt2046_handle.e_irq_pin),
    .mode         = GPIO_MODE_INPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_DISABLE,
  };

  esp_err_t ret = gpio_config(&irq_config);
}

bool
DEV_XPT2046_Read (lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  static int16_t last_x = 0;
  static int16_t last_y = 0;
  bool           valid  = false;

  int16_t x = last_x;
  int16_t y = last_y;
  if (xpt2048_is_touch_detected() == TOUCH_DETECTED)
  {
    valid = true;

    x = xpt2046_cmd(CMD_X_READ);
    y = xpt2046_cmd(CMD_Y_READ);
    ESP_LOGI(TAG, "P(%d,%d)", x, y);

    /*Normalize Data back to 12-bits*/
    x = x >> 4;
    y = y >> 4;
    ESP_LOGI(TAG, "P_norm(%d,%d)", x, y);

    xpt2046_corr(&x, &y);
    xpt2046_avg(&x, &y);
    last_x = x;
    last_y = y;

    ESP_LOGI(TAG, "x = %d, y = %d", x, y);
  }
  else
  {
    avg_last = 0;
  }

  data->point.x = x;
  data->point.y = y;
  data->state   = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;

  return false;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static xpt2046_touch_detect_t
xpt2048_is_touch_detected ()
{
  // check IRQ pin if we IRQ or IRQ and preessure
#if XPT2046_TOUCH_IRQ || XPT2046_TOUCH_IRQ_PRESS
  uint8_t irq = gpio_get_level(xpt2046_handle.e_irq_pin);

  if (irq != 0)
  {
    return TOUCH_NOT_DETECTED;
  }
#endif
  // check pressure if we are pressure or IRQ and pressure
#if XPT2046_TOUCH_PRESS || XPT2046_TOUCH_IRQ_PRESS
  int16_t z1 = xpt2046_cmd(CMD_Z1_READ) >> 3;
  int16_t z2 = xpt2046_cmd(CMD_Z2_READ) >> 3;

  // this is not what the confusing datasheet says but it seems to
  // be enough to detect real touches on the panel
  int16_t z = z1 + 4096 - z2;

  if (z < XPT2046_TOUCH_THRESHOLD)
  {
    return TOUCH_NOT_DETECTED;
  }
#endif

  return TOUCH_DETECTED;
}

static int16_t
xpt2046_cmd (uint8_t cmd)
{
  uint8_t data[2];
  tp_spi_read_reg(cmd, data, 2);
  int16_t val = (data[0] << 8) | data[1];
  return val;
}

static void
xpt2046_corr (int16_t *x, int16_t *y)
{
#if XPT2046_XY_SWAP != 0
  int16_t swap_tmp;
  swap_tmp = *x;
  *x       = *y;
  *y       = swap_tmp;
#endif

  if ((*x) > XPT2046_X_MIN)
  {
    (*x) -= XPT2046_X_MIN;
  }
  else
  {
    (*x) = 0;
  }

  if ((*y) > XPT2046_Y_MIN)
  {
    (*y) -= XPT2046_Y_MIN;
  }
  else
  {
    (*y) = 0;
  }

  (*x) = (uint32_t)((uint32_t)(*x) * LV_HOR_RES)
         / (XPT2046_X_MAX - XPT2046_X_MIN);

  (*y) = (uint32_t)((uint32_t)(*y) * LV_VER_RES)
         / (XPT2046_Y_MAX - XPT2046_Y_MIN);

#if XPT2046_X_INV != 0
  (*x) = LV_HOR_RES - (*x);
#endif

#if XPT2046_Y_INV != 0
  (*y) = LV_VER_RES - (*y);
#endif
}

static void
xpt2046_avg (int16_t *x, int16_t *y)
{
  /*Shift out the oldest data*/
  uint8_t i;
  for (i = XPT2046_AVG - 1; i > 0; i--)
  {
    avg_buf_x[i] = avg_buf_x[i - 1];
    avg_buf_y[i] = avg_buf_y[i - 1];
  }

  /*Insert the new point*/
  avg_buf_x[0] = *x;
  avg_buf_y[0] = *y;
  if (avg_last < XPT2046_AVG)
  {
    avg_last++;
  }

  /*Sum the x and y coordinates*/
  int32_t x_sum = 0;
  int32_t y_sum = 0;
  for (i = 0; i < avg_last; i++)
  {
    x_sum += avg_buf_x[i];
    y_sum += avg_buf_y[i];
  }

  /*Normalize the sums*/
  (*x) = (int32_t)x_sum / avg_last;
  (*y) = (int32_t)y_sum / avg_last;
}

static void
tp_spi_read_reg (uint8_t reg, uint8_t *data, uint8_t byte_count)
{
  spi_transaction_t t = { .length    = (byte_count + sizeof(reg)) * 8,
                          .rxlength  = byte_count * 8,
                          .cmd       = reg,
                          .rx_buffer = data,
                          .flags     = 0 };

  // Read - send first byte as command
  esp_err_t ret = spi_device_transmit(*xpt2046_handle.p_spi_Handle, &t);
  assert(ret == ESP_OK);
}