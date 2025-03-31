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

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_TIMESTAMP_CreateTask (void)
{
  xTaskCreate(APP_TIMESTAMP_task, "timestamp task", 1024 * 3, NULL, 10, NULL);
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

  xEventGroupSetBits(*s_handle_timestamp.p_flag_time_event,
                     TIME_SOURCE_SNTP_READY);
}

static void
APP_TIMESTAMP_task (void *arg)
{
  time_t      now;
  EventBits_t bits;
  while (1)
  {
    // Wait for SNTP or RTC to be ready
    bits = xEventGroupWaitBits(*s_handle_timestamp.p_flag_time_event,
                               TIME_SOURCE_SNTP_READY,
                               pdFALSE,
                               pdFALSE,
                               portMAX_DELAY);

    if (bits & TIME_SOURCE_SNTP_READY)
    {
      time(&now);
      now += 7 * 3600;
      ESP_LOGI(TAG, "Time: %ld\n", (uint32_t)now);
    }
    else
    {
      ESP_LOGW(TAG, "Time not synchronized yet (waiting for SNTP)");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}