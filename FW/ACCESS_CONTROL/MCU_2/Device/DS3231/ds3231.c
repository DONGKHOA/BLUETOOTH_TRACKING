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
  { // Cehck bit OSF (bit 7)
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

// esp_err_t
// DEV_DS3231_Register_Read (ds3231_data_t *p_ds3231_data, i2c_port_t e_i2c_port)
// {
//   uint8_t reg_addr = SECOND_VALUE_ADDRESS;
//   int8_t  ret
//       = BSP_i2cWriteReadBuffer(e_i2c_port,
//                                DS3231_ADDRESS,
//                                &reg_addr,
//                                1,
//                                (uint8_t *)p_ds3231_data,
//                                7,
//                                1000 / portTICK_PERIOD_MS);

//   for (uint8_t i = 0; i < sizeof(ds3231_data_t); i++)
//   {
//     *((uint8_t *)p_ds3231_data + i)
//         = BCD_To_DEC(*((uint8_t *)p_ds3231_data + i));
//   }
//   return ret;
// }

// esp_err_t
// DEV_DS3231_Register_Write (ds3231_data_t *p_ds3231_data, i2c_port_t e_i2c_port)
// {
//   int     ret;
//   uint8_t tx_buf[8] = {};
//   for (uint8_t i = 0; i < sizeof(ds3231_data_t); i++)
//   {
//     tx_buf[i + 1] = DEC_To_BCD(*((uint8_t *)p_ds3231_data + i));
//   }
//   tx_buf[0] = SECOND_VALUE_ADDRESS;
//   ret       = BSP_i2cWriteBuffer(e_i2c_port,
//                            DS3231_ADDRESS,
//                            tx_buf,
//                            sizeof(tx_buf),
//                            1000 / portTICK_PERIOD_MS);

//   return ret;
// }
esp_err_t DEV_DS3231_Register_Read(ds3231_data_t *p, i2c_port_t port)
{
  uint8_t reg_addr = SECOND_VALUE_ADDRESS;
  uint8_t raw[7];

  esp_err_t ret = BSP_i2cWriteReadBuffer(port, DS3231_ADDRESS,
                                         &reg_addr, 1,
                                         raw, 7,
                                         1000 / portTICK_PERIOD_MS);

  if (ret != ESP_OK) return ret;

  p->u8_second = BCD_To_DEC(raw[0]);
  p->u8_minute = BCD_To_DEC(raw[1]);
  p->u8_hour   = BCD_To_DEC(raw[2]);
  p->u8_day    = BCD_To_DEC(raw[3]);
  p->u8_date   = BCD_To_DEC(raw[4]);
  p->u8_month  = BCD_To_DEC(raw[5] & 0x1F); // mask bit 7 (century)
  p->u8_year   = BCD_To_DEC(raw[6]);

  return ESP_OK;
}

esp_err_t DEV_DS3231_Register_Write(ds3231_data_t *p, i2c_port_t port)
{
  uint8_t tx_buf[8];
  tx_buf[0] = SECOND_VALUE_ADDRESS;
  tx_buf[1] = DEC_To_BCD(p->u8_second);
  tx_buf[2] = DEC_To_BCD(p->u8_minute);
  tx_buf[3] = DEC_To_BCD(p->u8_hour);
  tx_buf[4] = DEC_To_BCD(p->u8_day);
  tx_buf[5] = DEC_To_BCD(p->u8_date);
  tx_buf[6] = DEC_To_BCD(p->u8_month);
  tx_buf[7] = DEC_To_BCD(p->u8_year);

  return BSP_i2cWriteBuffer(port, DS3231_ADDRESS,
                            tx_buf, sizeof(tx_buf),
                            1000 / portTICK_PERIOD_MS);
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