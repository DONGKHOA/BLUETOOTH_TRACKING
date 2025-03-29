/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "ui.h"
#include "ui_events.h"

#include "esp_camera.h"
#include "app_data.h"

#include <stdbool.h>
#include <stdio.h>

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "ENROLL_EVENT"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_Enroll_Task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static bool b_is_initialize = false;

static lv_obj_t          *camera_canvas = NULL;
static lv_draw_rect_dsc_t rectangle_face;
static lv_draw_rect_dsc_t rectangle_left_eye;
static lv_draw_rect_dsc_t rectangle_right_eye;
static lv_draw_rect_dsc_t rectangle_left_mouth;
static lv_draw_rect_dsc_t rectangle_right_mouth;
static lv_draw_rect_dsc_t rectangle_nouse;

static QueueHandle_t      *p_camera_recognition_queue;
static QueueHandle_t      *p_result_recognition_queue;
static EventGroupHandle_t *p_display_event;

static camera_fb_t              *fb = NULL;
static data_result_recognition_t s_data_result_recognition;

static TaskHandle_t s_enroll_task;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
EVENT_Enroll_Before (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    p_camera_recognition_queue = &s_data_system.s_camera_recognition_queue;
    p_result_recognition_queue = &s_data_system.s_result_recognition_queue;
    p_display_event            = &s_data_system.s_display_event;

    camera_canvas = lv_canvas_create(ui_Enroll);

    lv_draw_rect_dsc_init(&rectangle_face);
    rectangle_face.bg_opa       = LV_OPA_TRANSP; // transparent fill
    rectangle_face.border_width = 2;
    rectangle_face.border_color = lv_color_make(0, 255, 0);

    lv_draw_rect_dsc_init(&rectangle_left_eye);
    rectangle_left_eye.bg_opa       = LV_OPA_TRANSP; // transparent fill
    rectangle_left_eye.border_width = 2;
    rectangle_left_eye.border_color = lv_color_make(0, 255, 0);

    lv_draw_rect_dsc_init(&rectangle_right_eye);
    rectangle_right_eye.bg_opa       = LV_OPA_TRANSP; // transparent fill
    rectangle_right_eye.border_width = 2;
    rectangle_right_eye.border_color = lv_color_make(0, 255, 0);

    lv_draw_rect_dsc_init(&rectangle_left_mouth);
    rectangle_left_mouth.bg_opa       = LV_OPA_TRANSP; // transparent fill
    rectangle_left_mouth.border_width = 2;
    rectangle_left_mouth.border_color = lv_color_make(0, 255, 0);

    lv_draw_rect_dsc_init(&rectangle_right_mouth);
    rectangle_right_mouth.bg_opa       = LV_OPA_TRANSP; // transparent fill
    rectangle_right_mouth.border_width = 2;
    rectangle_right_mouth.border_color = lv_color_make(0, 255, 0);

    lv_draw_rect_dsc_init(&rectangle_nouse);
    rectangle_nouse.bg_opa       = LV_OPA_TRANSP; // transparent fill
    rectangle_nouse.border_width = 2;
    rectangle_nouse.border_color = lv_color_make(0, 255, 0);

    xTaskCreate(
        APP_Enroll_Task, "enroll task", 1024 * 4, NULL, 7, &s_enroll_task);

    b_is_initialize = true;
  }
  else
  {
    vTaskResume(s_enroll_task);
  }

  xEventGroupSetBits(*p_display_event,
                     ENROLL_FACE_ID_BIT | ENROLL_FINGERPRINT_BIT);
}

void
EVENT_Enroll_After (lv_event_t *e)
{
  xEventGroupClearBits(*p_display_event,
                       ENROLL_FACE_ID_BIT | ENROLL_FINGERPRINT_BIT);

  vTaskSuspend(s_enroll_task);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_Enroll_Task (void *arg)
{
  while (1)
  {
    if (xQueueReceive(*p_camera_recognition_queue, &fb, 10) == pdPASS)
    {

      if (!fb)
      {
        continue;
      }

      lv_canvas_set_buffer(
          camera_canvas, fb->buf, fb->width, fb->height, LV_IMG_CF_TRUE_COLOR);

      esp_camera_fb_return(fb);
    }

    if (xQueueReceive(
            *p_result_recognition_queue, &s_data_result_recognition, 1)
        == pdPASS)
    {
      lv_canvas_draw_rect(camera_canvas,
                          s_data_result_recognition.s_coord_box_face.x1,
                          s_data_result_recognition.s_coord_box_face.y1,
                          s_data_result_recognition.s_coord_box_face.x2
                              - s_data_result_recognition.s_coord_box_face.x1,
                          s_data_result_recognition.s_coord_box_face.y2
                              - s_data_result_recognition.s_coord_box_face.y1,
                          &rectangle_face);

      lv_canvas_draw_rect(camera_canvas,
                          s_data_result_recognition.s_left_eye.x,
                          s_data_result_recognition.s_left_eye.y,
                          2,
                          2,
                          &rectangle_left_eye);

      lv_canvas_draw_rect(camera_canvas,
                          s_data_result_recognition.s_right_eye.x,
                          s_data_result_recognition.s_right_eye.y,
                          2,
                          2,
                          &rectangle_right_eye);

      lv_canvas_draw_rect(camera_canvas,
                          s_data_result_recognition.s_left_mouth.x,
                          s_data_result_recognition.s_left_mouth.y,
                          2,
                          2,
                          &rectangle_left_mouth);

      lv_canvas_draw_rect(camera_canvas,
                          s_data_result_recognition.s_right_mouth.x,
                          s_data_result_recognition.s_right_mouth.y,
                          2,
                          2,
                          &rectangle_right_mouth);

      lv_canvas_draw_rect(camera_canvas,
                          s_data_result_recognition.s_nose.x,
                          s_data_result_recognition.s_nose.y,
                          2,
                          2,
                          &rectangle_nouse);
    }

    lv_obj_invalidate(camera_canvas);
  }
}