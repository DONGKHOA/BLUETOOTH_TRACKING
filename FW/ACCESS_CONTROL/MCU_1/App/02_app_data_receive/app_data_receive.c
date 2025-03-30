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
  QueueHandle_t *p_display_data_queue;
} data_receive_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_DATA_RECEIVE_Task(void *arg);

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
      APP_DATA_RECEIVE_Task, "data receive task", 1024 * 2, NULL, 13, NULL);
}
void
APP_DATA_RECEIVE_Init (void)
{
  s_data_receive_data.p_display_data_queue
      = &s_data_system.s_display_data_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_DATA_RECEIVE_Task (void *arg)
{
  twai_message_t s_receive_message;
  while (1)
  {
    BSP_canReceive(&s_receive_message, pdMS_TO_TICKS(50));

    switch (s_receive_message.data[0])
    {
      case DATA_SYNC_RESPONSE_ENROLL_FACE:

        break;

      case DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT:

        break;

      case DATA_SYNC_RESPONSE_AUTHENTICATION:

        break;

      case DATA_SYNC_STATE_CONNECTION:

        break;

      case DATA_SYNC_RESPONSE_ATTENDANCE:

        break;

      default:
        break;
    }

    for (uint16_t i = 1; i < s_receive_message.data_length_code; i++)
    {
    }
  }
}