/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "dht22.h"

#include "sdkconfig.h"
/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

static const char *DHT22_TAG = "DHT22";

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static uint8_t DEV_DHT22_Send_Start_Signal(gpio_num_t e_gpio_pin);
static uint8_t DEV_DHT22_Read_Byte(gpio_num_t e_gpio_pin, uint8_t *value);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/
esp_err_t
DEV_DHT22_Init (dht22_data_t *p_dht22_data, gpio_num_t e_gpio_pin)
{
  esp_rom_gpio_pad_select_gpio(e_gpio_pin);
  return ESP_OK;
}

uint8_t
DEV_DHT22_GetData (dht22_data_t *dht22_data, gpio_num_t e_gpio_pin)
{
  uint8_t buf[5] = { 0 };
  uint8_t checksum;

  if (DEV_DHT22_Send_Start_Signal(e_gpio_pin) == DHT_TIMEOUT_ERROR)
  {
    ESP_LOGE(DHT22_TAG, "No response from DHT22!");
    return DHT_TIMEOUT_ERROR;
  }

  if ((DEV_DHT22_Read_Byte(e_gpio_pin, &buf[0])
       || DEV_DHT22_Read_Byte(e_gpio_pin, &buf[1])
       || DEV_DHT22_Read_Byte(e_gpio_pin, &buf[2])
       || DEV_DHT22_Read_Byte(e_gpio_pin, &buf[3])
       || DEV_DHT22_Read_Byte(e_gpio_pin, &checksum))
      == DHT_TIMEOUT_ERROR)
  {
    ESP_LOGE(DHT22_TAG, "Failed to read data from DHT22!");
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
static uint8_t
DEV_DHT22_Send_Start_Signal (gpio_num_t e_gpio_pin)
{
  BSP_gpioSetDirection(e_gpio_pin, GPIO_MODE_OUTPUT, GPIO_PULL_UP);
  BSP_gpioSetState(e_gpio_pin, 0); // pull down

  ets_delay_us(1000); // MCU sends out start signal and pulls down voltage for
  //                        // at least 1ms to let DHT22 detect the signal
  BSP_gpioSetState(e_gpio_pin, 1); // pull up
  ets_delay_us(30); // MCU pulls up voltage and waits for DHT response

  BSP_gpioSetDirection(e_gpio_pin, GPIO_MODE_INPUT, GPIO_NO_PULL);

  // CHECK RESPONSE
  int64_t GetTick_us = esp_timer_get_time();
  while (!BSP_gpioGetState(
      e_gpio_pin)) // DHT sends out response signal & keeps it for 80us
  {
    if (esp_timer_get_time() - GetTick_us > 250)
    {
      return DHT_TIMEOUT_ERROR;
    }
  }

  GetTick_us = esp_timer_get_time();
  while (BSP_gpioGetState(e_gpio_pin)) // DHT pulls up voltage and keeps it for
                                       // 80us
  {
    if (esp_timer_get_time() - GetTick_us > 250)
    {
      return DHT_TIMEOUT_ERROR;
    }
  }

  return DHT_OK;
}

static uint8_t
DEV_DHT22_Read_Byte (gpio_num_t e_gpio_pin, uint8_t *value)
{
  *value = 0;
  for (int i = 8; i > 0; i--)
  {
    *value <<= 1;

    int64_t timestamp_us = esp_timer_get_time();
    while (BSP_gpioGetState(e_gpio_pin) == 0) // Start of bit (50µs low)
    {
      if (esp_timer_get_time() - timestamp_us > 80) // timeout 80µs
      {
        return DHT_TIMEOUT_ERROR;
      }
    }

    timestamp_us = esp_timer_get_time();
    while (BSP_gpioGetState(e_gpio_pin) == 1) // Wait status HIGH (26-70µs)
    {
      if (esp_timer_get_time() - timestamp_us > 80)
      {
        return DHT_TIMEOUT_ERROR;
      }
    }

    // Check if the HIGH time is longer than 50µs => Bit 1, otherwise Bit 0
    if ((esp_timer_get_time() - timestamp_us) > 50)
    {
      *value |= 1;
    }
  }
  return DHT_OK;
}