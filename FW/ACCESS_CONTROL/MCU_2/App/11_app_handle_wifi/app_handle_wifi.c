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
  uint8_t       *p_ssid;
  uint8_t       *p_pass;
  uint8_t       u8_buffer[64];
  QueueHandle_t *p_send_data_queue;
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

  memcpy(s_handle_wifi.p_ssid, "The Little Bean", sizeof("The Little Bean"));
  memcpy(s_handle_wifi.p_pass, "75Caothang", sizeof("75Caothang"));

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
    if (WIFI_state_connect() == CONNECT_OK)
    {
      ESP_LOGI(TAG, "WiFi Connect OK!");

      // Notify the state of connection to transmit task via queue
      xQueueSend(*s_handle_wifi.p_send_data_queue, , 0);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static void
APP_HANLDE_WIFI_Combine_data (uint8_t *p_data, uint8_t u8_len)
{
  uint8_t u8_index = 0;
  uint8_t u8_count = 0;

  // Combine data to send to MCU1
  s_handle_wifi.u8_buffer[u8_index++] = ;
  s_handle_wifi.u8_buffer[u8_index++] = u8_len;

  for (u8_count = 0; u8_count < u8_len; u8_count++)
  {
    s_handle_wifi.u8_buffer[u8_index++] = p_data[u8_count];
  }

  s_handle_wifi.u8_buffer[u8_index++] = DATA_STOP_FRAME;
}
