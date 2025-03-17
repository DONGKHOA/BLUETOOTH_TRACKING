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
  uint8_t           *p_ssid;
  uint8_t           *p_pass;
} handle_wifi_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_HANDLE_WIFI_task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_wifi_t s_handle_wifi;
static char          s_ssid[1024];

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_HANDLE_WIFI_CreateTask (void)
{
  xTaskCreate(APP_HANDLE_WIFI_task, "wifi task", 1024 * 10, NULL, 13, NULL);
}

void
APP_HANDLE_WIFI_Init (void)
{
  s_handle_wifi.p_ssid           = s_data_system.u8_ssid;
  s_handle_wifi.p_pass           = s_data_system.u8_pass;

  memcpy(s_handle_wifi.p_ssid, "PIF_CLUB", sizeof("PIF_CLUB"));
  memcpy(s_handle_wifi.p_pass, "chinsochin", sizeof("chinsochin"));

  WIFI_StaInit();
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_HANDLE_WIFI_task (void *arg)
{
  WIFI_Scan(s_ssid);
  WIFI_Connect(s_handle_wifi.p_ssid, s_handle_wifi.p_pass);

  while (1)
  {
    // if (WIFI_state_connect() == CONNECT_OK)
    // {
    //   ESP_LOGI(TAG, "WiFi Connect OK!");
    // }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
