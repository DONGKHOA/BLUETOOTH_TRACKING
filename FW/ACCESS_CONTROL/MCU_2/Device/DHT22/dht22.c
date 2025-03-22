/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "rom/ets_sys.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "dht22.h"

#include "sdkconfig.h"
/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

#define DHT22_TAG "DHT22"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void    DEV_DHT22_SendStartSignal(gpio_num_t e_gpio_pin);
static uint8_t DEV_DHT22_CheckResponse(gpio_num_t e_gpio_pin);
static uint8_t DEV_DHT22_ReadByte(gpio_num_t e_gpio_pin);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

uint8_t
DEV_DHT22_GetData (dht22_data_t *dht22_data, gpio_num_t e_gpio_pin)
{
  uint8_t buf[4] = { 0 };
  uint8_t checksum;

  DEV_DHT22_SendStartSignal(e_gpio_pin);
  DEV_DHT22_CheckResponse(e_gpio_pin);

  buf[0]   = DEV_DHT22_ReadByte(e_gpio_pin);
  buf[1]   = DEV_DHT22_ReadByte(e_gpio_pin);
  buf[2]   = DEV_DHT22_ReadByte(e_gpio_pin);
  buf[3]   = DEV_DHT22_ReadByte(e_gpio_pin);
  checksum = DEV_DHT22_ReadByte(e_gpio_pin);

  if ((buf[0] || buf[1] || buf[2] || buf[3] || checksum) == DHT_TIMEOUT_ERROR)
  {
    ESP_LOGE(DHT22_TAG, "Failed to read data from DHT22 (Timeout)!");
    return DHT_TIMEOUT_ERROR;
  }

  // Data format: 8-bit integral RH data + 8-bit decimal RH data + 8-bit
  // integral T data + 8-bit decimal T data + 8-bit checksum. If the data
  // transmission is correct, the checksum should be the last 8 bits of "8-bit
  // integral RH data + 8-bit decimal RH data + 8-bit integral T data + 8-bit
  // decimal T data".

  // Check checksum
  if (checksum != ((buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF))
  {
    ESP_LOGE(DHT22_TAG, "Data from DHT22 is invalid (Checksum Error)!");
    return DHT_CRC_ERROR;
  }

  // Convert humidity value from 16-bit data
  uint16_t raw_humidity = (buf[0] << 8) | buf[1];
  dht22_data->humidity  = (float)raw_humidity / 10.0; // Unit: %

  // Convert temperature value from 16-bit data
  uint16_t raw_temperature = (buf[2] << 8) | buf[3];

  // Check if the temperature value is negative (MSB = 1)
  if (raw_temperature & 0x8000)
  {
    raw_temperature &= 0x7FFF; // Clear negative bit
    dht22_data->temperature
        = -((float)raw_temperature / 10.0); // Negative temperature
  }
  else
  {
    dht22_data->temperature = (float)raw_temperature / 10.0;
  }

  // Log the result
  ESP_LOGI(DHT22_TAG,
           "Temperature: %.1f°C, Humidity: %.1f%%",
           dht22_data->temperature,
           dht22_data->humidity);

  return DHT_OK;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/
static void
DEV_DHT22_SendStartSignal (gpio_num_t e_gpio_pin)
{
  BSP_gpioSetDirection(e_gpio_pin, GPIO_MODE_OUTPUT, GPIO_PULL_UP);
  BSP_gpioSetState(e_gpio_pin, 0); // pull down

  ets_delay_us(1200); // MCU sends out start signal and pulls down voltage

  BSP_gpioSetState(e_gpio_pin, 1); // pull up
  ets_delay_us(30); // MCU pulls up voltage and waits for DHT response

  BSP_gpioSetDirection(e_gpio_pin, GPIO_MODE_INPUT, GPIO_NO_PULL);
}

static uint8_t
DEV_DHT22_CheckResponse (gpio_num_t e_gpio_pin)
{
  uint8_t response = 0;
  ets_delay_us(40);

  if (!BSP_gpioGetState(e_gpio_pin)) // If the pin is low
  {
    ets_delay_us(80); // wait for 80us

    if (BSP_gpioGetState(e_gpio_pin))
    {
      response = 1;
    }
  }

  int64_t timestamp_us = esp_timer_get_time();
  while (BSP_gpioGetState(e_gpio_pin)) // wait for the pin to go low
  {
    if (esp_timer_get_time() - timestamp_us > 80)
    {
      return DHT_TIMEOUT_ERROR;
    }
  }

  return response;
}

static uint8_t
DEV_DHT22_ReadByte (gpio_num_t e_gpio_pin)
{
  uint8_t i            = 0;
  uint8_t j            = 0;
  int64_t timestamp_us = 0;

  for (j = 0; j < 8; j++)
  {
    timestamp_us = esp_timer_get_time();
    while (!BSP_gpioGetState(e_gpio_pin)) // wait for the pin to go high
    {
      if (esp_timer_get_time() - timestamp_us > 250)
      {
        return DHT_TIMEOUT_ERROR;
      }
    }

    // wait for 40 us
    ets_delay_us(40);

    if (!BSP_gpioGetState(e_gpio_pin)) // if the pin is low
    {
      i &= ~(1 << (7 - j)); // write 0
    }
    else
    {
      i |= (1 << (7 - j)); // if the pin is high, write 1
    }

    timestamp_us = esp_timer_get_time();
    while (BSP_gpioGetState(e_gpio_pin)) // wait for the pin to go low
    {
      if (esp_timer_get_time() - timestamp_us > 250)
      {
        return DHT_TIMEOUT_ERROR;
      }
    }
  }
  return i;
}