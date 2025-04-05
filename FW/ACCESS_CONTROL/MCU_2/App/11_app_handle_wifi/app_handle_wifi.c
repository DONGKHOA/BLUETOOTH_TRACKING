/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "app_handle_wifi.h"
#include "app_data.h"
#include "environment.h"

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
  uint8_t            *p_ssid;
  uint8_t            *p_pass;
  uint8_t             u8_buffer[64];
  QueueHandle_t      *p_send_data_queue;
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
  s_handle_wifi.p_send_data_queue = &s_data_system.s_send_data_queue;
  s_handle_wifi.p_ssid            = s_data_system.u8_ssid;
  s_handle_wifi.p_pass            = s_data_system.u8_pass;

  memcpy(s_handle_wifi.p_ssid, "Thuc Coffee", sizeof("Thuc Coffee"));
  memcpy(s_handle_wifi.p_pass, "18006230", sizeof("18006230"));

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

  DATA_SYNC_t s_data_sync;

  while (1)
  {
    if (WIFI_state_connect() == CONNECT_OK)
    {
      s_data_sync.u8_data_start     = DATA_SYNC_STATE_CONNECTION;
      s_data_sync.u8_data_packet[0] = DATA_SYNC_STATE_CONNECT;
      s_data_sync.u8_data_length    = 1;
      s_data_sync.u8_data_stop      = DATA_STOP_FRAME;
      // Notify the state of connection to transmit task via queue
      // xQueueSend(*s_handle_wifi.p_send_data_queue, &s_data_sync, 0);

    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}