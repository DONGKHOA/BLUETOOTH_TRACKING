/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#include "app_mqtt_client.h"
#include "app_data.h"
#include "json_generate.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "wifi_helper.h"

#include "nvs_rw.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG           "APP_MQTT_CLIENT"
#define URL_DEFAULT   "mqtt://broker.emqx.io"
#define TOPIC_DEFAULT "01020304/data"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct mqtt_client_data
{
  char                     c_data_pub[10240];
  char                     c_topic_pub[64];
  dev_info_t               s_dev_infor[256];
  uint8_t                  u8_num_dev;
  QueueHandle_t           *p_data_mqtt_queue;
  esp_mqtt_client_handle_t s_MQTT_Client;
} mqtt_client_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void        APP_MQTT_CLIENT_Task(void *arg);
static inline void APP_MQTT_CLIENT_UpdateDataDev(
    tracking_infor_tag_t *p_tracking_infor_tag);
static void mqtt_event_handler(void            *handler_args,
                               esp_event_base_t base,
                               int32_t          event_id,
                               void            *event_data);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static mqtt_client_data_t s_mqtt_client_data;
static bool               b_mqtt_client_connected = false; //

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_MQTT_CLIENT_CreateTask (void)
{
  xTaskCreate(APP_MQTT_CLIENT_Task, "mqtt task", 1024 * 2, NULL, 10, NULL);
}

void
APP_MQTT_CLIENT_Init (void)
{
  s_mqtt_client_data.u8_num_dev = 0;
  memset(
      s_mqtt_client_data.c_data_pub, 0, sizeof(s_mqtt_client_data.c_data_pub));
  memset(s_mqtt_client_data.c_topic_pub,
         0,
         sizeof(s_mqtt_client_data.c_topic_pub));
  memset(s_mqtt_client_data.s_dev_infor,
         0,
         sizeof(s_mqtt_client_data.s_dev_infor));

  s_mqtt_client_data.p_data_mqtt_queue = &s_data_system.s_location_tag_queue;

  char mqtt_server[64];

  if (NVS_ReadString(MQTTSERVER_NVS, "MQTTSERVER_NVS", mqtt_server) == ESP_OK)
  {
    esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = mqtt_server,
    };
    s_mqtt_client_data.s_MQTT_Client = esp_mqtt_client_init(&mqtt_cfg);
  }
  else
  {
    esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = URL_DEFAULT,
    };
    s_mqtt_client_data.s_MQTT_Client = esp_mqtt_client_init(&mqtt_cfg);
  }

  if (NVS_ReadString(
          MQTTSERVER_NVS, "MQTTTOPIC_NVS", s_mqtt_client_data.c_topic_pub)
      != ESP_OK)
  {
    strcpy(s_mqtt_client_data.c_topic_pub, TOPIC_DEFAULT);
  }

  esp_mqtt_client_register_event(s_mqtt_client_data.s_MQTT_Client,
                                 MQTT_EVENT_ANY,
                                 mqtt_event_handler,
                                 NULL);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_MQTT_CLIENT_Task (void *arg)
{
  tracking_infor_tag_t tracking_infor_tag;
  uint8_t              is_init = 0;

  while (1)
  {
    if (WIFI_state_connect() == CONNECT_OK)
    {
      if (is_init == 0)
      {
        is_init = 1;
        esp_mqtt_client_start(s_mqtt_client_data.s_MQTT_Client);
      }
    }

    if (xQueueReceive(
            *s_mqtt_client_data.p_data_mqtt_queue, &tracking_infor_tag, 1000)
        == pdPASS)
    {

      if (!b_mqtt_client_connected)
      {
        continue;
      }

      APP_MQTT_CLIENT_UpdateDataDev(&tracking_infor_tag);
      s_mqtt_client_data.u8_num_dev++;

      char c_gateway_id[9];

      snprintf(c_gateway_id,
               sizeof(c_gateway_id),
               "%08" PRIx32,
               tracking_infor_tag.u32_gateway_ID);

      char *generated_str
          = create_device_json(c_gateway_id,
                               tracking_infor_tag.c_gateway_version,
                               s_mqtt_client_data.s_dev_infor,
                               s_mqtt_client_data.u8_num_dev);

      // Check validity before copying
      if (generated_str)
      {
        strncpy(s_mqtt_client_data.c_data_pub,
                generated_str,
                sizeof(s_mqtt_client_data.c_data_pub) - 1);
        s_mqtt_client_data.c_data_pub[sizeof(s_mqtt_client_data.c_data_pub) - 1]
            = '\0';          // Ensure null-termination
        free(generated_str); // Free the original allocated string
      }

      esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                              s_mqtt_client_data.c_topic_pub,
                              (const char *)&s_mqtt_client_data.c_data_pub,
                              0,
                              1,
                              0);

      s_mqtt_client_data.u8_num_dev = 0;
      memset(s_mqtt_client_data.c_data_pub,
             0,
             sizeof(s_mqtt_client_data.c_data_pub));
    }
  }
}

static inline void
APP_MQTT_CLIENT_UpdateDataDev (tracking_infor_tag_t *p_tracking_infor_tag)
{
  for (uint8_t i = 0; i < 6; i++)
  {
    // "%02X" ensures 2 uppercase hex digits
    snprintf(&s_mqtt_client_data.s_dev_infor[s_mqtt_client_data.u8_num_dev]
                  .mac[i * 2],
             3,
             "%02X",
             p_tracking_infor_tag->u8_beacon_addr[i]);
  }

  // Null-terminate the string
  s_mqtt_client_data.s_dev_infor[s_mqtt_client_data.u8_num_dev].mac[12] = '\0';

  s_mqtt_client_data.s_dev_infor[s_mqtt_client_data.u8_num_dev].rssi
      = p_tracking_infor_tag->i8_filtered_rssi;
}

static void
mqtt_event_handler (void            *handler_args,
                    esp_event_base_t base,
                    int32_t          event_id,
                    void            *event_data)
{
  ESP_LOGD(TAG,
           "Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
           base,
           event_id);

  switch ((esp_mqtt_event_id_t)event_id)
  {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      b_mqtt_client_connected = true;
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGE(TAG, "MQTT_EVENT_DISCONNECTED");
      b_mqtt_client_connected = false;
      break;
    default:
      ESP_LOGI(TAG, "Other event");
      break;
  }
}
