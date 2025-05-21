/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_time_system.h"
#include "app_data.h"

#include <sys/time.h>
#include "esp_sntp.h"
#include "esp_log.h"

#include "ds3231.h"

/******************************************************************************
 *    PUBLIC DATA
 *****************************************************************************/

struct tm timeinfo;
time_t    now;

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_TIME_SYSTEM"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  ds3231_data_t       s_ds3231_data;
  QueueHandle_t      *p_send_data_queue;
  EventGroupHandle_t *p_flag_time_event;
  SemaphoreHandle_t  *p_time_mutex;
} handle_rtc_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_rtc_t s_time_system;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_TIME_SYSTEM_Task(void *arg);
static void APP_TIME_SYSTEM_Notification_cb(struct timeval *tv);
static void convert_ds3231_to_tm(const ds3231_data_t *src, struct tm *dest);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_TIME_SYSTEM_CreateTask (void)
{
  xTaskCreate(APP_TIME_SYSTEM_Task, "RTC task", 1024 * 4, NULL, 7, NULL);
}

void
APP_TIME_SYSTEM_Init (void)
{
  s_time_system.p_send_data_queue = &s_data_system.s_send_data_queue;
  s_time_system.p_flag_time_event = &s_data_system.s_flag_time_event;
  s_time_system.p_time_mutex      = &s_data_system.s_time_mutex;

  DEV_DS3231_Init(&s_time_system.s_ds3231_data, I2C_NUM);

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_set_time_sync_notification_cb(APP_TIME_SYSTEM_Notification_cb);
  esp_sntp_init();

  xEventGroupSetBits(*s_time_system.p_flag_time_event, TIME_SOURCE_RTC_READY);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_TIME_SYSTEM_Notification_cb (struct timeval *tv)
{
  ESP_LOGI(TAG, "Time synchronized\n");

  time(&now);
  now += 7 * 3600;
  localtime_r(&now, &timeinfo);

  xEventGroupClearBits(*s_time_system.p_flag_time_event, TIME_SOURCE_RTC_READY);
  xEventGroupSetBits(*s_time_system.p_flag_time_event, TIME_SOURCE_RTC_OFF);
}

static void
APP_TIME_SYSTEM_Task (void *arg)
{
  DATA_SYNC_t s_DATA_SYNC;
  EventBits_t uxBit;
  while (1)
  {
    uxBit = xEventGroupWaitBits(*s_time_system.p_flag_time_event,
                                TIME_SOURCE_RTC_READY | TIME_SOURCE_RTC_OFF
                                    | TIME_SOURCE_SNTP_READY,
                                pdFALSE,
                                pdFALSE,
                                0);

    if (uxBit & TIME_SOURCE_RTC_READY)
    {
      xSemaphoreTake(*s_time_system.p_time_mutex, 100 / portTICK_PERIOD_MS);

      DEV_DS3231_Register_Read(&s_time_system.s_ds3231_data, I2C_NUM);

      xSemaphoreGive(*s_time_system.p_time_mutex);

      s_DATA_SYNC.u8_data_start     = DATA_SYNC_TIME;
      s_DATA_SYNC.u8_data_packet[0] = s_time_system.s_ds3231_data.u8_minute;
      s_DATA_SYNC.u8_data_packet[1] = s_time_system.s_ds3231_data.u8_hour;
      s_DATA_SYNC.u8_data_packet[2] = s_time_system.s_ds3231_data.u8_date;
      s_DATA_SYNC.u8_data_packet[3] = s_time_system.s_ds3231_data.u8_month;
      s_DATA_SYNC.u8_data_packet[4] = s_time_system.s_ds3231_data.u8_year;
      s_DATA_SYNC.u8_data_length    = 5;
      s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

      xQueueSend(*s_time_system.p_send_data_queue, &s_DATA_SYNC, 0);
    }

    else if (uxBit & TIME_SOURCE_RTC_OFF)
    {
      // Sync SNTP time to RTC
      s_time_system.s_ds3231_data.u8_hour   = timeinfo.tm_hour;
      s_time_system.s_ds3231_data.u8_minute = timeinfo.tm_min;
      s_time_system.s_ds3231_data.u8_date   = timeinfo.tm_mday;
      s_time_system.s_ds3231_data.u8_month  = timeinfo.tm_mon;
      s_time_system.s_ds3231_data.u8_year   = timeinfo.tm_year;
      s_time_system.s_ds3231_data.u8_day    = timeinfo.tm_wday;

      DEV_DS3231_Register_Write(&s_time_system.s_ds3231_data, I2C_NUM);

      xEventGroupClearBits(*s_time_system.p_flag_time_event,
                           TIME_SOURCE_RTC_OFF);
      xEventGroupSetBits(*s_time_system.p_flag_time_event,
                         TIME_SOURCE_SNTP_READY);
    }

    else if (uxBit & TIME_SOURCE_SNTP_READY)
    {
      s_DATA_SYNC.u8_data_start     = DATA_SYNC_TIME;
      s_DATA_SYNC.u8_data_packet[0] = (uint8_t)timeinfo.tm_min;
      s_DATA_SYNC.u8_data_packet[1] = (uint8_t)timeinfo.tm_hour;
      s_DATA_SYNC.u8_data_packet[2] = (uint8_t)timeinfo.tm_mday;
      s_DATA_SYNC.u8_data_packet[3] = (uint8_t)timeinfo.tm_mon;
      s_DATA_SYNC.u8_data_packet[4] = (uint8_t)timeinfo.tm_year;
      s_DATA_SYNC.u8_data_length    = 5;
      s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

      xQueueSend(*s_time_system.p_send_data_queue, &s_DATA_SYNC, 0);

      ESP_LOGI(TAG, "Time: %ld", (uint32_t)now);
    }

    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
