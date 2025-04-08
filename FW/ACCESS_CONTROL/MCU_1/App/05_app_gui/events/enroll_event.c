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
 *  PRIVATE DEFINES
 *****************************************************************************/

/******************************************************************************
 *    PUBLIC DATA
 *****************************************************************************/

char    number_id[64]  = { 0 };
uint8_t number_id_send = 0;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_Enroll_Timer(lv_timer_t *timer);

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

static uint8_t enroll_index;

static lv_obj_t   *popup_invalid_id;
static lv_timer_t *timer_enroll;

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
EVENT_Menu_To_Enroll (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    b_is_initialize                       = true;
    s_enroll_event_data.p_send_data_queue = &s_data_system.s_send_data_queue;
    s_enroll_event_data.p_receive_data_event_queue
        = &s_data_system.s_receive_data_event_queue;
    timer_enroll = lv_timer_create(APP_Enroll_Timer, 30, NULL);
  }

  memset(number_id, 0, sizeof(number_id));
  enroll_index = 0;
  lv_label_set_text(ui_NumberUserID, number_id);
}

void
EVENT_Enroll_Button0 (lv_event_t *e)
{
  number_id[enroll_index] = '0';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button1 (lv_event_t *e)
{
  number_id[enroll_index] = '1';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button2 (lv_event_t *e)
{
  number_id[enroll_index] = '2';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button3 (lv_event_t *e)
{
  number_id[enroll_index] = '3';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button4 (lv_event_t *e)
{
  number_id[enroll_index] = '4';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button5 (lv_event_t *e)
{
  number_id[enroll_index] = '5';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button6 (lv_event_t *e)
{
  number_id[enroll_index] = '6';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button7 (lv_event_t *e)
{
  number_id[enroll_index] = '7';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button8 (lv_event_t *e)
{
  number_id[enroll_index] = '8';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_Button9 (lv_event_t *e)
{
  number_id[enroll_index] = '9';
  lv_label_set_text(ui_NumberUserID, number_id);
  enroll_index++;
}
void
EVENT_Enroll_DelButton (lv_event_t *e)
{
  enroll_index--;
  number_id[enroll_index] = 0;
  lv_label_set_text(ui_NumberUserID, number_id);
}
void
EVENT_Enroll_EnterButton (lv_event_t *e)
{
  number_id_send                = atoi(number_id);
  s_DATA_SYNC.u8_data_start     = DATA_SYNC_REQUEST_USER_DATA;
  s_DATA_SYNC.u8_data_packet[0] = number_id_send;
  s_DATA_SYNC.u8_data_length    = 1;
  s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
  xQueueSend(*s_enroll_event_data.p_send_data_queue, &s_DATA_SYNC, 0);
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/
static void
APP_Enroll_Timer (lv_timer_t *timer)
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
          // Invalid ID
          popup_invalid_id = lv_obj_create(ui_Enroll);
          lv_obj_set_width(popup_invalid_id, 280);
          lv_obj_set_height(popup_invalid_id, 180);
          lv_obj_set_align(popup_invalid_id, LV_ALIGN_CENTER);
          lv_obj_clear_flag(popup_invalid_id, LV_OBJ_FLAG_SCROLLABLE); /// Flags
          lv_obj_set_style_shadow_color(popup_invalid_id,
                                        lv_color_hex(0x969696),
                                        LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_shadow_opa(
              popup_invalid_id, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_shadow_width(
              popup_invalid_id, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_shadow_spread(
              popup_invalid_id, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_shadow_ofs_x(
              popup_invalid_id, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_shadow_ofs_y(
              popup_invalid_id, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        if (s_DATA_SYNC.u8_data_packet[0] == 0x01)
        {
          // Valid ID
          _ui_screen_change(&ui_UserInfo,
                            LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                            50,
                            0,
                            &ui_UserInfo_screen_init);
        }
        break;

      default:
        break;
    }
  }
}