/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "ds3231.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define I2C_MASTER_NUM        0
#define I2C_MASTER_FREQ_HZ    400000 /*!< I2C master clock frequency */
#define I2C_MASTER_TIMEOUT_MS 1000

#define DS3231_ADDRESS       0x68
#define SECOND_VALUE_ADDRESS 0x00
#define DS3231_STATUS        0x0F

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static uint8_t DEC_To_BCD(uint8_t u8_DEC_value);
static uint8_t BCD_To_DEC(uint8_t u8_BCD_value);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
DEV_DS3231_Init (ds3231_data_t *p_ds3231_data,
                 gpio_num_t     e_sda_io,
                 gpio_num_t     e_scl_io)
{
  BSP_i2cDriverInit(I2C_MASTER_NUM,
                    I2C_MODE_MASTER,
                    e_sda_io,
                    e_scl_io,
                    GPIO_PULLUP_ENABLE,
                    GPIO_PULLUP_ENABLE,
                    I2C_MASTER_FREQ_HZ);

  uint8_t status_reg;
  uint8_t reg_addr = DS3231_STATUS;
  BSP_i2cWriteReadToDevice(I2C_MASTER_NUM,
                           DS3231_ADDRESS,
                           &reg_addr,
                           1,
                           &status_reg,
                           1,
                           I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

  if (status_reg & (1 << 7))
  { // Kiểm tra bit OSF (bit 7)
    printf("Oscillator stopped. Initializing RTC...\n");
    // Reset bit OSF
    status_reg &= ~(1 << 7);

    *p_ds3231_data = (ds3231_data_t) { .u8_hour   = 15,
                                       .u8_minute = 44,
                                       .u8_second = 0,
                                       .u8_day    = 1,
                                       .u8_date   = 7,
                                       .u8_month  = 2,
                                       .u8_year   = 25 };

    DEV_DS3231_Register_Write(p_ds3231_data);

    // Xóa bit OSF
    status_reg &= ~(1 << 7);
    reg_addr = DS3231_STATUS;
    BSP_i2cWriteToDevice(I2C_MASTER_NUM,
                         DS3231_ADDRESS,
                         (uint8_t[]) { reg_addr, status_reg },
                         2,
                         I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  }
  else
  {
    printf("RTC is running. Reading current data...\n");

    // Đọc thời gian hiện tại từ DS3231
    DEV_DS3231_Register_Read(p_ds3231_data);
  }
}

esp_err_t
DEV_DS3231_Register_Read (ds3231_data_t *p_ds3231_data)
{
  uint8_t reg_addr = SECOND_VALUE_ADDRESS;
  int8_t  ret
      = BSP_i2cWriteReadToDevice(I2C_MASTER_NUM,
                                 DS3231_ADDRESS,
                                 &reg_addr,
                                 1,
                                 (uint8_t *)p_ds3231_data,
                                 7,
                                 I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

  for (uint8_t i = 0; i < sizeof(ds3231_data_t); i++)
  {
    *((uint8_t *)p_ds3231_data + i)
        = BCD_To_DEC(*((uint8_t *)p_ds3231_data + i));
  }
  return ret;
}

esp_err_t
DEV_DS3231_Register_Write (ds3231_data_t *p_ds3231_data)
{
  int     ret;
  uint8_t tx_buf[8] = {};
  for (uint8_t i = 0; i < sizeof(ds3231_data_t); i++)
  {
    tx_buf[i + 1] = DEC_To_BCD(*((uint8_t *)p_ds3231_data + i));
  }
  tx_buf[0] = SECOND_VALUE_ADDRESS;
  ret       = BSP_i2cWriteToDevice(I2C_MASTER_NUM,
                             DS3231_ADDRESS,
                             tx_buf,
                             sizeof(tx_buf),
                             I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

  return ret;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static uint8_t
BCD_To_DEC (uint8_t u8_BCD_value)
{
  return ((u8_BCD_value >> 4) * 10) + (u8_BCD_value & 0x0F);
}

static uint8_t
DEC_To_BCD (uint8_t u8_DEC_value)
{
  return ((u8_DEC_value / 10) << 4) | (u8_DEC_value % 10);
}