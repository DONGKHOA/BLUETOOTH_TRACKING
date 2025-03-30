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
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_TIMESTAMP"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct
{
  EventGroupHandle_t *p_flag_time_event;
} handle_timestamp_t;
/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

void        APP_TIMESTAMP_Notification_cb(struct timeval *tv);
static void APP_TIMESTAMP_task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_timestamp_t s_handle_timestamp;
static bool               b_time_synced = false; // Time synchronized flag

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_TIMESTAMP_CreateTask (void)
{
  xTaskCreate(APP_TIMESTAMP_task, "timestamp task", 1024 * 4, NULL, 10, NULL);
}

void
APP_TIMESTAMP_Init (void)
{
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

void
APP_TIMESTAMP_Notification_cb (struct timeval *tv)
{
  ESP_LOGI(TAG, "Time synchronized\n");
  //   b_time_synced = true;

  xEventGroupSetBits(*s_handle_timestamp.p_flag_time_event,
                     TIME_SOURCE_SNTP_READY);

  xEventGroupClearBits(*s_handle_timestamp.p_flag_time_event,
                       TIME_SOURCE_RTC_READY);
}

static void
APP_TIMESTAMP_task (void *arg)
{
  time_t now;
  while (1)
  {
    // Wait for SNTP or RTC to be ready
    EventBits_t bits
        = xEventGroupWaitBits(*s_handle_timestamp.p_flag_time_event,
                              TIME_SOURCE_SNTP_READY | TIME_SOURCE_RTC_READY,
                              pdFALSE,
                              pdFALSE,
                              portMAX_DELAY);

    if (bits & TIME_SOURCE_SNTP_READY)
    {
      time(&now);
      now += 7 * 3600;
      localtime_r(&now, &timeinfo);

      ESP_LOGI(TAG,
               "[SNTP] Time: %02d:%02d:%02d %02d/%02d/%04d",
               timeinfo.tm_hour,
               timeinfo.tm_min,
               timeinfo.tm_sec,
               timeinfo.tm_mday,
               timeinfo.tm_mon + 1,
               timeinfo.tm_year + 1900);
    }
    else
    {
      ESP_LOGW(TAG, "Time not synchronized yet (waiting for SNTP)");
    }

    //   if (!b_time_synced)
    //   {
    //     ESP_LOGI(TAG, "Time not synchronized\r\n");
    //     continue;
    //   }

    //   time(&now);
    //   now += 7 * 3600;
    //   ESP_LOGI(TAG, "Time: %ld\r\n", (uint32_t)now);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}