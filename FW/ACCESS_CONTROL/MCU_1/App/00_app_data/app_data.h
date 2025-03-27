#ifndef DATA_APP_DATA_ESP32_H_
#define DATA_APP_DATA_ESP32_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include <stdint.h>

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

  typedef struct
  {
    struct coord_face
    {
      uint16_t x1;
      uint16_t x2;
      uint16_t y1;
      uint16_t y2;
    } s_coord_face;
    struct coord_eye
    {
      uint16_t x1;
      uint16_t x2;
      uint16_t y1;
      uint16_t y2;
    } s_coord_eye;
    bool ena_face;
    bool ena_eye;
  } coord_data_recognition_t;

  /**
   * @brief Data structure holding data of system
   *
   * @note s_camera_capture_queue: Camera control command queue
   * (app_handle_camera -> app_gui)
   *
   * @note s_camera_recognition_queue: Face recognition result queue
   * (app_face_recognition -> app_gui)
   *
   * @note s_send_data_queue: Data transmission queue (app_gui ->
   * app_data_transmit)
   *
   */
  typedef struct _DATA_System_t
  {
    QueueHandle_t s_camera_capture_queue;
    QueueHandle_t s_camera_recognition_queue;
    QueueHandle_t s_result_recognition_queue;
    QueueHandle_t s_send_data_queue;
    QueueHandle_t s_receive_data_queue;
  } DATA_System_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
