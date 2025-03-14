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
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef enum
  {
    TAG_INDOOR,
    TAG_OUTDOOR,
    TAG_UNKNOWN
  } location_tag_t;

  /**
   * @brief Data structure holding data of system
   *
   *
   * @note s_rssi_ibeacon_queue: RSSI iBeacon data queue (app_ble_ibeacon ->
   * app_ble_tracking)
   *
   * @note s_location_tag_queue: Location tag data queue (app_ble_tracking ->
   * app_data_transmit)
   *
   * @note s_control_camera_queue: Camera control command queue (app_gui ->
   * app_handle_camera)
   *
   * @note s_send_data_queue: Data transmission queue (app_gui ->
   * app_data_transmit)
   *
   * @note s_send_recognition_queue: Face recognition result queue
   * (app_face_recognition -> app_gui)
   */
  typedef struct _DATA_System_t
  {
    QueueHandle_t      s_rssi_ibeacon_queue;
    QueueHandle_t      s_location_tag_queue;
    QueueHandle_t      s_control_camera_queue;
    QueueHandle_t      s_send_data_queue;
    QueueHandle_t      s_send_recognition_queue;
    EventGroupHandle_t s_event_data_cam_eventgroup;
  } DATA_System_t;

  /**
   * @brief Structure defining the data sync between the app_ble_ibeacon and
   * app_ble_tracking.
   */
  typedef struct
  {
    uint8_t beacon_addr[6]; // 6 Bytes Address
    int8_t  i8_filtered_rssi;
  } ibeacon_infor_tag_t;

  /**
   * @brief Structure defining the data sync between the app_ble_tracking and
   * app_data_transmit.
   */
  typedef struct
  {
    uint8_t beacon_addr[6]; // 6 Bytes Address
    int8_t  i8_filtered_rssi;
  } location_infor_tag_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
