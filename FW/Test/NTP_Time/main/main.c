#include <stdio.h>

// #include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_rw.h"
#include "wifi.h"

#define TAG "NTP"

uint8_t ssid_name[] = "Khoa";
uint8_t password[]  = "17042021";

void
time_sync_notification_cb (struct timeval *tv)
{
  ESP_LOGI(TAG, "Time synchronized\n");
}

void
initialize_sntp (void)
{
  ESP_LOGI(TAG, "Initializing SNTP\n");
  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  esp_sntp_init();
}

void
sntp_task (void *pvParameters)
{
  initialize_sntp();

  while (1)
  {
    time_t now;
    time(&now);
    now += 7 * 3600;
    printf("Time: %lld\r\n", now);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void
app_main (void)
{
  NVS_Init();
  WIFI_StaInit();
  WIFI_Connect(ssid_name, password);

  xTaskCreate(&sntp_task, "sntp_task", 4096 * 2, NULL, 5, NULL);
}
