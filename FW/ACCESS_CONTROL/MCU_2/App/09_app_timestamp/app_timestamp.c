/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdbool.h>

#include "app_timestamp.h"
#include "app_data.h"

#include <sys/time.h>
#include "esp_sntp.h"
#include "esp_log.h"

/******************************************************************************
 *    PUBLIC DATA
 *****************************************************************************/

struct tm timeinfo;
time_t    now;

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_TIMESTAMP"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t      *p_send_data_queue;
  EventGroupHandle_t *p_flag_time_event;
  TimerHandle_t       s_timestamp_timer;
} handle_timestamp_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_TIMESTAMP_Notification_cb(struct timeval *tv);
static void APP_TIMESTAMP_Task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_timestamp_t s_handle_timestamp;

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_TIMESTAMP_CreateTask (void)
{
  xTaskCreate(APP_TIMESTAMP_Task, "timestamp task", 1024 * 10, NULL, 10, NULL);
}

void
APP_TIMESTAMP_Init (void)
{
  s_handle_timestamp.p_send_data_queue = &s_data_system.s_send_data_queue;
  s_handle_timestamp.p_flag_time_event = &s_data_system.s_flag_time_event;

  xEventGroupClearBits(*s_handle_timestamp.p_flag_time_event,
                       TIME_SOURCE_SNTP_READY);

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_set_time_sync_notification_cb(APP_TIMESTAMP_Notification_cb);
  esp_sntp_init();
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_TIMESTAMP_Notification_cb (struct timeval *tv)
{
  ESP_LOGI(TAG, "Time synchronized\n");

  xEventGroupSetBits(*s_handle_timestamp.p_flag_time_event,
                     TIME_SOURCE_SNTP_READY);
  xEventGroupClearBits(*s_handle_timestamp.p_flag_time_event,
                       TIME_SOURCE_RTC_READY);
  xEventGroupSetBits(*s_handle_timestamp.p_flag_time_event,
                     TIME_SOURCE_RTC_OFF);
}

static void
APP_TIMESTAMP_Task (void *arg)
{
  DATA_SYNC_t s_DATA_SYNC;
  EventBits_t uxBit;

  while (1)
  {
    uxBit = xEventGroupGetBits(*s_handle_timestamp.p_flag_time_event);
    if (uxBit & TIME_SOURCE_SNTP_READY)
    {
      time(&now);
      now += 7 * 3600;

      localtime_r(&now, &timeinfo);

      s_DATA_SYNC.u8_data_start     = DATA_SYNC_TIME;
      s_DATA_SYNC.u8_data_packet[0] = (uint8_t)timeinfo.tm_min;
      s_DATA_SYNC.u8_data_packet[1] = (uint8_t)timeinfo.tm_hour;
      s_DATA_SYNC.u8_data_packet[2] = (uint8_t)timeinfo.tm_mday;
      s_DATA_SYNC.u8_data_packet[3] = (uint8_t)timeinfo.tm_mon;
      s_DATA_SYNC.u8_data_packet[4] = (uint8_t)timeinfo.tm_year;
      s_DATA_SYNC.u8_data_length    = 5;
      s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

      xQueueSend(*s_handle_timestamp.p_send_data_queue, &s_DATA_SYNC, 0);

      ESP_LOGI(TAG, "Time: %ld", (uint32_t)now);
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}