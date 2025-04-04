/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>
#include <stdio.h>

#include "app_data_receive.h"
#include "app_data.h"

#include "environment.h"

#include "can.h"
#include "sn65hvd230dr.h"
#include "esp_log.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_DATA_RECEIVE"

#define CAN_ID   0x123
#define CAN_EXTD 0
#define CAN_RTR  0

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

/**
 * @brief Structure defining the data of app_data_receive.
 */
typedef struct ble_ibeacon_data
{
  QueueHandle_t *p_data_mqtt_queue;
} data_receive_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_DATA_RECEIVE_task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static data_receive_data_t s_data_receive_data;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_DATA_RECEIVE_CreateTask (void)
{
  xTaskCreate(
      APP_DATA_RECEIVE_task, "APP_DATA_RECEIVE_task", 1024 * 2, NULL, 5, NULL);
}
void
APP_DATA_RECEIVE_Init (void)
{
  s_data_receive_data.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_DATA_RECEIVE_task (void *arg)
{
  twai_message_t s_receive_message;
  DATA_SYNC_t    s_DATA_SYNC;

  while (1)
  {
    if (BSP_canReceive(&s_receive_message, pdMS_TO_TICKS(50)) != ESP_OK)
    {
      continue;
    }

    if (s_receive_message.identifier != CAN_ID)
    {
      continue;
    }

    switch (s_receive_message.data[0])
    {
      case DATA_SYNC_ENROLL_FINGERPRINT:

        s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
        s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
        s_DATA_SYNC.u8_data_length    = s_receive_message.data[2];
        s_DATA_SYNC.u8_data_stop      = s_receive_message.data[3];
        
      case DATA_SYNC_REQUEST_USER_DATA:
      case DATA_SYNC_ENROLL_FACE:
      case DATA_SYNC_REQUEST_AUTHENTICATION:
      case DATA_SYNC_REQUEST_ATTENDANCE:

        s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
        s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
        s_DATA_SYNC.u8_data_length    = s_receive_message.data[2];
        s_DATA_SYNC.u8_data_stop      = s_receive_message.data[3];

        xQueueSend(*s_data_receive_data.p_data_mqtt_queue, &s_DATA_SYNC, 0);

        break;

      default:
        printf("Other state\r\n");
        break;
    }
  }
}