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
 *    PUBLIC VARIABLES
 *****************************************************************************/

char time_buffer[8];
char date_buffer[16];

time_data_t s_time_data = { 0 };

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_PROCESS_HOME_DATA_Task(void *arg);
static void EVENT_UPDATE_HOME_TIME_Timer(lv_timer_t *timer);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static lv_obj_t   *ui_HomeTime;
static lv_obj_t   *ui_HomeDay;
static lv_timer_t *timer_home;

static QueueHandle_t *p_receive_data_event_queue;

static TaskHandle_t s_home_task_handle;

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
EVENT_Home_Before (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    b_is_initialize            = true;
    p_receive_data_event_queue = &s_data_system.s_receive_data_event_queue;

    ui_HomeTime = lv_label_create(ui_POPUPHomePanel);
    lv_obj_set_width(ui_HomeTime, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_HomeTime, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_HomeTime, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(
        ui_HomeTime, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_HomeTime, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(
        ui_HomeTime, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_HomeDay = lv_label_create(ui_Home);
    lv_obj_set_width(ui_HomeDay, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_HomeDay, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_HomeDay, 23);
    lv_obj_set_y(ui_HomeDay, 108);
    lv_obj_set_align(ui_HomeDay, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_color(
        ui_HomeDay, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_HomeDay, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(
        ui_HomeDay, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    timer_home = lv_timer_create(EVENT_UPDATE_HOME_TIME_Timer, 30, NULL);

    xTaskCreate(EVENT_PROCESS_HOME_DATA_Task,
                "process home task",
                1024 * 4,
                NULL,
                6,
                &s_home_task_handle);
  }
  else
  {
    lv_timer_resume(timer_home);
    vTaskResume(s_home_task_handle);
  }

  lv_label_set_text(ui_HomeTime, time_buffer);
  lv_label_set_text(ui_HomeDay, date_buffer);
}

void
EVENT_Home_After (lv_event_t *e)
{
  lv_timer_pause(timer_home);
  vTaskSuspend(s_home_task_handle);
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/

static void
EVENT_PROCESS_HOME_DATA_Task (void *arg)
{
  DATA_SYNC_t s_DATA_SYNC;

  while (1)
  {
    if (xQueueReceive(*p_receive_data_event_queue, &s_DATA_SYNC, portMAX_DELAY)
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

        default:
          break;
      }
    }
  }
}

static void
EVENT_UPDATE_HOME_TIME_Timer (lv_timer_t *timer)
{
  lv_label_set_text(ui_HomeTime, time_buffer);
  lv_label_set_text(ui_HomeDay, date_buffer);
}