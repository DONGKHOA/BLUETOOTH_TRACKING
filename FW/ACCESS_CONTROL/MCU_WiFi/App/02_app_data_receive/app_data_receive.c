/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "esp_log.h"

#include "app_data_receive.h"
#include "app_data.h"

#include "can.h"
#include "sn65hvd230dr.h"

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
  QueueHandle_t           *p_data_mqtt_queue;
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
  xTaskCreate(APP_DATA_RECEIVE_task, "APP_DATA_RECEIVE_task", 1024 * 2, NULL, 5, NULL);
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
  twai_message_t     receive_message;
  twai_status_info_t status;
  while (1)
  {
    BSP_canGetStatus(&status);
    if (status.msgs_to_rx > 0)
    {
      BSP_canReceive(&receive_message, portMAX_DELAY);
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