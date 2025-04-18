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

#include "esp_log.h"

/******************************************************************************
 *  PUBLIC VARIABLES
 *****************************************************************************/

extern char time_buffer[8];
extern char date_buffer[16];

extern time_data_t s_time_data;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_UPDATE_AUTHENTICATE_TIME_Timer(lv_timer_t *timer);

static void EVENT_AUTHENTICATE_DeletePopupFail_Timer(lv_timer_t *timer);
static void EVENT_AUTHENTICATE_DeletePopupSuccess_Timer(lv_timer_t *timer);
static void EVENT_AUTHENTICATE_DeletePopupPasswordCorrect_Timer(
    lv_timer_t *timer);
static void EVENT_AUTHENTICATE_DeletePopup_Timer(lv_timer_t *timer);

static void EVENT_PROCESS_AUTHENTICATE_DATA_Task(void *arg);

static void EVENT_AUTHENTICATE_ShowSuccessPopup(void *user_data);
static void EVENT_AUTHENTICATE_ShowFailPopup(void *user_data);

static void EVENT_AUTHENTICATE_ShowPasswordCorrectPopup(void *user_data);
static void EVENT_AUTHENTICATE_ShowPasswordIncorrectPopup(void *user_data);

static void EVENT_AUTHENTICATE_ShowEnrollScreen(void *param);

static void EVENT_AUTHENTICATE_ShowInvalidPopup(void *user_data);

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t *p_send_data_queue;
  QueueHandle_t *p_receive_data_event_queue;
} authenticate_event_data_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static TaskHandle_t s_authenticate_task_handle;

static lv_obj_t   *ui_AuthenticateTime;
static lv_obj_t   *ui_PassIDAuthenticateText;
static lv_obj_t   *ui_NumberPassIDAuthenticateText;
static lv_timer_t *timer_authenticate;
static lv_timer_t *timer_update_time_authenticate;

static DATA_SYNC_t               s_DATA_SYNC;
static authenticate_event_data_t s_authenticate_event_data;

static char    authenticate_number_id[8] = { 0 };
static uint8_t authenticate_number_id_send;
static uint8_t authenticate_index;
static int     authenticate_password = 0;
static bool    b_is_initialize       = false;
static bool    b_is_authenticate     = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/
void
EVENT_Menu_To_Authenticate (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    b_is_initialize = true;
    s_authenticate_event_data.p_send_data_queue
        = &s_data_system.s_send_data_queue;
    s_authenticate_event_data.p_receive_data_event_queue
        = &s_data_system.s_receive_data_event_queue;

    ui_AuthenticateTime = lv_label_create(ui_Authenticate);
    lv_obj_set_width(ui_AuthenticateTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_AuthenticateTime, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_AuthenticateTime, 23);
    lv_obj_set_y(ui_AuthenticateTime, -105);
    lv_obj_set_align(ui_AuthenticateTime, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_color(ui_AuthenticateTime,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_AuthenticateTime, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_AuthenticateTime,
                               &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_PassIDAuthenticateText = lv_label_create(ui_PassIDAuthenticatePanel);
    lv_obj_set_width(ui_PassIDAuthenticateText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_PassIDAuthenticateText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_PassIDAuthenticateText, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_color(ui_PassIDAuthenticateText,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_PassIDAuthenticateText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_PassIDAuthenticateText,
                               &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NumberPassIDAuthenticateText
        = lv_label_create(ui_PassIDAuthenticatePanel);
    lv_obj_set_width(ui_NumberPassIDAuthenticateText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_NumberPassIDAuthenticateText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_NumberPassIDAuthenticateText, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_NumberPassIDAuthenticateText,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_NumberPassIDAuthenticateText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_NumberPassIDAuthenticateText,
                               &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    timer_update_time_authenticate
        = lv_timer_create(EVENT_UPDATE_AUTHENTICATE_TIME_Timer, 1000, NULL);

    xTaskCreate(EVENT_PROCESS_AUTHENTICATE_DATA_Task,
                "process authenticate task",
                1024 * 4,
                NULL,
                6,
                &s_authenticate_task_handle);
  }
  else
  {
    lv_timer_resume(timer_update_time_authenticate);
    vTaskResume(s_authenticate_task_handle);
  }

  b_is_authenticate = false;

  lv_label_set_text(ui_PassIDAuthenticateText, "ID:");
  lv_label_set_text(ui_AuthenticateTime, time_buffer);

  memset(authenticate_number_id, 0, sizeof(authenticate_number_id));
  authenticate_index = 0;
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
}

void
EVENT_Authenticate_After (lv_event_t *e)
{
  lv_timer_pause(timer_update_time_authenticate);
  vTaskSuspend(s_authenticate_task_handle);
}

void
EVENT_Authenticate_Button0 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '0';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button1 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '1';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button2 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '2';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button3 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '3';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button4 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '4';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button5 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '5';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button6 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '6';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button7 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '7';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button8 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '8';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_Button9 (lv_event_t *e)
{
  if (authenticate_index >= 8)
  {
    return;
  }

  authenticate_number_id[authenticate_index] = '9';
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
  authenticate_index++;
}

void
EVENT_Authenticate_DelButton (lv_event_t *e)
{
  if (authenticate_index == 0)
  {
    return;
  }

  authenticate_index--;
  authenticate_number_id[authenticate_index] = 0;
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
}

void
EVENT_Authenticate_EnterButton (lv_event_t *e)
{
  if (!b_is_authenticate)
  {
    authenticate_number_id_send   = atoi(authenticate_number_id);
    s_DATA_SYNC.u8_data_start     = DATA_SYNC_REQUEST_AUTHENTICATION;
    s_DATA_SYNC.u8_data_packet[0] = (authenticate_number_id_send << 8) & 0xFF;
    s_DATA_SYNC.u8_data_packet[1] = authenticate_number_id_send & 0xFF;
    s_DATA_SYNC.u8_data_length    = 2;
    s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
    xQueueSend(*s_authenticate_event_data.p_send_data_queue, &s_DATA_SYNC, 0);
  }
  else
  {
    authenticate_password = atoi(authenticate_number_id);
    if (authenticate_password == AUTHENTICATE_PASSWORD)
    {
      b_is_authenticate = false;
      lv_async_call(EVENT_AUTHENTICATE_ShowPasswordCorrectPopup, NULL);
    }
    else
    {
      // Invalid Password -> Show popup
      lv_async_call(EVENT_AUTHENTICATE_ShowPasswordIncorrectPopup, NULL);
    }
  }
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/
static void
EVENT_PROCESS_AUTHENTICATE_DATA_Task (void *arg)
{
  while (1)
  {
    if (xQueueReceive(*s_authenticate_event_data.p_receive_data_event_queue,
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

        case DATA_SYNC_RESPONSE_AUTHENTICATION:

          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_SUCCESS)
          {
            // Enter Password to access enroll screen if authenticator is admin
            b_is_authenticate = true;

            lv_async_call(EVENT_AUTHENTICATE_ShowSuccessPopup, NULL);
          }

          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_FAIL)
          {
            b_is_authenticate = false;
            lv_async_call(EVENT_AUTHENTICATE_ShowFailPopup, NULL);
          }

          break;

        case DATA_SYNC_RESPONSE_USER_DATA:
          if (s_DATA_SYNC.u8_data_packet[0] == 0x00)
          {
            // Invalid ID -> Show popup
            lv_async_call(EVENT_AUTHENTICATE_ShowInvalidPopup, NULL);
          }

          break;

        default:
          break;
      }
    }
  }
}

static void
EVENT_UPDATE_AUTHENTICATE_TIME_Timer (lv_timer_t *timer)
{
  lv_label_set_text(ui_AuthenticateTime, time_buffer);
}

static void
EVENT_AUTHENTICATE_ShowInvalidPopup (void *user_data)
{
  if (timer_authenticate != NULL)
  {
    lv_timer_del(timer_authenticate);
    timer_authenticate = NULL;
  }

  lv_obj_t *popup_authenticate = lv_obj_create(ui_Authenticate);
  lv_obj_set_size(popup_authenticate, 280, 180);
  lv_obj_set_align(popup_authenticate, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_authenticate, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_authenticate, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_authenticate, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_authenticate, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_authenticate);
  lv_label_set_text(label_invalid_id, "Invalid ID");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_authenticate = lv_timer_create(
      EVENT_AUTHENTICATE_DeletePopup_Timer, 700, popup_authenticate);
}

static void
EVENT_AUTHENTICATE_ShowPasswordCorrectPopup (void *user_data)
{
  if (timer_authenticate != NULL)
  {
    lv_timer_del(timer_authenticate);
    timer_authenticate = NULL;
  }

  lv_obj_t *popup_authenticate = lv_obj_create(ui_Authenticate);
  lv_obj_set_size(popup_authenticate, 280, 180);
  lv_obj_set_align(popup_authenticate, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_authenticate, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_authenticate, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_authenticate, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_authenticate, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_authenticate);
  lv_label_set_text(label_invalid_id, "Password Correct");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0x00A00B), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_authenticate
      = lv_timer_create(EVENT_AUTHENTICATE_DeletePopupPasswordCorrect_Timer,
                        700,
                        popup_authenticate);
}

static void
EVENT_AUTHENTICATE_ShowPasswordIncorrectPopup (void *user_data)
{
  if (timer_authenticate != NULL)
  {
    lv_timer_del(timer_authenticate);
    timer_authenticate = NULL;
  }

  lv_obj_t *popup_authenticate = lv_obj_create(ui_Authenticate);
  lv_obj_set_size(popup_authenticate, 280, 180);
  lv_obj_set_align(popup_authenticate, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_authenticate, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_authenticate, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_authenticate, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_authenticate, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_authenticate);
  lv_label_set_text(label_invalid_id, "Password Incorrect");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_authenticate = lv_timer_create(
      EVENT_AUTHENTICATE_DeletePopupFail_Timer, 700, popup_authenticate);
}
static void
EVENT_AUTHENTICATE_ShowFailPopup (void *user_data)
{
  if (timer_authenticate != NULL)
  {
    lv_timer_del(timer_authenticate);
    timer_authenticate = NULL;
  }

  lv_obj_t *popup_authenticate = lv_obj_create(ui_Authenticate);
  lv_obj_set_size(popup_authenticate, 280, 180);
  lv_obj_set_align(popup_authenticate, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_authenticate, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_authenticate, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_authenticate, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_authenticate, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_authenticate);
  lv_label_set_text(label_invalid_id, "Authentication Failed");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_authenticate = lv_timer_create(
      EVENT_AUTHENTICATE_DeletePopupFail_Timer, 700, popup_authenticate);
}

static void
EVENT_AUTHENTICATE_ShowSuccessPopup (void *user_data)
{
  if (timer_authenticate != NULL)
  {
    lv_timer_del(timer_authenticate);
    timer_authenticate = NULL;
  }

  lv_obj_t *popup_authenticate = lv_obj_create(ui_Authenticate);
  lv_obj_set_size(popup_authenticate, 280, 180);
  lv_obj_set_align(popup_authenticate, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_authenticate, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_authenticate, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_authenticate, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_authenticate, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_authenticate, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_authenticate);
  lv_label_set_text(label_invalid_id, "Authentication Success");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0x00A00B), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_authenticate = lv_timer_create(
      EVENT_AUTHENTICATE_DeletePopupSuccess_Timer, 700, popup_authenticate);
}

static void
EVENT_AUTHENTICATE_DeletePopup_Timer (lv_timer_t *timer)
{
  lv_obj_t *popup_authenticate = (lv_obj_t *)timer->user_data;

  if (popup_authenticate && lv_obj_is_valid(popup_authenticate))
  {
    lv_obj_del(popup_authenticate);
  }

  lv_timer_del(timer);

  if (timer_authenticate == timer)
  {
    timer_authenticate = NULL;
  }
}

static void
EVENT_AUTHENTICATE_DeletePopupFail_Timer (lv_timer_t *timer)
{
  lv_obj_t *popup_authenticate = (lv_obj_t *)timer->user_data;

  if (popup_authenticate && lv_obj_is_valid(popup_authenticate))
  {
    lv_obj_del(popup_authenticate);
  }

  lv_timer_del(timer);

  if (timer_authenticate == timer)
  {
    timer_authenticate = NULL;
  }
}

static void
EVENT_AUTHENTICATE_DeletePopupSuccess_Timer (lv_timer_t *timer)
{
  lv_obj_t *popup_authenticate = (lv_obj_t *)timer->user_data;

  if (popup_authenticate && lv_obj_is_valid(popup_authenticate))
  {
    lv_obj_del(popup_authenticate);
  }

  lv_timer_del(timer);

  if (timer_authenticate == timer)
  {
    timer_authenticate = NULL;
  }

  lv_label_set_text(ui_PassIDAuthenticateText, "Password:");

  memset(authenticate_number_id, 0, sizeof(authenticate_number_id));
  authenticate_index = 0;
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);
}

static void
EVENT_AUTHENTICATE_DeletePopupPasswordCorrect_Timer (lv_timer_t *timer)
{
  lv_obj_t *popup_authenticate = (lv_obj_t *)timer->user_data;

  if (popup_authenticate && lv_obj_is_valid(popup_authenticate))
  {
    lv_obj_del(popup_authenticate);
  }

  lv_timer_del(timer);

  if (timer_authenticate == timer)
  {
    timer_authenticate = NULL;
  }

  memset(authenticate_number_id, 0, sizeof(authenticate_number_id));
  authenticate_index = 0;
  lv_label_set_text(ui_NumberPassIDAuthenticateText, authenticate_number_id);

  lv_async_call(EVENT_AUTHENTICATE_ShowEnrollScreen, NULL);
}

static void
EVENT_AUTHENTICATE_ShowEnrollScreen (void *param)
{
  lv_timer_pause(timer_update_time_authenticate);

  vTaskSuspend(s_authenticate_task_handle);

  _ui_screen_change(
      &ui_Enroll, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, &ui_Enroll_screen_init);

  lv_async_call((lv_async_cb_t)EVENT_Authenticate_To_Enroll, NULL);
}