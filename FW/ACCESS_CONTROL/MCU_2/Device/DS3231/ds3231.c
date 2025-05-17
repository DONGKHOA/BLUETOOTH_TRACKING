/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>

#include "ds3231.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

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
DEV_DS3231_Init (ds3231_data_t *p_ds3231_data, i2c_port_t e_i2c_port)
{
  uint8_t status_reg;
  uint8_t reg_addr = DS3231_STATUS;
  BSP_i2cWriteReadBuffer(e_i2c_port,
                         DS3231_ADDRESS,
                         &reg_addr,
                         1,
                         &status_reg,
                         1,
                         1000 / portTICK_PERIOD_MS);

  if (status_reg & (1 << 7))
  { // Check bit OSF (bit 7)
    printf("Oscillator stopped. Initializing RTC...\n");
    // Reset bit OSF
    status_reg &= ~(1 << 7);

  *p_ds3231_data = (ds3231_data_t) { .u8_hour   = 21,
                                     .u8_minute = 40,
                                     .u8_second = 0,
                                     .u8_day    = 4,
                                     .u8_date   = 1,
                                     .u8_month  = 5,
                                     .u8_year   = 125 };

  DEV_DS3231_Register_Write(p_ds3231_data, e_i2c_port);

  // Clear bit OSF
  status_reg &= ~(1 << 7);
  reg_addr = DS3231_STATUS;
  BSP_i2cWriteBuffer(e_i2c_port,
                     DS3231_ADDRESS,
                     (uint8_t[]) { reg_addr, status_reg },
                     2,
                     1000 / portTICK_PERIOD_MS);
  }
  else
  {
    printf("RTC is running. Reading current data...\n");

    // Read the current time of DS3231
    DEV_DS3231_Register_Read(p_ds3231_data, e_i2c_port);
  }
}

esp_err_t
DEV_DS3231_Register_Read (ds3231_data_t *p_ds3231_data, i2c_port_t e_i2c_port)
{
  uint8_t reg_addr = SECOND_VALUE_ADDRESS;
  int8_t  ret      = BSP_i2cWriteReadBuffer(e_i2c_port,
                                      DS3231_ADDRESS,
                                      &reg_addr,
                                      1,
                                      (uint8_t *)p_ds3231_data,
                                      7,
                                      1000 / portTICK_PERIOD_MS);

  for (uint8_t i = 0; i < sizeof(ds3231_data_t); i++)
  {
    *((uint8_t *)p_ds3231_data + i)
        = BCD_To_DEC(*((uint8_t *)p_ds3231_data + i));
  }
  return ret;
}

esp_err_t
DEV_DS3231_Register_Write (ds3231_data_t *p_ds3231_data, i2c_port_t e_i2c_port)
{
  int     ret;
  uint8_t tx_buf[8] = {};
  for (uint8_t i = 0; i < sizeof(ds3231_data_t); i++)
  {
    tx_buf[i + 1] = DEC_To_BCD(*((uint8_t *)p_ds3231_data + i));
  }
  tx_buf[0] = SECOND_VALUE_ADDRESS;
  ret       = i2c_master_write_to_device(e_i2c_port,
                                   DS3231_ADDRESS,
                                   tx_buf,
                                   sizeof(tx_buf),
                                   1000 / portTICK_PERIOD_MS);

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