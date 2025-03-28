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

typedef struct
{
  int left_eye_x;
  int left_eye_y;
  int right_eye_x;
  int right_eye_y;
  int left_mouth_x;
  int left_mouth_y;
  int right_mouth_x;
  int right_mouth_y;
  int nose_x;
  int nose_y;
} app_handle_camera_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void print_detection_result(std::list<dl::detect::result_t> &results);
static bool check_face_in_box(int left_eye_x,
                              int left_eye_y,
                              int right_eye_x,
                              int right_eye_y,
                              int nose_x,
                              int nose_y,
                              int mouth_left_x,
                              int mouth_left_y,
                              int mouth_right_x,
                              int mouth_right_y,
                              int x_min,
                              int y_min,
                              int x_max,
                              int y_max);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static app_handle_camera_t s_app_handle_camera = { .left_eye_x    = 0,
                                                   .left_eye_y    = 0,
                                                   .right_eye_x   = 0,
                                                   .right_eye_y   = 0,
                                                   .left_mouth_x  = 0,
                                                   .left_mouth_y  = 0,
                                                   .right_mouth_x = 0,
                                                   .right_mouth_y = 0,
                                                   .nose_x        = 0,
                                                   .nose_y        = 0 };

/******************************************************************************
 *   METHOD
 *****************************************************************************/

Face::Face ()
    : // Initialize detectors with required parameters
    detector(0.3F, 0.3F, 10, 0.3F)
    , // score_threshold, nms_threshold, top_k, resize_scale
    detector2(0.4F, 0.3F, 10) // score_threshold, nms_threshold, top_k
{
  this->p_camera_capture_queue     = &s_data_system.s_camera_capture_queue;
  this->p_camera_recognition_queue = &s_data_system.s_camera_recognition_queue;
  this->p_result_recognition_queue = &s_data_system.s_result_recognition_queue;
  this->p_display_event            = &s_data_system.s_display_event;
  this->recognizer                 = new FaceRecognition112V1S8();

  this->u8_stable_face_count = 0;
}

Face::~Face () // Added destructor implementation
{
  delete recognizer;
}

void
Face::CreateTask ()
{
  xTaskCreate(&Face::APP_FACE_RECOGNITION_Task,
              "face recognition task",
              1024 * 4,
              this, // Pass the Face instance as a void*
              13,
              NULL);
}

void
Face::APP_FACE_RECOGNITION_Task (void *pvParameters)
{
  Face        *self = static_cast<Face *>(pvParameters);
  camera_fb_t *fb   = nullptr;
  EventBits_t  uxBits;

  coord_data_recognition_t s_coord_data_recognition = {
    .s_coord_face = { 0, 0, 0, 0 }, .s_coord_eye = { 0, 0, 0, 0 }, .ID = -1
  };

  self->recognizer->set_partition(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");

  self->recognizer->set_ids_from_flash();

  while (1)
  {
    if (xQueueReceive(*self->p_camera_capture_queue, &fb, portMAX_DELAY)
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
        if (self->u8_stable_face_count > 7)
        {
        //   dl::image::draw_filled_rectangle((uint16_t *)fb->buf,
        //                                    fb->height,
        //                                    fb->width,
        //                                    0,
        //                                    220,
        //                                    340,
        //                                    240,
        //                                    RGB565_MASK_WHITE);
          rgb_printf(fb, 80, 234, RGB565_MASK_GREEN, "Enroll success");
          if (is_face_enrolled == false)
          {
            std::string text_id
                = std::to_string(users[usr_data_selected_item].id);
            self->recognizer->enroll_id(
                (uint16_t *)fb->buf,
                { (int)fb->height, (int)fb->width, 3 },
                detect_results.front().keypoint,
                text_id,
                true);
            ESP_LOGI(TAG,
                     "Enroll ID %d",
                     self->recognizer->get_enrolled_ids().back().id);
            is_face_enrolled = true;
          }

          post_enroll_fb_count++;
          if (post_enroll_fb_count >= 10)
          {
            post_enroll_fb_count = 0;
            stable_face_count       = 0;
            ESP_LOGI(TAG, "Enroll completed.");
            esc_faceid_enroll();
          }
        }
        else
        {
          // draw_fixed_eye_box((uint16_t *)fb->buf, fb->height,
          // fb->width);
          // dl::image::draw_filled_rectangle((uint16_t *)fb->buf,
          //                                  fb->height,
          //                                  fb->width,
          //                                  0,
          //                                  220,
          //                                  340,
          //                                  240,
          //                                  RGB565_MASK_WHITE);
          // rgb_printf(fb, 20, 234, RGB565_MASK_BLACK, "Keep your eyes in
          // the box");

          std::list<dl::detect::result_t> &detect_candidates
              = self->detector.infer(
                  (uint16_t *)fb->buf,
                  { (int)fb->height, (int)fb->width, 3 });
          detect_results = self->detector2.infer(
              (uint16_t *)fb->buf,
              { (int)fb->height, (int)fb->width, 3 },
              detect_candidates);

          if (detect_results.size())
          {
            // draw_detection((uint16_t *)fb->buf, fb->height,
            // fb->width, detect_results, s_app_handle_camera.left_eye_x,
            // s_app_handle_camera.left_eye_y, s_app_handle_camera.right_eye_x,
            // s_app_handle_camera.right_eye_y,
            // s_app_handle_camera.left_mouth_x,
            // s_app_handle_camera.left_mouth_y,
            // s_app_handle_camera.right_mouth_x,
            // s_app_handle_camera.right_mouth_y, s_app_handle_camera.nose_x,
            // s_app_handle_camera.nose_y);
            if (check_eyes_in_box(s_app_handle_camera.left_eye_x,
                                  s_app_handle_camera.left_eye_y,
                                  s_app_handle_camera.right_eye_x,
                                  s_app_handle_camera.right_eye_y,
                                  110,
                                  80,
                                  210,
                                  120))
            {
              self->u8_stable_face_count++;
              ESP_LOGI(TAG, "stable_face_count: %d", self->u8_stable_face_count);
            }
          }
        }
      }

      if ((uxBits & DELETE_FACE_ID_BIT) != 0)
      {
      }
      xQueueSend(*self->p_camera_recognition_queue, &fb, 0);
    }
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
print_detection_result (std::list<dl::detect::result_t> &results)
{
  int i = 0;
  for (std::list<dl::detect::result_t>::iterator prediction = results.begin();
       prediction != results.end();
       prediction++, i++)
  {
    ESP_LOGI("detection_result",
             "[%2d]: (%3d, %3d, %3d, %3d)",
             i,
             prediction->box[0],
             prediction->box[1],
             prediction->box[2],
             prediction->box[3]);

    if (prediction->keypoint.size() == 10)
    {
      ESP_LOGI("detection_result",
               "      left eye: (%3d, %3d), right eye: (%3d, %3d), nose: (%3d, "
               "%3d), mouth left: (%3d, %3d), mouth right: (%3d, %3d)",
               prediction->keypoint[0],
               prediction->keypoint[1], // left eye
               prediction->keypoint[6],
               prediction->keypoint[7], // right eye
               prediction->keypoint[4],
               prediction->keypoint[5], // nose
               prediction->keypoint[2],
               prediction->keypoint[3], // mouth left corner
               prediction->keypoint[8],
               prediction->keypoint[9]); // mouth right corner
    }
  }
}

static bool
check_face_in_box (int left_eye_x,
                   int left_eye_y,
                   int right_eye_x,
                   int right_eye_y,
                   int nose_x,
                   int nose_y,
                   int mouth_left_x,
                   int mouth_left_y,
                   int mouth_right_x,
                   int mouth_right_y,
                   int x_min,
                   int y_min,
                   int x_max,
                   int y_max)
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