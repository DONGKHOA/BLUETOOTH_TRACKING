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

char FingerCheck[4];
char FaceIDCheck[4];

extern char full_name[64];
extern char enroll_number_id[8];

extern char time_buffer[8];
extern char date_buffer[16];

extern time_data_t s_time_data;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_USERINFO_DeletePopup_Timer(lv_timer_t *timer);
static void EVENT_UPDATE_USERINFO_TIME_Timer(lv_timer_t *timer);

static void EVENT_PROCESS_USERINFO_DATA_Task(void *arg);
static void EVENT_USERINFO_ShowInvalidPopup(void *user_data);
static void EVENT_USERINFO_ShowFingerEnrollScreen(void *param);
static void EVENT_USERINFO_ShowFaceIDEnrollScreen(void *param);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static lv_obj_t   *ui_UserInfoTime;
static lv_obj_t   *ui_IDTextEnroll2;
static lv_obj_t   *ui_NumberID;
static lv_obj_t   *ui_FingerCheck;
static lv_obj_t   *ui_FaceIDCheck;
static lv_timer_t *timer_userinfo;
static lv_timer_t *timer_update_time_userinfo;

static QueueHandle_t *p_receive_data_event_queue;

static TaskHandle_t s_userinfo_task_handle;

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/
void
EVENT_Enroll_To_UserInfo (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    b_is_initialize            = true;
    p_receive_data_event_queue = &s_data_system.s_receive_data_event_queue;

    ui_UserInfoTime = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_UserInfoTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_UserInfoTime, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_UserInfoTime, 23);
    lv_obj_set_y(ui_UserInfoTime, -105);
    lv_obj_set_align(ui_UserInfoTime, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_color(ui_UserInfoTime,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_UserInfoTime, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_UserInfoTime,
                               &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_IDTextEnroll2 = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_IDTextEnroll2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_IDTextEnroll2, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_IDTextEnroll2, -25);
    lv_obj_set_y(ui_IDTextEnroll2, -75);
    lv_obj_set_align(ui_IDTextEnroll2, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_IDTextEnroll2,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_IDTextEnroll2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NumberID = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_NumberID, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_NumberID, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_NumberID, -109);
    lv_obj_set_y(ui_NumberID, -74);
    lv_obj_set_align(ui_NumberID, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(
        ui_NumberID, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_NumberID, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_FingerCheck = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_FingerCheck, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_FingerCheck, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_FingerCheck, -35);
    lv_obj_set_y(ui_FingerCheck, -41);
    lv_obj_set_align(ui_FingerCheck, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_FingerCheck,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_FingerCheck, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_FaceIDCheck = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_FaceIDCheck, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_FaceIDCheck, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_FaceIDCheck, -35);
    lv_obj_set_y(ui_FaceIDCheck, -4);
    lv_obj_set_align(ui_FaceIDCheck, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_FaceIDCheck,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_FaceIDCheck, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    timer_update_time_userinfo
        = lv_timer_create(EVENT_UPDATE_USERINFO_TIME_Timer, 1000, NULL);

    xTaskCreate(EVENT_PROCESS_USERINFO_DATA_Task,
                "process userinfo task",
                1024 * 4,
                NULL,
                6,
                &s_userinfo_task_handle);
  }
  else
  {
    lv_timer_resume(timer_update_time_userinfo);
    vTaskResume(s_userinfo_task_handle);
  }

  lv_label_set_text(ui_UserInfoTime, time_buffer);

  lv_label_set_text(ui_IDTextEnroll2, full_name);
  lv_label_set_text(ui_NumberID, enroll_number_id);

  lv_label_set_text(ui_FingerCheck, FingerCheck);
  lv_label_set_text(ui_FaceIDCheck, FaceIDCheck);
}

void
EVENT_UserInfo_Finger (lv_event_t *e)
{
  if (strcmp(FingerCheck, "1/1") != 0)
  {
    lv_async_call(EVENT_USERINFO_ShowFingerEnrollScreen, NULL);
  }
  else
  {
    lv_async_call(EVENT_USERINFO_ShowInvalidPopup, NULL);
  }
}

void EVENT_UserInfo_FaceID (lv_event_t *e)
{
  if (strcmp(FaceIDCheck, "1/1") != 0)
  {
    lv_async_call(EVENT_USERINFO_ShowFaceIDEnrollScreen, NULL);
  }
  else
  {
    lv_async_call(EVENT_USERINFO_ShowInvalidPopup, NULL);
  }
}

void
EVENT_UserInfo_Back (lv_event_t *e)
{
  lv_timer_pause(timer_update_time_userinfo);
  vTaskSuspend(s_userinfo_task_handle);
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/

static void
EVENT_PROCESS_USERINFO_DATA_Task (void *arg)
{
  DATA_SYNC_t s_DATA_SYNC;

  while (1)
  {
    if (xQueueReceive(*p_receive_data_event_queue, &s_DATA_SYNC, portMAX_DELAY)
        == pdPASS)
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

        default:
          break;
      }
    }
  }
}

static void
EVENT_UPDATE_USERINFO_TIME_Timer (lv_timer_t *timer)
{
  lv_label_set_text(ui_UserInfoTime, time_buffer);
}

static void
EVENT_USERINFO_ShowInvalidPopup (void *user_data)
{
  if (timer_userinfo != NULL)
  {
    lv_timer_del(timer_userinfo);
    timer_userinfo = NULL;
  }

  lv_obj_t *popup_userinfo = lv_obj_create(ui_UserInfo);
  lv_obj_set_size(popup_userinfo, 280, 180);
  lv_obj_set_align(popup_userinfo, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_userinfo, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_userinfo, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_userinfo, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_userinfo, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_userinfo, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_userinfo, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_userinfo, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_userinfo);
  lv_label_set_text(label_invalid_id, "Delete before enroll");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_userinfo
      = lv_timer_create(EVENT_USERINFO_DeletePopup_Timer, 700, popup_userinfo);
}

static void
EVENT_USERINFO_DeletePopup_Timer (lv_timer_t *timer)
{
  lv_obj_t *popup_userinfo = (lv_obj_t *)timer->user_data;

  if (popup_userinfo && lv_obj_is_valid(popup_userinfo))
  {
    lv_obj_del(popup_userinfo);
  }

  lv_timer_del(timer);

  if (timer_userinfo == timer)
  {
    timer_userinfo = NULL;
  }
}

static void
EVENT_USERINFO_ShowFingerEnrollScreen (void *param)
{
  lv_timer_pause(timer_update_time_userinfo);

  vTaskSuspend(s_userinfo_task_handle);

  if (!lv_obj_is_valid(ui_FingerEnroll))
  {
    printf("ui_FingerEnroll is not valid!");
    return;
  }

  _ui_screen_change(&ui_FingerEnroll,
                    LV_SCR_LOAD_ANIM_MOVE_LEFT,
                    500,
                    0,
                    &ui_FingerEnroll_screen_init);

  lv_async_call((lv_async_cb_t)EVENT_Enroll_Finger, NULL);
}

static void
EVENT_USERINFO_ShowFaceIDEnrollScreen (void *param)
{
  lv_timer_pause(timer_update_time_userinfo);

  vTaskSuspend(s_userinfo_task_handle);

  if (!lv_obj_is_valid(ui_FaceIDEnroll))
  {
    printf("ui_FaceIDEnroll is not valid!");
    return;
  }

  _ui_screen_change(&ui_FaceIDEnroll,
                    LV_SCR_LOAD_ANIM_MOVE_LEFT,
                    500,
                    0,
                    &ui_FaceIDEnroll_screen_init);

  lv_async_call((lv_async_cb_t)EVENT_Enroll_FaceID, NULL);
}