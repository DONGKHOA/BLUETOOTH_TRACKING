#ifndef DATA_APP_DATA_ESP32_H_
#define DATA_APP_DATA_ESP32_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

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

#define ATTENDANCE_BIT         BIT0
#define ENROLL_FACE_ID_BIT     BIT1
#define ENROLL_FINGERPRINT_BIT BIT2
#define DELETE_FACE_ID_BIT     BIT3
#define DELETE_FINGERPRINT_BIT BIT4
#define AUTHENTICATION_BIT     BIT5

  typedef enum
  {
    NOTIFICATION_NONE = 0,
    NOTIFICATION_ENROLL_1, // "Keep your eyes in the box"
    NOTIFICATION_ENROLL_2, // "Enroll success"
  } notification_recognition_t;

  typedef struct
  {
    struct coord_box_face
    {
      uint16_t x1;
      uint16_t x2;
      uint16_t y1;
      uint16_t y2;
    } s_coord_box_face;
    struct left_eye
    {
      uint16_t x;
      uint16_t y;
    } s_left_eye;
    struct right_eye
    {
      uint16_t x;
      uint16_t y;
    } s_right_eye;
    struct left_mouth
    {
      uint16_t x;
      uint16_t y;
    } s_left_mouth;
    struct right_mouth
    {
      uint16_t x;
      uint16_t y;
    } s_right_mouth;
    struct nose
    {
      uint16_t x;
      uint16_t y;
    } s_nose;
    int                        ID;
    notification_recognition_t e_notification_recognition;
  } data_result_recognition_t;

  typedef struct 
  {
    uint8_t * u8_buff;
    size_t width;
    size_t height; 
  } camera_capture_t;

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
    QueueHandle_t      s_camera_capture_queue;
    QueueHandle_t      s_camera_recognition_queue;
    QueueHandle_t      s_result_recognition_queue;
    QueueHandle_t      s_send_data_queue;
    QueueHandle_t      s_receive_data_queue;
    EventGroupHandle_t s_display_event;
  } DATA_System_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
