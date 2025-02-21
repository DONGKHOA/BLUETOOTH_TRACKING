/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "dht22.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define DHT22_PIN GPIO_NUM_10

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

dht22_data_t dht22;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void DHT22_Task(void *pvParameters);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
DHT22_Task (void *pvParameters)
{
  DEV_DHT22_Init(&dht22, DHT22_PIN);
  while (1)
  {
    DEV_DHT22_GetData(&dht22, DHT22_PIN);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
