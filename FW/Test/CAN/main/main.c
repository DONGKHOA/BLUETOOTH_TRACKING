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

#include "sn65hvd230dr.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define CAN_MODE      TWAI_MODE_NORMAL
#define TXD_PIN       GPIO_NUM_19
#define RXD_PIN       GPIO_NUM_20
#define TXD_QUEUE_LEN 1024
#define RXD_QUEUE_LEN 1024
#define INTR_FLAG     ESP_INTR_FLAG_LEVEL3 // lowest priority
#define CAN_BITRATE   5

#define CAN_ID   0x123
#define CAN_EXTD 0
#define CAN_RTR  0

/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

char *p_text = "ESP2CAN";

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void TestMain_CAN_ReceiveMessage_Task(void *pvParameters);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  // Config parameter of CAN protocol
  BSP_canConfigDefault();
  BSP_canConfigMode(CAN_MODE);
  BSP_canConfigIO(TXD_PIN, RXD_PIN);
  BSP_canConfigQueue(TXD_QUEUE_LEN, RXD_QUEUE_LEN);
  BSP_canConfigIntr(INTR_FLAG);
  BSP_canConfigBitRate(CAN_BITRATE);

  // Install TWAI driver
  BSP_canDriverInit();

  // Start TWAI driver
  BSP_canStart();

  // Create Task to receive data
  xTaskCreate(TestMain_CAN_ReceiveMessage_Task,
              "TestMain_CAN_ReceiveMessage_Task",
              4096,
              NULL,
              10,
              NULL);
  while (1)
  {
    // Send Message
    DEV_CAN_SendMessage(
        CAN_ID, CAN_EXTD, CAN_RTR, (uint8_t *)p_text, strlen(p_text));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
TestMain_CAN_ReceiveMessage_Task (void *pvParameters)
{
  twai_message_t     receive_message;
  twai_status_info_t status;
  while (1)
  {
    BSP_canGetStatus(&status);
    if (status.msgs_to_rx > 0)
    {
      BSP_canReceive(&receive_message, portMax_DELAY);
      if (receive_message.extd)
      {
        printf("Message is in Extended Format\n");
      }
      else
      {
        printf("Message is in Standard Format\n");
      }

      ESP_LOGI("CAN", "ID is %lX", receive_message.identifier);
      if (!(receive_message.rtr))
      {
        for (int i = 0; i < receive_message.data_length_code; i++)
        {
          ESP_LOGI("CAN", "Data byte %d = %X", i, receive_message.data[i]);
        }
      }
    }
  }
}