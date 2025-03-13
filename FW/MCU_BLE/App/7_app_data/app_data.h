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

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
