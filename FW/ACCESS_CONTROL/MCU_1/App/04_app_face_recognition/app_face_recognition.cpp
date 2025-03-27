/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_face_recognition.hpp"

#include "esp_camera.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_FACE_RECOGNITION"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_FACE_RECOGNITION_Attendance(Face *self);
static void APP_FACE_RECOGNITION_Enroll(Face *self);
static void APP_FACE_RECOGNITION_Delete(Face *self);

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

      if ((uxBits & ATTENDANCE_BIT) != 0)
      {
        APP_FACE_RECOGNITION_Attendance(self);
      }

      if ((uxBits & ENROLL_FACE_ID_BIT) != 0)
      {
        APP_FACE_RECOGNITION_Enroll(self);
      }

      if ((uxBits & DELETE_FACE_ID_BIT) != 0)
      {
        APP_FACE_RECOGNITION_Delete(self);
      }

      esp_camera_fb_return(fb);
    }
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void APP_FACE_RECOGNITION_Attendance(Face *self)
{

}

static void APP_FACE_RECOGNITION_Enroll(Face *self)
{

}

static void APP_FACE_RECOGNITION_Delete(Face *self)
{

}