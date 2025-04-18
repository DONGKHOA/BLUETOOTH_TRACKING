/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "wifi_helper.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_ip_addr.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "WIFI_STATION_MODE"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void WIFI_Sta_event_handler(void            *arg,
                                   esp_event_base_t event_base,
                                   int32_t          event_id,
                                   void            *event_data);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static uint8_t       s_retry_num          = 0;
extern WIFI_Status_t state_connected_wifi;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
WIFI_StaInit (void)
{
  //  s_wifi_data.p_flag_wifi_event = &s_data_system.s_flag_wifi_event;
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void
WIFI_Connect (uint8_t *ssid, uint8_t *password)
{
  state_connected_wifi = CONNECT_FAIL;
  esp_wifi_stop();

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&config);

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  esp_event_handler_instance_register(WIFI_EVENT,
                                      ESP_EVENT_ANY_ID,
                                      &WIFI_Sta_event_handler,
                                      NULL,
                                      &instance_any_id);
  esp_event_handler_instance_register(IP_EVENT,
                                      IP_EVENT_STA_GOT_IP,
                                      &WIFI_Sta_event_handler,
                                      NULL,
                                      &instance_got_ip);

  wifi_config_t wifi_config =
  {
    .sta =
      {
          .threshold.authmode = WIFI_AUTH_WPA2_PSK,
          .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
      },
  };

  for (uint8_t i = 0; i < 32; i++)
  {
    wifi_config.sta.ssid[i]     = *(ssid + i);
    wifi_config.sta.password[i] = *(password + i);
  }

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();

  ESP_LOGI(TAG, "Wifi_init_station finished.");
}
/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/
static void
WIFI_Sta_event_handler (void            *arg,
                        esp_event_base_t event_base,
                        int32_t          event_id,
                        void            *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (s_retry_num < MAXIMUM_RETRY)
    {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    }
    else
    {
      state_connected_wifi = CONNECT_FAIL;
    }
    ESP_LOGE(TAG, "connect to the AP fail");
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    state_connected_wifi     = CONNECT_OK;
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
  }
}