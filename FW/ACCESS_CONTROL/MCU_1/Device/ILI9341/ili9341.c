/******************************************************************************
 *      INCLUDES
 ******************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "ili9341.h"

#include "spi.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "ILI9341"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  uint8_t cmd;
  uint8_t data[16];
  uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end
                     // of cmds.
} lcd_init_cmd_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void ili9341_set_orientation(uint8_t orientation);

static void ili9341_send_cmd(uint8_t cmd);
static void ili9341_send_data(void *data, uint16_t length);
static void ili9341_send_color(void *data, uint16_t length);

/******************************************************************************
 *  PRIVATE DATA
 *****************************************************************************/

static ili9341_handle_t ili9341_handle;
static lv_disp_drv_t *LastDriver;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
DEV_ILI9341_Init (ili9341_handle_t *p_ili9341_handle)
{
	lcd_init_cmd_t ili_init_cmds[]={
		{0xCF, {0x00, 0x83, 0X30}, 3},
		{0xED, {0x64, 0x03, 0X12, 0X81}, 4},
		{0xE8, {0x85, 0x01, 0x79}, 3},
		{0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
		{0xF7, {0x20}, 1},
		{0xEA, {0x00, 0x00}, 2},
		{0xC0, {0x26}, 1},          /*Power control*/
		{0xC1, {0x11}, 1},          /*Power control */
		{0xC5, {0x35, 0x3E}, 2},    /*VCOM control*/
		{0xC7, {0xBE}, 1},          /*VCOM control*/
		{0x36, {0x28}, 1},          /*Memory Access Control*/
		{0x3A, {0x55}, 1},			/*Pixel Format Set*/
		{0xB1, {0x00, 0x1B}, 2},
		{0xF2, {0x08}, 1},
		{0x26, {0x01}, 1},
		{0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
		{0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
		{0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
		{0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
		{0x2C, {0}, 0},
		{0xB7, {0x07}, 1},
		{0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
		{0x11, {0}, 0x80},
		{0x29, {0}, 0x80},
		{0, {0}, 0xff},
	};

  ili9341_handle = *p_ili9341_handle;

  // Reset the display
  gpio_set_level(ili9341_handle.e_rst_pin, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(ili9341_handle.e_rst_pin, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "Initialization.");

  // Send all the commands
  uint16_t cmd = 0;
  while (ili_init_cmds[cmd].databytes != 0xff)
  {
    ili9341_send_cmd(ili_init_cmds[cmd].cmd);
    ili9341_send_data(ili_init_cmds[cmd].data,
                      ili_init_cmds[cmd].databytes & 0x1F);
    if (ili_init_cmds[cmd].databytes & 0x80)
    {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    cmd++;
  }

  ili9341_set_orientation(3);
  ili9341_send_cmd(0x20);
}

void
DEV_ILI9341_Flush (lv_disp_drv_t   *drv,
                   const lv_area_t *area,
                   lv_color_t      *color_map)
{
	LastDriver=drv;
  uint8_t data[4];

  /*Column addresses*/
  ili9341_send_cmd(0x2A);
  data[0] = (area->x1 >> 8) & 0xFF;
  data[1] = area->x1 & 0xFF;
  data[2] = (area->x2 >> 8) & 0xFF;
  data[3] = area->x2 & 0xFF;
  ili9341_send_data(data, 4);

  /*Page addresses*/
  ili9341_send_cmd(0x2B);
  data[0] = (area->y1 >> 8) & 0xFF;
  data[1] = area->y1 & 0xFF;
  data[2] = (area->y2 >> 8) & 0xFF;
  data[3] = area->y2 & 0xFF;
  ili9341_send_data(data, 4);

  /*Memory write*/
  ili9341_send_cmd(0x2C);

  uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
  ili9341_send_color((void *)color_map, size);
}

void
DEV_ILI9341_EnableBacklight (bool backlight)
{
}

void
DEV_ILI9341_SleepIn (void)
{
}

void
DEV_ILI9341_SleepOut (void)
{
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
ili9341_set_orientation (uint8_t orientation)
{
  const char *orientation_str[]
      = { "PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED" };

  ESP_LOGI(TAG, "Display orientation: %s", orientation_str[orientation]);
  uint8_t data[] = { 0x48, 0x88, 0x28, 0xE8 };

  ESP_LOGI(TAG, "0x36 command value: 0x%02X", data[orientation]);

  ili9341_send_cmd(0x36);
  ili9341_send_data((void *)&data[orientation], 1);
}

static void
ili9341_send_cmd (uint8_t cmd)
{
  gpio_set_level(ili9341_handle.e_dc_pin, 0); /*Command mode*/
  BSP_spiWriteByte(*ili9341_handle.p_spi_Handle, cmd);
}

static void
ili9341_send_data (void *data, uint16_t length)
{
  gpio_set_level(ili9341_handle.e_dc_pin, 1); /*Data mode*/
  BSP_spiWriteMultipleByte(*ili9341_handle.p_spi_Handle, data, length);
}

static void
ili9341_send_color (void *data, uint16_t length)
{
  gpio_set_level(ili9341_handle.e_dc_pin, 1); /* Data mode */

  static uint8_t Byte[12800 * 2];
  uint16_t *color_data = (uint16_t *)data; // Cast `void *` to `uint16_t *`
  uint16_t  index      = 0;
  for (uint16_t i = 0; i < length; i++)
  {
    Byte[index++] = color_data[i] & 0xFF;
    Byte[index++] = (color_data[i] >> 8) & 0xFF;
  }

  BSP_spiWriteMultipleByte(
      *ili9341_handle.p_spi_Handle, Byte, length * 2); // Send correct length

  lv_disp_flush_ready(LastDriver);
}
