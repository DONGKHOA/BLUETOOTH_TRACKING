/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "app_ble_tracking.h"
#include "app_data.h"

#include "wifi_helper.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_BLE_TRACKING"

#define GATEWAY_ID      0x01020304
#define GATEWAY_VERSION "1.0.0"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

/**
 * @brief Structure defining the data of app_ble_tracking.
 */
typedef struct ble_ibeacon_data
{
  QueueHandle_t *p_rssi_ibeacon_queue;
  QueueHandle_t *p_location_tag_queue;
  QueueHandle_t *p_addr_tag_queue;
} ble_tracking_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_BLE_TRACKING_Task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static ble_tracking_data_t s_ble_tracking_data;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_BLE_TRACKING_CreateTask (void)
{
  xTaskCreate(
      APP_BLE_TRACKING_Task, "ble tracking task", 1024 * 2, NULL, 11, NULL);
}

void
APP_BLE_TRACKING_Init (void)
{
  s_ble_tracking_data.p_addr_tag_queue = &s_data_system.s_addr_tag_queue;

  s_ble_tracking_data.p_rssi_ibeacon_queue
      = &s_data_system.s_rssi_ibeacon_queue;

  s_ble_tracking_data.p_location_tag_queue
      = &s_data_system.s_location_tag_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_BLE_TRACKING_Task (void *arg)
{
  ibeacon_infor_tag_t  s_beacon_infor_tag;
  tracking_infor_tag_t tracking_infor_tag;
  addr_tag_t           addr_tag;

  tracking_infor_tag.u32_gateway_ID = GATEWAY_ID;
  strcpy(tracking_infor_tag.c_gateway_version, GATEWAY_VERSION);

  while (1)
  {
    if (xQueueReceive(*s_ble_tracking_data.p_rssi_ibeacon_queue,
                      &s_beacon_infor_tag,
                      portMAX_DELAY)
        == pdPASS)
    {
      if (WIFI_state_connect() != CONNECT_OK)
      {
        continue;
      }

      memcpy(tracking_infor_tag.u8_beacon_addr,
             s_beacon_infor_tag.u8_beacon_addr,
             6);
      tracking_infor_tag.i8_filtered_rssi = s_beacon_infor_tag.i8_filtered_rssi;

      xQueueSend(*s_ble_tracking_data.p_location_tag_queue,
                 (void *)&tracking_infor_tag,
                 (TickType_t)0);

      xQueueSend(*s_ble_tracking_data.p_addr_tag_queue,
                 (void *)&addr_tag,
                 (TickType_t)0);
    }
  }
}