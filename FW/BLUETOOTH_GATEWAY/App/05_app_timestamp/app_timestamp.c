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

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

void        APP_TIMESTAMP_Notification_cb(struct timeval *tv);
static void APP_TIMESTAMP_task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static bool        b_time_synced = false; // Time synchronized flag

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_TIMESTAMP_CreateTask (void)
{
  xTaskCreate(APP_TIMESTAMP_task, "timestamp task", 1024 * 2, NULL, 14, NULL);
}

void
APP_TIMESTAMP_Init (void)
{
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
  b_time_synced = true;
}

static void
APP_TIMESTAMP_task (void *arg)
{
  // time_t now;
  while (1)
  {
    // if (xSemaphoreTake(*s_timestamp.p_wifi_semaphore, portMAX_DELAY) == pdTRUE)
    // {
    //   if (!b_time_synced)
    //   {
    //     ESP_LOGI(TAG, "Time not synchronized\n");
    //     continue;
    //   }
      
    //   time(&now);
    //   now += 7 * 3600;
    //   ESP_LOGI(TAG, "Time: %ld\n", (uint32_t)now);
    // }
  }
}