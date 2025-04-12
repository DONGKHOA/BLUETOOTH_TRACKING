/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "ui.h"
#include "ui_events.h"

#include "app_data.h"

#include "esp_camera.h"

#include <stdbool.h>
#include <stdio.h>

#include "environment.h"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_Attendance_Timer(lv_timer_t *timer);
static void EVENT_ATTENDANCE_ShowHomeScreen(void *param);

static void EVENT_PROCESS_ATTENDANCE_DATA_Task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static TaskHandle_t s_attendance_task_handle;

static lv_obj_t   *camera_canvas = NULL;
static lv_timer_t *timer_attendance;

static QueueHandle_t      *p_send_data_queue;
static QueueHandle_t      *p_receive_data_event_queue;
static QueueHandle_t      *p_camera_capture_queue;
static QueueHandle_t      *p_result_recognition_queue;
static EventGroupHandle_t *p_display_event;

static DATA_SYNC_t s_DATA_SYNC;

static camera_fb_t              *fb = NULL;
static data_result_recognition_t s_data_result_recognition
    = { .s_coord_box_face = { 0, 0, 0, 0 },
        .s_left_eye       = { 0, 0 },
        .s_right_eye      = { 0, 0 },
        .s_left_mouth     = { 0, 0 },
        .s_right_mouth    = { 0, 0 },
        .s_nose           = { 0, 0 },
        .ID               = -1 };

static bool               b_is_initialize = false;
static lv_draw_rect_dsc_t rectangle_face;
static lv_draw_rect_dsc_t rectangle_left_eye;
static lv_draw_rect_dsc_t rectangle_right_eye;
static lv_draw_rect_dsc_t rectangle_left_mouth;
static lv_draw_rect_dsc_t rectangle_right_mouth;
static lv_draw_rect_dsc_t rectangle_nouse;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
EVENT_Attendance_Before (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    p_camera_capture_queue     = &s_data_system.s_camera_capture_queue;
    p_result_recognition_queue = &s_data_system.s_result_recognition_queue;
    p_display_event            = &s_data_system.s_display_event;
    p_send_data_queue          = &s_data_system.s_send_data_queue;
    p_receive_data_event_queue = &s_data_system.s_receive_data_event_queue;

    camera_canvas = lv_canvas_create(ui_Attendance);

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

    b_is_initialize  = true;
    timer_attendance = lv_timer_create(APP_Attendance_Timer, 30, NULL);

    xTaskCreate(EVENT_PROCESS_ATTENDANCE_DATA_Task,
                "process attendance task",
                1024 * 4,
                NULL,
                6,
                &s_attendance_task_handle);
  }
  else
  {
    lv_timer_resume(timer_attendance);
    vTaskResume(s_attendance_task_handle);
  }

  xEventGroupSetBits(*p_display_event, ATTENDANCE_BIT);

  // Send Start Attendance to MCU2
  s_DATA_SYNC.u8_data_start     = DATA_SYNC_START_ATTENDANCE;
  s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_DUMMY;
  s_DATA_SYNC.u8_data_length    = 1;
  s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
  xQueueSend(*p_send_data_queue, &s_DATA_SYNC, 0);
}

void
EVENT_Attendance_After (lv_event_t *e)
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

  lv_timer_pause(timer_attendance);
  vTaskSuspend(s_attendance_task_handle);
  xEventGroupClearBits(*p_display_event, ATTENDANCE_BIT);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
EVENT_PROCESS_ATTENDANCE_DATA_Task (void *arg)
{
  while (1)
  {
    if (xQueueReceive(*p_receive_data_event_queue, &s_DATA_SYNC, portMAX_DELAY)
        == pdTRUE)
    {
      switch (s_DATA_SYNC.u8_data_start)
      {
        case DATA_SYNC_RESPONSE_ATTENDANCE:
          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_SUCCESS)
          {
            xEventGroupClearBits(*p_display_event, ATTENDANCE_BIT);
          }

          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_FAIL)
          {
            xEventGroupClearBits(*p_display_event, ATTENDANCE_BIT);
          }
          break;

        default:
          break;
      }
    }
  }
}

static void
APP_Attendance_Timer (lv_timer_t *timer)
{
  if ((xEventGroupGetBits(*p_display_event) & ATTENDANCE_BIT) == 0)
  {
    lv_async_call(EVENT_ATTENDANCE_ShowHomeScreen, NULL);
  }

  xQueueReceive(*p_result_recognition_queue, &s_data_result_recognition, 1);

  if (xQueueReceive(*p_camera_capture_queue, &fb, 1) == pdPASS)
  {
    lv_canvas_set_buffer(
        camera_canvas, fb->buf, fb->width, fb->height, LV_IMG_CF_TRUE_COLOR);

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
  esp_camera_fb_return(fb);
}

static void
EVENT_ATTENDANCE_ShowHomeScreen (void *param)
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

  lv_timer_pause(timer_attendance);
  vTaskSuspend(s_attendance_task_handle);

  _ui_screen_change(
      &ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);

  lv_async_call((lv_async_cb_t)EVENT_Home_Before, NULL);
}