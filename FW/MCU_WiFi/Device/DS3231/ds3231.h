#ifndef DS3231_H
#define DS3231_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"
#include "i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct
  {
    uint8_t u8_second;
    uint8_t u8_minute;
    uint8_t u8_hour;
    uint8_t u8_day;
    uint8_t u8_date;
    uint8_t u8_month;
    uint8_t u8_year;
  } __attribute__((packed)) ds3231_data_t;

  /****************************************************************************
   *   PUBLIC FUNCTIONS
   ***************************************************************************/

  void      DEV_DS3231_Init(ds3231_data_t *p_ds3231_data,
                            gpio_num_t     e_sda_io,
                            gpio_num_t     e_scl_io);
  esp_err_t DEV_DS3231_Register_Read(ds3231_data_t *p_ds3231_data);
  esp_err_t DEV_DS3231_Register_Write(ds3231_data_t *p_ds3231_data);

#ifdef __cplusplus
}
#endif

#endif /* DS3231_H*/