#ifndef DATA_APP_DATA_ESP32_H_
#define DATA_APP_DATA_ESP32_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *    PUBLIC DEFINES
   ***************************************************************************/

#define APP_CONFIGURATION_ENABLE  BIT2
#define APP_CONFIGURATION_DISABLE BIT3

#define MQTTSERVER_NVS "MQTTSERVER_NVS"
#define MQTTTOPIC_NVS  "MQTTTOPIC_NVS"

  /*** GPIO peripheral *******************************************************/

#define BUTTON_USER_PIN GPIO_NUM_12
#define LED_STATUS_PIN  GPIO_NUM_5

  /*** state system **********************************************************/

#define STATUS_WIFI_CONNECTED    BIT0
#define STATUS_WIFI_DISCONNECTED BIT1
#define STATUS_MQTT_CONNECTED    BIT2
#define STATUS_MQTT_DISCONNECTED BIT3
#define STATUS_BLE_CONFIGURATION BIT4

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  /**
   * @brief Data structure holding data of system
   *
   *
   * @note s_rssi_ibeacon_queue: RSSI iBeacon data queue (app_ble_ibeacon ->
   * app_ble_tracking)
   *
   * @note s_location_tag_queue: Location tag data queue (app_ble_tracking ->
   * app_mqtt_client)
   *
   */
  typedef struct _DATA_System_t
  {
    QueueHandle_t      s_rssi_ibeacon_queue;
    QueueHandle_t      s_location_tag_queue;
    EventGroupHandle_t s_configuration_event;
  } DATA_System_t;

  /**
   * @brief Structure defining the data sync between the app_ble_ibeacon and
   * app_ble_tracking.
   */
  typedef struct
  {
    uint8_t u8_beacon_addr[6]; // 6 Bytes Address
    int8_t  i8_filtered_rssi;
  } ibeacon_infor_tag_t;

  /**
   * @brief Structure defining the data sync between the app_ble_tracking and
   * app_data_transmit.
   */
  typedef struct
  {
    uint8_t  u8_beacon_addr[6]; // 6 Bytes Address
    uint32_t u32_gateway_ID;
    char     c_gateway_version[6];
    int8_t   i8_filtered_rssi;
  } tracking_infor_tag_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
