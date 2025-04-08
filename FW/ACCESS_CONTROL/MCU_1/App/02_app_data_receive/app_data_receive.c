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
typedef struct
{
  QueueHandle_t *p_receive_data_event_queue;
} data_receive_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_DATA_RECEIVE_Task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static data_receive_data_t s_data_receive_data;
static uint16_t            u16_user_len = 0;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_DATA_RECEIVE_CreateTask (void)
{
  xTaskCreate(
      APP_DATA_RECEIVE_Task, "data receive task", 1024 * 4, NULL, 13, NULL);
}
void
APP_DATA_RECEIVE_Init (void)
{
  s_data_receive_data.p_receive_data_event_queue
      = &s_data_system.s_receive_data_event_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_DATA_RECEIVE_Task (void *arg)
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
      case DATA_SYNC_RESPONSE_ENROLL_FACE:

        s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
        s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
        s_DATA_SYNC.u8_data_length    = s_receive_message.data[2];
        s_DATA_SYNC.u8_data_stop      = s_receive_message.data[3];

        break;

      case DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT:

        s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
        s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
        s_DATA_SYNC.u8_data_length    = s_receive_message.data[2];
        s_DATA_SYNC.u8_data_stop      = s_receive_message.data[3];

        break;

      case DATA_SYNC_RESPONSE_AUTHENTICATION:

        s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
        s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
        s_DATA_SYNC.u8_data_length    = s_receive_message.data[2];
        s_DATA_SYNC.u8_data_stop      = s_receive_message.data[3];

        break;

      case DATA_SYNC_STATE_CONNECTION:

        s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
        s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
        s_DATA_SYNC.u8_data_length    = s_receive_message.data[2];
        s_DATA_SYNC.u8_data_stop      = s_receive_message.data[3];

        break;

      case DATA_SYNC_RESPONSE_ATTENDANCE:

        s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
        s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
        s_DATA_SYNC.u8_data_length    = s_receive_message.data[2];
        s_DATA_SYNC.u8_data_stop      = s_receive_message.data[3];

        break;

      // case DATA_SYNC_NUMBER_OF_USER_DATA:

      //   s_DATA_SYNC.u8_data_start     = s_receive_message.data[0];
      //   s_DATA_SYNC.u8_data_packet[0] = s_receive_message.data[1];
      //   s_DATA_SYNC.u8_data_packet[1] = s_receive_message.data[2];
      //   s_DATA_SYNC.u8_data_length    = s_receive_message.data[3];
      //   s_DATA_SYNC.u8_data_stop      = s_receive_message.data[4];

      //   u16_user_len = (s_DATA_SYNC.u8_data_packet[0] << 8)
      //                  | s_DATA_SYNC.u8_data_packet[1];

      //   xQueueSend(
      //       *s_data_receive_data.p_receive_data_event_queue, &s_DATA_SYNC, 0);

      //   break;

      // case DATA_SYNC_DETAIL_OF_USER_DATA:

      //   memset(&s_DATA_SYNC, 0, sizeof(s_DATA_SYNC));

      //   s_DATA_SYNC.u8_data_start = s_receive_message.data[0];

      //   int payload_len = s_receive_message.data_length_code - 3;
      //   for (int i = 0; i < payload_len; i++)
      //   {
      //     s_DATA_SYNC.u8_data_packet[i] = s_receive_message.data[i + 1];
      //   }

      //   s_DATA_SYNC.u8_data_length = s_receive_message.data[payload_len + 1];
      //   s_DATA_SYNC.u8_data_stop   = s_receive_message.data[payload_len + 2];

      //   xQueueSend(
      //       *s_data_receive_data.p_receive_data_event_queue, &s_DATA_SYNC, 0);

        break;

      default:
        printf("Other state\r\n");
        break;
    }
  }
}