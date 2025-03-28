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
static lv_draw_rect_dsc_t rect_dsc;

static QueueHandle_t      *p_camera_recognition_queue;
static QueueHandle_t      *p_result_recognition_queue;
static EventGroupHandle_t *p_display_event;

static camera_fb_t             *fb = NULL;
static coord_data_recognition_t s_coord_data_recognition;

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

    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa       = LV_OPA_TRANSP; // transparent fill
    rect_dsc.border_width = 2;
    rect_dsc.border_color = lv_color_make(0, 255, 0);

    xTaskCreate(APP_Enroll_Task, "enroll task", 1024, NULL, 7, &s_enroll_task);

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
    if (xQueueReceive(*p_result_recognition_queue, &s_coord_data_recognition, 1)
        == pdPASS)
    {
    }

    if (xQueueReceive(*p_camera_recognition_queue, &fb, 1) == pdPASS)
    {

      if (!fb)
      {
        continue;
      }

      lv_canvas_set_buffer(
          camera_canvas, fb->buf, fb->width, fb->height, LV_IMG_CF_TRUE_COLOR);

      // lv_canvas_draw_rect(
      //     camera_canvas, , &rect_dsc);

      lv_obj_invalidate(camera_canvas);
      esp_camera_fb_return(fb);
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}