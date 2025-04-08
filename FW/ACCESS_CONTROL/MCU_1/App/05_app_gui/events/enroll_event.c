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
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_ENROLL_DeletePopup_Timer(lv_timer_t *timer);
static void EVENT_PROCESS_ENROLL_DATA_CreateTask(void);
static void EVENT_PROCESS_ENROLL_DATA_Task(void *arg);

static void EVENT_ENROLL_ShowInvalidPopup(void *user_data);
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

static enroll_event_data_t s_enroll_event_data;
static DATA_SYNC_t         s_DATA_SYNC;

static char    enroll_number_id[64] = { 0 };
static uint8_t enroll_number_id_send;

static uint8_t enroll_index;
static char    full_name[64] = "";
static int     full_name_len = 0;
static int     packet_count  = 0;

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
    EVENT_PROCESS_ENROLL_DATA_CreateTask();
  }

  memset(enroll_number_id, 0, sizeof(enroll_number_id));
  enroll_index = 0;
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
}

void
EVENT_Enroll_Button0 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '0';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button1 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '1';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button2 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '2';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button3 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '3';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button4 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '4';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button5 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '5';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button6 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '6';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button7 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '7';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button8 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '8';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button9 (lv_event_t *e)
{
  enroll_number_id[enroll_index] = '9';
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
  enroll_index++;
}
void
EVENT_Enroll_DelButton (lv_event_t *e)
{
  enroll_index--;
  enroll_number_id[enroll_index] = 0;
  lv_label_set_text(ui_NumberUserID, enroll_number_id);
}
void
EVENT_Enroll_EnterButton (lv_event_t *e)
{
  enroll_number_id_send         = atoi(enroll_number_id);
  s_DATA_SYNC.u8_data_start     = DATA_SYNC_REQUEST_USER_DATA;
  s_DATA_SYNC.u8_data_packet[0] = (enroll_number_id_send << 8) & 0xFF;
  s_DATA_SYNC.u8_data_packet[1] = enroll_number_id_send & 0xFF;
  s_DATA_SYNC.u8_data_length    = 2;
  s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
  xQueueSend(*s_enroll_event_data.p_send_data_queue, &s_DATA_SYNC, 0);

  memset(full_name, 0, sizeof(full_name));
  full_name_len = 0;
  packet_count  = 0;
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/
static void
EVENT_PROCESS_ENROLL_DATA_CreateTask (void)
{
  xTaskCreate(EVENT_PROCESS_ENROLL_DATA_Task,
              "process enroll task",
              1024 * 4,
              NULL,
              6,
              NULL);
}

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
            lv_label_set_text(ui_FingerCheck, "0/1");
            lv_label_set_text(ui_FaceIDCheck, "0/1");
          }

          if (s_DATA_SYNC.u8_data_packet[0] == 0x02)
          {
            lv_label_set_text(ui_FingerCheck, "0/1");
            lv_label_set_text(ui_FaceIDCheck, "1/1");
          }

          if (s_DATA_SYNC.u8_data_packet[0] == 0x03)
          {
            lv_label_set_text(ui_FingerCheck, "1/1");
            lv_label_set_text(ui_FaceIDCheck, "0/1");
          }

          if (s_DATA_SYNC.u8_data_packet[0] == 0x04)
          {
            lv_label_set_text(ui_FingerCheck, "1/1");
            lv_label_set_text(ui_FaceIDCheck, "1/1");
          }

          break;

        case DATA_SYNC_RESPONSE_USERNAME_DATA:
          // Handle username data response -> Change screen to user info
          bool valid_char_added = false;

          if (s_DATA_SYNC.u8_data_packet[0] == '\n'
              && s_DATA_SYNC.u8_data_packet[1] == '\n')
          {
            // Change scrreen to user info
            lv_async_call(EVENT_ENROLL_ShowUserInfoScreen, NULL);
            break;
          }

          for (int i = 0; i < s_DATA_SYNC.u8_data_length; i++)
          {
            uint8_t byte = s_DATA_SYNC.u8_data_packet[i];

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
      = lv_timer_create(EVENT_ENROLL_DeletePopup_Timer, 1000, popup_enroll);
}

static void
EVENT_ENROLL_ShowUserInfoScreen (void *param)
{
  lv_label_set_text(ui_IDTextEnroll2, full_name);
  _ui_screen_change(&ui_UserInfo,
                    LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                    50,
                    0,
                    &ui_UserInfo_screen_init);
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