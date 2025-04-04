/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "app_handle_wifi.h"
#include "app_data.h"

#include "wifi_helper.h"

#include "esp_log.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_HANDLE_WIFI"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  uint8_t u8_ssid[32];
  uint8_t u8_pass[32];
} handle_wifi_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_HANDLE_WIFI_task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_wifi_t s_handle_wifi;
static TaskHandle_t  s_handle_wifi_task;
static char          s_ssid[1024];

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_HANDLE_WIFI_CreateTask (void)
{
  xTaskCreate(APP_HANDLE_WIFI_task,
              "wifi task",
              1024 * 10,
              NULL,
              13,
              &s_handle_wifi_task);
}

void
APP_HANDLE_WIFI_Init (void)
{
  memcpy(s_handle_wifi.u8_ssid, "Van Son", sizeof("Van Son"));
  memcpy(s_handle_wifi.u8_pass, "26061975", sizeof("26061975"));

  WIFI_StaInit();
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_HANDLE_WIFI_task (void *arg)
{
  WIFI_Scan(s_ssid);
  WIFI_Connect(s_handle_wifi.u8_ssid, s_handle_wifi.u8_pass);

  while (1)
  {
    if (WIFI_state_connect() == CONNECT_OK)
    {
      ESP_LOGI(TAG, "Connected to %s", s_handle_wifi.u8_ssid);
      vTaskSuspend(s_handle_wifi_task);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
