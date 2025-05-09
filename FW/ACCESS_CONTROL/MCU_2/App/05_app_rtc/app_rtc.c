/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <time.h>

#include "app_rtc.h"
#include "app_data.h"
#include "ds3231.h"

#include "esp_log.h"

/******************************************************************************
 *    PUBLIC VARIABLES
 *****************************************************************************/

extern struct tm timeinfo;

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_RTC"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  ds3231_data_t       s_ds3231_data;
  QueueHandle_t      *p_send_data_queue;
  EventGroupHandle_t *p_flag_time_event;
} handle_rtc_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_rtc_t s_handle_rtc;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_RTC_Task(void *arg);
static void convert_ds3231_to_tm(const ds3231_data_t *src, struct tm *dest);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_RTC_CreateTask (void)
{
  xTaskCreate(APP_RTC_Task, "RTC task", 1024 * 4, NULL, 7, NULL);
}

void
APP_RTC_Init (void)
{
  s_handle_rtc.p_send_data_queue = &s_data_system.s_send_data_queue;
  s_handle_rtc.p_flag_time_event = &s_data_system.s_flag_time_event;

  xEventGroupSetBits(*s_handle_rtc.p_flag_time_event, TIME_SOURCE_RTC_READY);

  DEV_DS3231_Init(&s_handle_rtc.s_ds3231_data, I2C_NUM_0);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_RTC_Task (void *arg)
{
  DATA_SYNC_t s_DATA_SYNC;
  EventBits_t uxBit;
  while (1)
  {
    uxBit = xEventGroupWaitBits(*s_handle_rtc.p_flag_time_event,
                                TIME_SOURCE_RTC_READY | TIME_SOURCE_RTC_OFF,
                                pdFALSE,
                                pdFALSE,
                                0);

    if (uxBit & TIME_SOURCE_RTC_READY)
    {
      DEV_DS3231_Register_Read(&s_handle_rtc.s_ds3231_data, I2C_NUM);

      s_DATA_SYNC.u8_data_start     = DATA_SYNC_TIME;
      s_DATA_SYNC.u8_data_packet[0] = s_handle_rtc.s_ds3231_data.u8_minute;
      s_DATA_SYNC.u8_data_packet[1] = s_handle_rtc.s_ds3231_data.u8_hour;
      s_DATA_SYNC.u8_data_packet[2] = s_handle_rtc.s_ds3231_data.u8_date;
      s_DATA_SYNC.u8_data_packet[3] = s_handle_rtc.s_ds3231_data.u8_month;
      s_DATA_SYNC.u8_data_packet[4] = s_handle_rtc.s_ds3231_data.u8_year;
      s_DATA_SYNC.u8_data_length    = 5;
      s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

      xQueueSend(*s_handle_rtc.p_send_data_queue, &s_DATA_SYNC, 0);
      printf("RTC time: %02d:%02d:%02d %02d/%02d/%02d\r\n",
             s_handle_rtc.s_ds3231_data.u8_hour,
             s_handle_rtc.s_ds3231_data.u8_minute,
             s_handle_rtc.s_ds3231_data.u8_second,
             s_handle_rtc.s_ds3231_data.u8_date,
             s_handle_rtc.s_ds3231_data.u8_month + 1,
             s_handle_rtc.s_ds3231_data.u8_year + 1900);
    }

    if (uxBit & TIME_SOURCE_RTC_OFF)
    {
      // Sync SNTP time to RTC
      s_handle_rtc.s_ds3231_data.u8_hour   = timeinfo.tm_hour;
      s_handle_rtc.s_ds3231_data.u8_minute = timeinfo.tm_min;
      s_handle_rtc.s_ds3231_data.u8_date   = timeinfo.tm_mday;
      s_handle_rtc.s_ds3231_data.u8_month  = timeinfo.tm_mon;
      s_handle_rtc.s_ds3231_data.u8_year   = timeinfo.tm_year;
      s_handle_rtc.s_ds3231_data.u8_day    = timeinfo.tm_wday;

      DEV_DS3231_Register_Write(&s_handle_rtc.s_ds3231_data, I2C_NUM);
      xEventGroupClearBits(*s_handle_rtc.p_flag_time_event,
                           TIME_SOURCE_RTC_OFF);

      printf("RTC time sync: %02d:%02d:%02d %02d/%02d/%02d\r\n",
             s_handle_rtc.s_ds3231_data.u8_hour,
             s_handle_rtc.s_ds3231_data.u8_minute,
             s_handle_rtc.s_ds3231_data.u8_second,
             s_handle_rtc.s_ds3231_data.u8_date,
             s_handle_rtc.s_ds3231_data.u8_month + 1,
             s_handle_rtc.s_ds3231_data.u8_year + 1900);
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

static void
convert_ds3231_to_tm (const ds3231_data_t *src, struct tm *dest)
{
  dest->tm_sec   = src->u8_second;     // 0-59
  dest->tm_min   = src->u8_minute;     // 0-59
  dest->tm_hour  = src->u8_hour;       // 0-23 (assumes 24-hour format)
  dest->tm_mday  = src->u8_date;       // 1-31
  dest->tm_mon   = src->u8_month - 1;  // Adjust to 0-11
  dest->tm_year  = src->u8_year + 100; // Years since 1900 (2000-2099)
  dest->tm_wday  = src->u8_day - 1;    // Adjust to 0-6 (Sunday=0)
  dest->tm_yday  = 0;                  // Not provided by DS3231
  dest->tm_isdst = -1;                 // No daylight saving time info
}