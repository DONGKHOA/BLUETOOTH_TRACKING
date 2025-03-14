/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "app_ble_tracking.h"
#include "app_data.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_BLE_TRACKING"

#define NAME_TAG_1 "IBEACON_TAG_1"
#define NAME_TAG_2 "IBEACON_TAG_2"

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
} ble_tracking_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_BLE_TRACKING_task(void *arg);
// static void APP_BLE_TRACKING_AsignNameTag(uint8_t *p_addr, char *p_name);
// static void APP_BLE_TRACKING_CheckLocationTag(location_tag_t *p_location_tag,
//                                               int8_t          rssi);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static ble_tracking_data_t s_ble_tracking_data;
// static uint8_t             addr_0[6] = { 0xC3, 0x00, 0x00, 0x35, 0x61, 0xE2 };
// static uint8_t             addr_1[6] = { 0xC3, 0x00, 0x00, 0x35, 0x61, 0xE3 };

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_BLE_TRACKING_CreateTask (void)
{
  xTaskCreate(APP_BLE_TRACKING_task, "ble tracking task", 1024 * 2, NULL, 14, NULL);
}

void
APP_BLE_TRACKING_Init (void)
{
  s_ble_tracking_data.p_rssi_ibeacon_queue
      = &s_data_system.s_rssi_ibeacon_queue; // Get RSSI iBeacon data queue

  s_ble_tracking_data.p_location_tag_queue
      = &s_data_system.s_location_tag_queue; // Get Location tag data queue
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_BLE_TRACKING_task (void *arg)
{
  ibeacon_infor_tag_t  s_beacon_infor_tag;
  location_infor_tag_t s_location_tag;
  while (1)
  {
    if (xQueueReceive(*s_ble_tracking_data.p_rssi_ibeacon_queue,
                      &s_beacon_infor_tag,
                      portMAX_DELAY)
        == pdPASS)
    {
      // APP_BLE_TRACKING_AsignNameTag(s_beacon_infor_tag.beacon_addr,
      //                               s_location_tag.p_name);
      // APP_BLE_TRACKING_CheckLocationTag(&s_location_tag.e_location_tag,
      //                                   s_beacon_infor_tag.i8_filtered_rssi);
      
      memcpy(s_location_tag.beacon_addr, s_beacon_infor_tag.beacon_addr, 6);
      s_location_tag.i8_filtered_rssi = s_beacon_infor_tag.i8_filtered_rssi;

      xQueueSend(*s_ble_tracking_data.p_location_tag_queue,
                 (void *)&s_location_tag,
                 (TickType_t)0);
    }
  }
}

// static void
// APP_BLE_TRACKING_AsignNameTag (uint8_t *p_addr, char *p_name)
// {
//   if (memcmp(p_addr, addr_0, 6) == 0)
//   {
//     strcpy(p_name, NAME_TAG_1);
//   }
//   else if (memcmp(p_addr, addr_1, 6) == 0)
//   {
//     strcpy(p_name, NAME_TAG_2);
//   }
//   else
//   {
//     strcpy(p_name, "UNKNOWN");
//   }
// }

// static void
// APP_BLE_TRACKING_CheckLocationTag (location_tag_t *p_location_tag, int8_t rssi)
// {
//   if (rssi > -90)
//   {
//     *p_location_tag = TAG_UNKNOWN;
//   }
//   else if (rssi > -80)
//   {
//     *p_location_tag = TAG_OUTDOOR;
//   }
//   else
//   {
//     *p_location_tag = TAG_INDOOR;
//   }
// }