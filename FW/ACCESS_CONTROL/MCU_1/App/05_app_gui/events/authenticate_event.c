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
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_AUTHENTICATE_DeletePopupFail_Timer(lv_timer_t *timer);
static void EVENT_AUTHENTICATE_DeletePopupSuccess_Timer(lv_timer_t *timer);
static void EVENT_PROCESS_AUTHENTICATE_DATA_CreateTask(void);
static void EVENT_PROCESS_AUTHENTICATE_DATA_Task(void *arg);

static void EVENT_AUTHENTICATE_ShowSuccessPopup(void *user_data);
static void EVENT_AUTHENTICATE_ShowFailPopup(void *user_data);

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

static authenticate_event_data_t s_authenticate_event_data;
static DATA_SYNC_t               s_DATA_SYNC;

static lv_timer_t *timer_authenticate;

uint8_t authenticate_number_id_send = 1;

static bool b_is_initialize = false;

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
    EVENT_PROCESS_AUTHENTICATE_DATA_CreateTask();
  }

  s_DATA_SYNC.u8_data_start     = DATA_SYNC_REQUEST_AUTHENTICATION;
  s_DATA_SYNC.u8_data_packet[0] = (authenticate_number_id_send << 8) & 0xFF;
  s_DATA_SYNC.u8_data_packet[1] = authenticate_number_id_send & 0xFF;
  s_DATA_SYNC.u8_data_length    = 2;
  s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
  xQueueSend(*s_authenticate_event_data.p_send_data_queue, &s_DATA_SYNC, 0);
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/

static void
EVENT_PROCESS_AUTHENTICATE_DATA_CreateTask (void)
{
  xTaskCreate(EVENT_PROCESS_AUTHENTICATE_DATA_Task,
              "process authenticate task",
              1024 * 4,
              NULL,
              6,
              NULL);
}

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
        case DATA_SYNC_RESPONSE_AUTHENTICATION:

          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_SUCCESS)
          {
            printf("Authentication success\r\n");

            // lv_async_call(EVENT_AUTHENTICATE_ShowEnrollScreen, NULL);
            lv_async_call(EVENT_AUTHENTICATE_ShowSuccessPopup, NULL);
          }

          if (s_DATA_SYNC.u8_data_packet[0] == DATA_SYNC_FAIL)
          {
            lv_async_call(EVENT_AUTHENTICATE_ShowFailPopup, NULL);
          }

          break;

        default:
          break;
      }
    }
  }
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
      EVENT_AUTHENTICATE_DeletePopupFail_Timer, 1000, popup_authenticate);
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
      EVENT_AUTHENTICATE_DeletePopupSuccess_Timer, 1000, popup_authenticate);
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

  _ui_screen_change(
      &ui_Menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 50, 0, &ui_Menu_screen_init);
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

  EVENT_Authenticate_To_Enroll(NULL);

  _ui_screen_change(
      &ui_Enroll, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 50, 0, &ui_Enroll_screen_init);
}