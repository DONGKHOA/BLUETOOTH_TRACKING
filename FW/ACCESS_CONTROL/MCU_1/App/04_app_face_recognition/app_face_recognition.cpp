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

} app_handle_camera_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void print_detection_result(std::list<dl::detect::result_t> &results);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

Face::Face ()
    : // Initialize detectors with required parameters
    detector(0.3F, 0.3F, 10, 0.3F)
    , // score_threshold, nms_threshold, top_k, resize_scale
    detector2(0.4F, 0.3F, 10) // score_threshold, nms_threshold, top_k
{
  this->p_camera_recognition_queue = &s_data_system.s_camera_recognition_queue;
  this->p_result_recognition_queue = &s_data_system.s_result_recognition_queue;
  this->p_display_event            = &s_data_system.s_display_event;
  this->recognizer                 = new FaceRecognition112V1S8();
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
              1024 * 2,
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

  coord_data_recognition_t s_coord_data_recognition
      = { .s_coord_face = { 0, 0, 0, 0 },
          .s_coord_eye  = { 0, 0, 0, 0 },
          .ena_face     = false,
          .ena_eye      = false };

  while (1)
  {
    if (xQueueReceive(*self->p_camera_recognition_queue, &fb, portMAX_DELAY)
        == pdPASS)
    {
      uxBits = xEventGroupWaitBits(*self->p_display_event,
                                   ATTENDANCE_BIT | ENROLL_FACE_ID_BIT
                                       | DELETE_FACE_ID_BIT,
                                   pdFALSE,
                                   pdFALSE,
                                   0);

      std::list<dl::detect::result_t> &detect_candidates = self->detector.infer(
          (uint16_t *)fb->buf, { (int)fb->height, (int)fb->width, 3 });
      std::list<dl::detect::result_t> &detect_results
          = self->detector2.infer((uint16_t *)fb->buf,
                                  { (int)fb->height, (int)fb->width, 3 },
                                  detect_candidates);

      if (detect_results.size() != 1)
      {
        continue;
      }

      if ((uxBits & ATTENDANCE_BIT) != 0)
      {
        self->recognizer->enroll_id((uint16_t *)fb->buf,
                                    { (int)fb->height, (int)fb->width, 3 },
                                    detect_results.front().keypoint,
                                    "",
                                    true);
        ESP_LOGW("ENROLL",
                 "ID %d is enrolled",
                 self->recognizer->get_enrolled_ids().back().id);
      }

      if ((uxBits & ENROLL_FACE_ID_BIT) != 0)
      {
        self->recognize_result = self->recognizer->recognize(
            (uint16_t *)fb->buf,
            { (int)fb->height, (int)fb->width, 3 },
            detect_results.front().keypoint);

        print_detection_result(detect_results);

        if (self->recognize_result.id > 0)
        {
          ESP_LOGI("RECOGNIZE",
                   "Similarity: %f, Match ID: %d",
                   self->recognize_result.similarity,
                   self->recognize_result.id);
        }
        else
        {
          ESP_LOGE("RECOGNIZE",
                   "Similarity: %f, Match ID: %d",
                   self->recognize_result.similarity,
                   self->recognize_result.id);
        }
      }

      if ((uxBits & DELETE_FACE_ID_BIT) != 0)
      {
        vTaskDelay(10);
        self->recognizer->delete_id(true);
        ESP_LOGE(
            "DELETE", "% d IDs left", self->recognizer->get_enrolled_id_num());
      }

      esp_camera_fb_return(fb);
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