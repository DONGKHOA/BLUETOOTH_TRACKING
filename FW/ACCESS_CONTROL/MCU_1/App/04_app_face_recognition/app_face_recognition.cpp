/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "sdkconfig.h"
#include "esp_camera.h"
#include "esp_log.h"

#include "app_data.h"
#include "app_face_recognition.hpp"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_FACE_RECOGNITION"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static bool check_face_in_box(uint16_t left_eye_x,
                              uint16_t left_eye_y,
                              uint16_t right_eye_x,
                              uint16_t right_eye_y,
                              uint16_t nose_x,
                              uint16_t nose_y,
                              uint16_t mouth_left_x,
                              uint16_t mouth_left_y,
                              uint16_t mouth_right_x,
                              uint16_t mouth_right_y,
                              uint16_t x_min,
                              uint16_t y_min,
                              uint16_t x_max,
                              uint16_t y_max);
static bool check_eyes_in_box(uint16_t left_eye_x,
                              uint16_t left_eye_y,
                              uint16_t right_eye_x,
                              uint16_t right_eye_y,
                              uint16_t x_min,
                              uint16_t y_min,
                              uint16_t x_max,
                              uint16_t y_max);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

/******************************************************************************
 *   METHOD
 *****************************************************************************/

Face::Face ()
    : // Initialize detectors with required parameters
    detector(0.3F, 0.3F, 10, 0.3F)
    , // score_threshold, nms_threshold, top_k, resize_scale
    detector2(0.4F, 0.3F, 10) // score_threshold, nms_threshold, top_k
{
  this->p_camera_recognition_queue = &s_data_system.s_camera_recognition_queue;
  this->p_result_recognition_queue = &s_data_system.s_result_recognition_queue;

  this->p_display_event = &s_data_system.s_display_event;

  this->recognizer = new FaceRecognition112V1S8();
}

Face::~Face () // Added destructor implementation
{
  delete recognizer;
}

void
Face::CreateTask ()
{
  xTaskCreate(&Face::APP_FACE_RECOGNITION_Task,
              "face task",
              1024 * 4,
              this, // Pass the Face instance as a void*
              7,
              NULL);
}

void
Face::APP_FACE_RECOGNITION_Task (void *pvParameters)
{
  Face        *self = static_cast<Face *>(pvParameters);
  static camera_capture_t s_camera_capture;
  EventBits_t  uxBits;

  uint8_t u8_post_enroll_frame_count = 0;
  uint8_t stable_face_count          = 0;
  bool    is_face_enrolled           = false;

  std::list<dl::detect::result_t> detect_results;

  data_result_recognition_t s_data_result_recognition
      = { .s_coord_box_face           = { 0, 0, 0, 0 },
          .s_left_eye                 = { 0, 0 },
          .s_right_eye                = { 0, 0 },
          .s_left_mouth               = { 0, 0 },
          .s_right_mouth              = { 0, 0 },
          .s_nose                     = { 0, 0 },
          .ID                         = -1,
          .e_notification_recognition = NOTIFICATION_NONE };

  self->recognizer->set_partition(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");

  self->recognizer->set_ids_from_flash();

  while (1)
  {
    if (xQueueReceive(*self->p_camera_recognition_queue, &s_camera_capture, portMAX_DELAY)
        == pdPASS)
    {
      uxBits = xEventGroupWaitBits(*self->p_display_event,
                                   ATTENDANCE_BIT | ENROLL_FACE_ID_BIT
                                       | DELETE_FACE_ID_BIT,
                                   pdFALSE,
                                   pdFALSE,
                                   0);

      if ((uxBits & ATTENDANCE_BIT) != 0)
      {
      }

      if ((uxBits & ENROLL_FACE_ID_BIT) != 0)
      {
        if (stable_face_count >= 255)
        {
          //   dl::image::draw_filled_rectangle((uint16_t *)fb->buf,
          //                                    fb->height,
          //                                    fb->width,
          //                                    0,
          //                                    220,
          //                                    340,
          //                                    240,
          //                                    RGB565_MASK_WHITE);
          // rgb_printf(fb, 80, 234, RGB565_MASK_GREEN, "Enroll success");
          // if (is_face_enrolled == false)
          // {
          //   self->recognizer->enroll_id((uint16_t *)fb->buf,
          //                               { (int)fb->height, (int)fb->width, 3
          //                               }, detect_results.front().keypoint,
          //                               "0",
          //                               true);
          //   ESP_LOGI(TAG,
          //            "Enroll ID %d",
          //            self->recognizer->get_enrolled_ids().back().id);
          //   is_face_enrolled = true;
          // }

          // u8_post_enroll_frame_count++;
          // if (u8_post_enroll_frame_count >= 10)
          // {
          //   u8_post_enroll_frame_count = 0;
          //   stable_face_count          = 0;
          //   ESP_LOGI(TAG, "Enroll completed.");
          // }
        }
        else
        {
          std::list<dl::detect::result_t> &detect_candidates
              = self->detector.infer((uint16_t *)s_camera_capture.u8_buff,
                                     { (int)s_camera_capture.height, (int)s_camera_capture.width, 3 });
          detect_results
              = self->detector2.infer((uint16_t *)s_camera_capture.u8_buff,
                                      { (int)s_camera_capture.height, (int)s_camera_capture.width, 3 },
                                      detect_candidates);

          if (detect_results.size())
          {
            vTaskDelay(pdMS_TO_TICKS(1));
            uint16_t i = 0;
            for (std::list<dl::detect::result_t>::iterator prediction
                 = detect_results.begin();
                 prediction != detect_results.end();
                 prediction++, i++)
            {
              if (prediction->keypoint.size() == 10)
              {
                s_data_result_recognition.s_coord_box_face.x1
                    = DL_MAX(prediction->box[0], 0);
                s_data_result_recognition.s_coord_box_face.y1
                    = DL_MAX(prediction->box[1], 0);
                s_data_result_recognition.s_coord_box_face.x2
                    = DL_MAX(prediction->box[2], 0);
                s_data_result_recognition.s_coord_box_face.y2
                    = DL_MAX(prediction->box[3], 0);

                s_data_result_recognition.s_left_eye.x
                    = DL_MAX(prediction->keypoint[0], 0);
                s_data_result_recognition.s_left_eye.y
                    = DL_MAX(prediction->keypoint[1], 0);

                s_data_result_recognition.s_left_mouth.x
                    = DL_MAX(prediction->keypoint[2], 0);
                s_data_result_recognition.s_left_mouth.y
                    = DL_MAX(prediction->keypoint[3], 0);

                s_data_result_recognition.s_nose.x
                    = DL_MAX(prediction->keypoint[4], 0);
                s_data_result_recognition.s_nose.y
                    = DL_MAX(prediction->keypoint[5], 0);

                s_data_result_recognition.s_right_eye.x
                    = DL_MAX(prediction->keypoint[6], 0);
                s_data_result_recognition.s_right_eye.y
                    = DL_MAX(prediction->keypoint[7], 0);

                s_data_result_recognition.s_right_mouth.x
                    = DL_MAX(prediction->keypoint[8], 0);
                s_data_result_recognition.s_right_mouth.y
                    = DL_MAX(prediction->keypoint[9], 0);
              }
            }

            if (check_eyes_in_box(s_data_result_recognition.s_left_eye.x,
                                  s_data_result_recognition.s_left_eye.y,
                                  s_data_result_recognition.s_right_eye.x,
                                  s_data_result_recognition.s_right_eye.y,
                                  110,
                                  80,
                                  210,
                                  120))
            {
              stable_face_count++;
              ESP_LOGI(TAG, "stable_face_count: %d", stable_face_count);
            }
            else
            {
              s_data_result_recognition.s_coord_box_face.x1 = 0;
              s_data_result_recognition.s_coord_box_face.y1 = 0;
              s_data_result_recognition.s_coord_box_face.x2 = 0;
              s_data_result_recognition.s_coord_box_face.y2 = 0;
              s_data_result_recognition.s_left_eye.x        = 0;
              s_data_result_recognition.s_left_eye.y        = 0;
              s_data_result_recognition.s_right_eye.x       = 0;
              s_data_result_recognition.s_right_eye.y       = 0;
              s_data_result_recognition.s_left_mouth.x      = 0;
              s_data_result_recognition.s_left_mouth.y      = 0;
              s_data_result_recognition.s_right_mouth.x     = 0;
              s_data_result_recognition.s_right_mouth.y     = 0;
              s_data_result_recognition.s_nose.x            = 0;
              s_data_result_recognition.s_nose.y            = 0;
            }
            xQueueSend(*self->p_result_recognition_queue,
                       &s_data_result_recognition,
                       0);
            
          }
        }
      }

      if ((uxBits & DELETE_FACE_ID_BIT) != 0)
      {
      }
    }
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static bool
check_face_in_box (uint16_t left_eye_x,
                   uint16_t left_eye_y,
                   uint16_t right_eye_x,
                   uint16_t right_eye_y,
                   uint16_t nose_x,
                   uint16_t nose_y,
                   uint16_t mouth_left_x,
                   uint16_t mouth_left_y,
                   uint16_t mouth_right_x,
                   uint16_t mouth_right_y,
                   uint16_t x_min,
                   uint16_t y_min,
                   uint16_t x_max,
                   uint16_t y_max)
{
  if (left_eye_x > x_min && left_eye_x < x_max && left_eye_y > y_min
      && left_eye_y < y_max && right_eye_x > x_min && right_eye_x < x_max
      && right_eye_y > y_min && right_eye_y < y_max && nose_x > x_min
      && nose_x < x_max && nose_y > y_min && nose_y < y_max
      && mouth_left_x > x_min && mouth_left_x < x_max && mouth_left_y > y_min
      && mouth_left_y < y_max && mouth_right_x > x_min && mouth_right_x < x_max
      && mouth_right_y > y_min && mouth_right_y < y_max)
  {
    return true;
  }
  return false;
}

bool
check_eyes_in_box (uint16_t left_eye_x,
                   uint16_t left_eye_y,
                   uint16_t right_eye_x,
                   uint16_t right_eye_y,
                   uint16_t x_min,
                   uint16_t y_min,
                   uint16_t x_max,
                   uint16_t y_max)
{
  if (left_eye_x > x_min && left_eye_x < x_max && left_eye_y > y_min
      && left_eye_y < y_max && right_eye_x > x_min && right_eye_x < x_max
      && right_eye_y > y_min && right_eye_y < y_max)
  {
    return true;
  }
  return false;
}