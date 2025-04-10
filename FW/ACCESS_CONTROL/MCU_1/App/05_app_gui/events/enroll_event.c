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

extern char FaceIDCheck[4];
extern char FingerCheck[4];

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_ENROLL_DeletePopup_Timer(lv_timer_t *timer);
static void EVENT_ENROLL_DeletePopupSuccess_Timer(lv_timer_t *timer);

static void EVENT_PROCESS_ENROLL_DATA_Task(void *arg);

static void EVENT_ENROLL_ShowInvalidPopup(void *user_data);
static void EVENT_ENROLL_ShowSuccessPopup(void *user_data);
static void EVENT_ENROLL_ShowUserInfoScreen(void *param);

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t *p_send_data_queue;
  QueueHandle_t *p_receive_data_event_queue;
} enroll_event_data_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/
char       full_name[64] = "";
static int full_name_len = 0;
static int packet_count  = 0;

char           enroll_number_id[8] = { 0 };
uint16_t       user_id;
static uint8_t enroll_index;

static TaskHandle_t s_enroll_task_handle;

static enroll_event_data_t s_enroll_event_data;
static DATA_SYNC_t         s_DATA_SYNC;

static lv_obj_t   *ui_NumberUserID;
static lv_timer_t *timer_enroll;

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/
void
EVENT_Authenticate_To_Enroll (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    b_is_initialize                       = true;
    s_enroll_event_data.p_send_data_queue = &s_data_system.s_send_data_queue;
    s_enroll_event_data.p_receive_data_event_queue
        = &s_data_system.s_receive_data_event_queue;

    ui_NumberUserID = lv_label_create(ui_IDEnrollPane);
    lv_obj_set_width(ui_NumberUserID, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_NumberUserID, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_NumberUserID, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_NumberUserID,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_NumberUserID, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_NumberUserID,
                               &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    xTaskCreate(EVENT_PROCESS_ENROLL_DATA_Task,
                "process enroll task",
                1024 * 4,
                NULL,
                6,
                &s_enroll_task_handle);
  }

  vTaskResume(s_enroll_task_handle);

  memset(enroll_number_id, 0, sizeof(enroll_number_id));
  enroll_index = 0;
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
}

void
EVENT_Enroll_Button0 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '0';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button1 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '1';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button2 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '2';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button3 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '3';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button4 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '4';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button5 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '5';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button6 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '6';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button7 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '7';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button8 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '8';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button9 (lv_event_t *e)
{
  if (enroll_index >= 8)
  {
    return;
  }

  enroll_number_id[enroll_index] = '9';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_DelButton (lv_event_t *e)
{
  if (enroll_index == 0)
  {
    return;
  }

  enroll_index--;
  enroll_number_id[enroll_index] = 0;
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
}
void
EVENT_Enroll_EnterButton (lv_event_t *e)
{
  memset(full_name, 0, sizeof(full_name));
  full_name_len = 0;
  packet_count  = 0;

  vTaskResume(s_enroll_task_handle);

  user_id                       = atoi(enroll_number_id);
  s_DATA_SYNC.u8_data_start     = DATA_SYNC_REQUEST_USER_DATA;
  s_DATA_SYNC.u8_data_packet[0] = (user_id << 8) & 0xFF;
  s_DATA_SYNC.u8_data_packet[1] = user_id & 0xFF;
  s_DATA_SYNC.u8_data_length    = 2;
  s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
  xQueueSend(*s_enroll_event_data.p_send_data_queue, &s_DATA_SYNC, 0);
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/
static void
EVENT_PROCESS_ENROLL_DATA_Task (void *arg)
{
  while (1)
  {
    if (xQueueReceive(*s_enroll_event_data.p_receive_data_event_queue,
                      &s_DATA_SYNC,
                      portMAX_DELAY)
        == pdTRUE)
    {
      switch (s_DATA_SYNC.u8_data_start)
      {
        case DATA_SYNC_RESPONSE_USER_DATA:
          if (s_DATA_SYNC.u8_data_packet[0] == 0x00)
          {
            // Invalid ID -> Show popup
            lv_async_call(EVENT_ENROLL_ShowInvalidPopup, NULL);
          }

          if (s_DATA_SYNC.u8_data_packet[0] == 0x01)
          {
            memcpy(FaceIDCheck, "0/1", sizeof(FaceIDCheck));
            memcpy(FingerCheck, "0/1", sizeof(FingerCheck));
          }

          if (s_DATA_SYNC.u8_data_packet[0] == 0x02)
          {
            memcpy(FaceIDCheck, "1/1", sizeof(FaceIDCheck));
            memcpy(FingerCheck, "0/1", sizeof(FingerCheck));
          }

          if (s_DATA_SYNC.u8_data_packet[0] == 0x03)
          {
            memcpy(FaceIDCheck, "0/1", sizeof(FaceIDCheck));
            memcpy(FingerCheck, "1/1", sizeof(FingerCheck));
          }

          if (s_DATA_SYNC.u8_data_packet[0] == 0x04)
          {
            memcpy(FaceIDCheck, "1/1", sizeof(FaceIDCheck));
            memcpy(FingerCheck, "1/1", sizeof(FingerCheck));
          }

          break;

        case DATA_SYNC_RESPONSE_USERNAME_DATA:
          // Handle username data response -> Change screen to user info
          bool valid_char_added = false;

          if (s_DATA_SYNC.u8_data_packet[0] == '\n'
              && s_DATA_SYNC.u8_data_packet[1] == '\n')
          {
            // Change screen to user info
            lv_async_call(EVENT_ENROLL_ShowSuccessPopup, NULL);
            break;
          }

          for (int i = 0; i < s_DATA_SYNC.u8_data_length; i++)
          {
            uint8_t byte = s_DATA_SYNC.u8_data_packet[i];
            printf("byte: %02X\n", byte);

            if (byte == 0x00 || byte == 0xA5)
            {
              break;
            }

            if (byte >= 32 && byte <= 126)
            {
              if (packet_count > 0 && !valid_char_added
                  && full_name_len < sizeof(full_name) - 1)
              {
                full_name[full_name_len++] = ' ';
              }

              if (full_name_len < sizeof(full_name) - 1)
              {
                full_name[full_name_len++] = (char)byte;
                valid_char_added           = true;
              }
            }
          }

          if (valid_char_added)
          {
            packet_count++;
          }

          break;

        default:
          break;
      }
    }
  }
}

static void
EVENT_ENROLL_ShowSuccessPopup (void *user_data)
{
  if (timer_enroll != NULL)
  {
    lv_timer_del(timer_enroll);
    timer_enroll = NULL;
  }

  lv_obj_t *popup_enroll = lv_obj_create(ui_Enroll);
  lv_obj_set_size(popup_enroll, 280, 180);
  lv_obj_set_align(popup_enroll, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_enroll, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_enroll, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_enroll, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_enroll, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_enroll, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_enroll, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_enroll, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_enroll);
  lv_label_set_text(label_invalid_id, "Valid ID");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0x00A00B), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_enroll = lv_timer_create(
      EVENT_ENROLL_DeletePopupSuccess_Timer, 700, popup_enroll);
}

static void
EVENT_ENROLL_ShowInvalidPopup (void *user_data)
{
  if (timer_enroll != NULL)
  {
    lv_timer_del(timer_enroll);
    timer_enroll = NULL;
  }

  lv_obj_t *popup_enroll = lv_obj_create(ui_Enroll);
  lv_obj_set_size(popup_enroll, 280, 180);
  lv_obj_set_align(popup_enroll, LV_ALIGN_CENTER);
  lv_obj_clear_flag(popup_enroll, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_shadow_color(
      popup_enroll, lv_color_hex(0x969696), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(popup_enroll, 255, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(popup_enroll, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(popup_enroll, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_x(popup_enroll, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_ofs_y(popup_enroll, 2, LV_PART_MAIN);

  lv_obj_t *label_invalid_id = lv_label_create(popup_enroll);
  lv_label_set_text(label_invalid_id, "Invalid ID");
  lv_obj_center(label_invalid_id);
  lv_obj_set_style_text_color(
      label_invalid_id, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_text_font(
      label_invalid_id, &lv_font_montserrat_18, LV_PART_MAIN);

  timer_enroll
      = lv_timer_create(EVENT_ENROLL_DeletePopup_Timer, 700, popup_enroll);
}

static void
EVENT_ENROLL_DeletePopup_Timer (lv_timer_t *timer)
{
  lv_obj_t *popup_enroll = (lv_obj_t *)timer->user_data;

  if (popup_enroll && lv_obj_is_valid(popup_enroll))
  {
    lv_obj_del(popup_enroll);
  }

  lv_timer_del(timer);

  if (timer_enroll == timer)
  {
    timer_enroll = NULL;
  }
}

static void
EVENT_ENROLL_DeletePopupSuccess_Timer (lv_timer_t *timer)
{
  lv_obj_t *popup_enroll = (lv_obj_t *)timer->user_data;

  if (popup_enroll && lv_obj_is_valid(popup_enroll))
  {
    lv_obj_del(popup_enroll);
  }

  lv_timer_del(timer);

  if (timer_enroll == timer)
  {
    timer_enroll = NULL;
  }

  lv_async_call(EVENT_ENROLL_ShowUserInfoScreen, NULL);
}

static void
EVENT_ENROLL_ShowUserInfoScreen (void *param)
{
  vTaskSuspend(s_enroll_task_handle);

  if (!lv_obj_is_valid(ui_UserInfo))
  {
    printf("ui_UserInfo is not valid!");
    return;
  }

  _ui_screen_change(&ui_UserInfo,
                    LV_SCR_LOAD_ANIM_MOVE_LEFT,
                    500,
                    0,
                    &ui_UserInfo_screen_init);

  lv_async_call((lv_async_cb_t)EVENT_Enroll_To_UserInfo, NULL);
}