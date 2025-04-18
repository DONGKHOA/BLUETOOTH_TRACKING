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

  /**
   * @brief Initializes the DS3231 device.
   *
   * This function sets up the DS3231 real-time clock (RTC) device with the
   * specified data structure and GPIO pins for SDA and SCL.
   *
   * @param p_ds3231_data Pointer to the DS3231 data structure.
   * @param e_sda_io GPIO number for the SDA (data) line.
   * @param e_scl_io GPIO number for the SCL (clock) line.
   */
  void DEV_DS3231_Init(ds3231_data_t *p_ds3231_data, i2c_port_t e_i2c_port);

  /**
   * @brief Reads data from the DS3231 device and stores it in the provided data
   * structure.
   *
   * This function communicates with the DS3231 real-time clock (RTC) module to
   * read the current time and date information. The retrieved data is then
   * stored in the structure pointed to by `p_ds3231_data`.
   *
   * @param[in,out] p_ds3231_data Pointer to a ds3231_data_t structure where the
   * read data will be stored.
   *
   * @return
   *     - ESP_OK: Success
   *     - ESP_ERR_INVALID_ARG: Invalid argument
   *     - ESP_FAIL: Communication with the DS3231 failed
   */
  esp_err_t DEV_DS3231_Register_Read(ds3231_data_t *p_ds3231_data,
                                     i2c_port_t     e_i2c_port);

  /**
   * @brief Writes data to the DS3231 device register.
   *
   * This function writes the provided data to the DS3231 device register.
   *
   * @param[in] p_ds3231_data Pointer to the structure containing the data to be
   * written to the DS3231 register.
   *
   * @return
   *     - ESP_OK: Success
   *     - ESP_ERR_INVALID_ARG: Invalid argument
   *     - ESP_FAIL: Write operation failed
   */
  esp_err_t DEV_DS3231_Register_Write(ds3231_data_t *p_ds3231_data,
                                      i2c_port_t     e_i2c_port);

#ifdef __cplusplus
}
#endif

#endif /* DS3231_H*/