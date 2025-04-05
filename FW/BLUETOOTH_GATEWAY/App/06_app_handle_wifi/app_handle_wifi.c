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
static int8_t matchingWIFIScan(char *data, uint8_t *ssid, uint8_t *pass);

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
  xTaskCreate(APP_HANDLE_WIFI_Task,
              "wifi task",
              1024 * 30,
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
  WIFI_Scan(s_ssid);

  uint8_t num_wifi = WIFI_GetNumSSID();

  if (num_wifi != 0)
  {
    WIFI_ScanSSID(s_handle_wifi.u8_ssid, 1);
    WIFI_ScanPass(s_handle_wifi.u8_pass, 1);
    if (matchingWIFIScan(s_ssid, s_handle_wifi.u8_ssid, s_handle_wifi.u8_pass)
        != -1)
    {
      WIFI_Connect(s_handle_wifi.u8_ssid, s_handle_wifi.u8_pass);
    }
  }
  vTaskDelete(s_handle_wifi_task);
}

static int8_t
matchingWIFIScan (char *data, uint8_t *ssid, uint8_t *pass)
{
  char *arg_list[50];
  char  buffer[1024];
  memcpy(buffer, data, strlen((char *)data));
  uint8_t arg_position = 0;

  // cut string
  char *temp_token = strtok(buffer, "\r");
  while (temp_token != NULL)
  {
    arg_list[arg_position] = temp_token;
    arg_position++;
    temp_token = strtok(NULL, "\r");
  }

  // check matching ssid in NVS
  for (uint8_t i = 0; i < arg_position; i++)
  {
    if (memcmp((uint8_t *)arg_list[i], ssid, strlen((char *)ssid) + 1) == 0)
    {
      return 1;
    }
  }
  return -1;
}
