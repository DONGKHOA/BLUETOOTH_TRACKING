#ifndef DHT22_H
#define DHT22_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

#include "gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef enum
  {
    DHT_CRC_ERROR = -1,
    DHT_TIMEOUT_ERROR,
    DHT_OK
  } dht22_status_t;

  typedef struct
  {
    float temperature;
    float humidity;
  } dht22_data_t;

  /****************************************************************************
   *   PUBLIC FUNCTIONS
   ***************************************************************************/

  /**
   * @brief Retrieves data from the DHT22 sensor.
   *
   * This function reads temperature and humidity data from the DHT22 sensor
   * connected to the specified GPIO pin and stores the data in the provided
   * dht22_data_t structure.
   *
   * @param dht22_data Pointer to a dht22_data_t structure where the sensor
   * data will be stored.
   * @param e_gpio_pin The GPIO pin number to which the DHT22 sensor is
   * connected.
   *
   * @return Returns DHT_OK on success, or an error code on
   * failure.
   */
  uint8_t DEV_DHT22_GetData(dht22_data_t *dht22_data, gpio_num_t e_gpio_pin);

#ifdef __cplusplus
}
#endif

#endif /* DHT22_H*/