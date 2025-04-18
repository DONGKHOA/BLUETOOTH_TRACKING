/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "dht22.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define DHT22_PIN GPIO_NUM_18

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

dht22_data_t dht22;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void TestMain_DHT22_Task(void *pvParameters);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  xTaskCreate(TestMain_DHT22_Task, "TestMain_DHT22_Task", 4096, NULL, 10, NULL);
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
TestMain_DHT22_Task (void *pvParameters)
{
  while (1)
  {
    DEV_DHT22_GetData(&dht22, DHT22_PIN);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
