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

static void   APP_HANDLE_WIFI_Task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_wifi_t s_handle_wifi;
static TaskHandle_t  s_handle_wifi_task;

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_HANDLE_WIFI_CreateTask (void)
{
  xTaskCreate(APP_HANDLE_WIFI_Task,
              "wifi task",
              1024 * 20,
              NULL,
              13,
              &s_handle_wifi_task);
}

void
APP_HANDLE_WIFI_Init (void)
{
  WIFI_StaInit();
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_HANDLE_WIFI_Task (void *arg)
{
  uint8_t num_wifi = WIFI_GetNumSSID();

  if (num_wifi != 0)
  {
    WIFI_ScanSSID(s_handle_wifi.u8_ssid, 1);
    WIFI_ScanPass(s_handle_wifi.u8_pass, 1);
    WIFI_Connect(s_handle_wifi.u8_ssid, s_handle_wifi.u8_pass);
  }
  vTaskDelete(s_handle_wifi_task);
}