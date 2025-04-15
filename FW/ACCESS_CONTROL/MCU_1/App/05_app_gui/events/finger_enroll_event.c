/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "ui.h"
#include "ui_events.h"

#include "app_data.h"

#include "environment.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************
 *  PUBLIC VARIABLES
 *****************************************************************************/

extern char     enroll_number_id[8];
extern uint16_t user_id;

extern char time_buffer[8];
extern char date_buffer[16];

extern time_data_t s_time_data;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_UPDATE_FINGER_ENROLL_TIME_Timer(lv_timer_t *timer);

static void EVENT_PROCESS_FINGER_ENROLL_DATA_Task(void *arg);

static void EVENT_FINGER_ShowSuccessStatus(void *user_data);
static void EVENT_FINGER_ShowFailStatus(void *user_data);
static void EVENT_FINGER_ShowSuccessEnroll(void *user_data);
static void EVENT_FINGER_ShowFailEnroll(void *user_data);

static void EVENT_ENROLL_ShowHomeScreen(void *param);

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t *p_send_data_queue;
  QueueHandle_t *p_receive_data_event_queue;
} finger_enroll_event_data_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static TaskHandle_t s_finger_enroll_task_handle;

static lv_obj_t   *ui_FingerEnrollTime;
static lv_obj_t   *ui_IDTextEnroll4;
static lv_obj_t   *ui_IDTextEnroll5;
static lv_obj_t   *ui_FingerStatus1;
static lv_obj_t   *ui_FingerStatus2;
static lv_timer_t *timer_update_time_finger_enroll;

static DATA_SYNC_t                s_DATA_SYNC;
static state_system_t            *p_state_system;
static finger_enroll_event_data_t s_finger_enroll_event_data;

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
EVENT_Enroll_Finger (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    p_state_system  = &s_data_system.s_state_system;
    b_is_initialize = true;
    s_finger_enroll_event_data.p_send_data_queue
        = &s_data_system.s_send_data_queue;
    s_finger_enroll_event_data.p_receive_data_event_queue
        = &s_data_system.s_receive_data_event_queue;

    ui_FingerEnrollTime = lv_label_create(ui_FingerEnroll);
    lv_obj_set_width(ui_FingerEnrollTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_FingerEnrollTime, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_FingerEnrollTime, 23);
    lv_obj_set_y(ui_FingerEnrollTime, -105);
    lv_obj_set_align(ui_FingerEnrollTime, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_color(ui_FingerEnrollTime,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_FingerEnrollTime, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_FingerEnrollTime,
                               &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_IDTextEnroll4 = lv_label_create(ui_POPUPMenuPanel4);
    lv_obj_set_width(ui_IDTextEnroll4, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_IDTextEnroll4, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_IDTextEnroll4, 15);
    lv_obj_set_y(ui_IDTextEnroll4, -30);
    lv_obj_set_align(ui_IDTextEnroll4, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(ui_IDTextEnroll4,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_IDTextEnroll4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_IDTextEnroll4,
                               &lv_font_montserrat_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_IDTextEnroll5 = lv_label_create(ui_POPUPMenuPanel4);
    lv_obj_set_width(ui_IDTextEnroll5, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_IDTextEnroll5, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_IDTextEnroll5, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(ui_IDTextEnroll5,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_IDTextEnroll5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_IDTextEnroll5,
                               &lv_font_montserrat_18,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_FingerStatus1 = lv_obj_create(ui_POPUPMenuPanel4);
    lv_obj_set_width(ui_FingerStatus1, 20);
    lv_obj_set_height(ui_FingerStatus1, 20);
    lv_obj_set_x(ui_FingerStatus1, -20);
    lv_obj_set_y(ui_FingerStatus1, 30);
    lv_obj_set_align(ui_FingerStatus1, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_FingerStatus1, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(
        ui_FingerStatus1, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(
        ui_FingerStatus1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_FingerStatus2 = lv_obj_create(ui_POPUPMenuPanel4);
    lv_obj_set_width(ui_FingerStatus2, 20);
    lv_obj_set_height(ui_FingerStatus2, 20);
    lv_obj_set_x(ui_FingerStatus2, 20);
    lv_obj_set_y(ui_FingerStatus2, 30);
    lv_obj_set_align(ui_FingerStatus2, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_FingerStatus2, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(
        ui_FingerStatus2, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(
        ui_FingerStatus2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    timer_update_time_finger_enroll
        = lv_timer_create(EVENT_UPDATE_FINGER_ENROLL_TIME_Timer, 1000, NULL);

    xTaskCreate(EVENT_PROCESS_FINGER_ENROLL_DATA_Task,
                "process finger enroll task",
                1024 * 4,
                NULL,
                6,
                &s_finger_enroll_task_handle);
  }
  else
  {
    lv_timer_resume(timer_update_time_finger_enroll);
    vTaskResume(s_finger_enroll_task_handle);
  }

  lv_label_set_text(ui_IDTextEnroll4, enroll_number_id);
  lv_label_set_text(ui_IDTextEnroll5, "Please place your finger");
  lv_obj_set_style_bg_color(ui_FingerStatus1,
                            lv_color_hex(0xFF0000),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_FingerStatus2,
                            lv_color_hex(0xFF0000),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_label_set_text(ui_FingerEnrollTime, time_buffer);

  s_DATA_SYNC.u8_data_start     = DATA_SYNC_ENROLL_FINGERPRINT;
  s_DATA_SYNC.u8_data_packet[0] = (user_id << 8) & 0xFF;
  s_DATA_SYNC.u8_data_packet[1] = user_id & 0xFF;
  s_DATA_SYNC.u8_data_length    = 2;
  s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
  xQueueSend(*s_finger_enroll_event_data.p_send_data_queue, &s_DATA_SYNC, 0);
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/

static void
EVENT_PROCESS_FINGER_ENROLL_DATA_Task (void *arg)
{
  while (1)
  {
    if (xQueueReceive(*s_finger_enroll_event_data.p_receive_data_event_queue,
                      &s_DATA_SYNC,
                      portMAX_DELAY)
        == pdTRUE)
    {
      switch (s_DATA_SYNC.u8_data_start)
      {
        case DATA_SYNC_TIME:
          // Handle time data response
          s_time_data.u8_minute = s_DATA_SYNC.u8_data_packet[0];
          s_time_data.u8_hour   = s_DATA_SYNC.u8_data_packet[1];
          s_time_data.u8_day    = s_DATA_SYNC.u8_data_packet[2];
          s_time_data.u8_month  = s_DATA_SYNC.u8_data_packet[3];
          s_time_data.u8_year   = s_DATA_SYNC.u8_data_packet[4];

          snprintf(time_buffer,
                   sizeof(time_buffer),
                   "%02d:%02d",
                   s_time_data.u8_hour,
                   s_time_data.u8_minute);

          snprintf(date_buffer,
                   sizeof(date_buffer),
                   "%02d-%02d-%04d",
                   s_time_data.u8_day,
                   s_time_data.u8_month + 1,
                   s_time_data.u8_year + 1900);

          break;

        case DATA_SYNC_RESPONSE_ENROLL_FOUND_FINGERPRINT:
          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_SUCCESS)
          {
            lv_async_call(EVENT_FINGER_ShowSuccessStatus, NULL);
          }

          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_FAIL)
          {
            lv_async_call(EVENT_FINGER_ShowFailStatus, NULL);
          }
          break;

        case DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT:
          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_SUCCESS)
          {
            lv_async_call(EVENT_FINGER_ShowSuccessEnroll, NULL);
          }

          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_FAIL)
          {
            lv_async_call(EVENT_FINGER_ShowFailEnroll, NULL);
          }
          break;

        default:
          break;
      }
    }
  }
}

static void
EVENT_UPDATE_FINGER_ENROLL_TIME_Timer (lv_timer_t *timer)
{
  lv_label_set_text(ui_FingerEnrollTime, time_buffer);
}

static void
EVENT_FINGER_ShowSuccessStatus (void *user_data)
{
  lv_label_set_text(ui_IDTextEnroll5, "Remove/Place finger again");
  lv_obj_set_style_bg_color(ui_FingerStatus1,
                            lv_color_hex(0x00FF1C),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void
EVENT_FINGER_ShowFailStatus (void *user_data)
{
  lv_label_set_text(ui_IDTextEnroll5, "Not found finger");
  lv_obj_set_style_bg_color(ui_FingerStatus1,
                            lv_color_hex(0xFF0000),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void
EVENT_FINGER_ShowSuccessEnroll (void *user_data)
{
  *p_state_system = STATE_ENROLL_SUCCESS;
  lv_label_set_text(ui_IDTextEnroll5, "Enroll successfully");
  lv_obj_set_style_bg_color(ui_FingerStatus2,
                            lv_color_hex(0x00FF1C),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_async_call(EVENT_ENROLL_ShowHomeScreen, NULL);
}

static void
EVENT_FINGER_ShowFailEnroll (void *user_data)
{
  *p_state_system = STATE_ENROLL_FAIL;
  lv_label_set_text(ui_IDTextEnroll5, "Enroll unsuccessfully");
  lv_obj_set_style_bg_color(ui_FingerStatus2,
                            lv_color_hex(0xFF0000),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_async_call(EVENT_ENROLL_ShowHomeScreen, NULL);
}

static void
EVENT_ENROLL_ShowHomeScreen (void *param)
{
  lv_timer_pause(timer_update_time_finger_enroll);

  vTaskSuspend(s_finger_enroll_task_handle);

  _ui_screen_change(
      &ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);

  lv_async_call((lv_async_cb_t)EVENT_Home_Before, NULL);
}