/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

#define TAG "APP_MQTT_CLIENT"

#define URL_DEFAULT   "mqtt://broker.emqx.io"
#define TOPIC_DEFAULT "01020304/data"

#define SIZE_DATA_MQTT_PUBLISH      10240
#define SIZE_TOPIC_MQTT_PUBLISH     64
#define SIZE_DEV_INFOR_MQTT_PUBLISH 256

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct mqtt_client_data
{
  QueueHandle_t           *p_data_mqtt_queue;
  state_system_t          *p_state_system;
  uint8_t                  u8_num_dev;
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
static char              *p_data_pub;
static char              *p_topic_pub;
static dev_info_t        *p_dev_infor;

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
  s_mqtt_client_data.p_state_system = &s_data_system.s_state_system;

  *s_mqtt_client_data.p_state_system = STATE_WIFI_DISCONNECTED;

  s_mqtt_client_data.u8_num_dev = 0;

  p_data_pub
      = (char *)heap_caps_malloc(SIZE_DATA_MQTT_PUBLISH, MALLOC_CAP_SPIRAM);
  p_topic_pub
      = (char *)heap_caps_malloc(SIZE_TOPIC_MQTT_PUBLISH, MALLOC_CAP_SPIRAM);
  p_dev_infor = (dev_info_t *)heap_caps_malloc(SIZE_DEV_INFOR_MQTT_PUBLISH,
                                               MALLOC_CAP_SPIRAM);

  memset(p_data_pub, 0, SIZE_DATA_MQTT_PUBLISH);
  memset(p_topic_pub, 0, SIZE_TOPIC_MQTT_PUBLISH);
  memset(p_dev_infor, 0, SIZE_DEV_INFOR_MQTT_PUBLISH);

  s_mqtt_client_data.p_data_mqtt_queue = &s_data_system.s_location_tag_queue;

  char mqtt_server[64];

  if (NVS_ReadString("MQTT", MQTTSERVER_NVS, mqtt_server, 64) == ESP_OK)
  {
    memmove(mqtt_server + 7, mqtt_server, strlen(mqtt_server) + 1);
    memcpy(mqtt_server, "mqtt://", 7);
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

  if (NVS_ReadString("MQTT", MQTTTOPIC_NVS, p_topic_pub, 64) != ESP_OK)
  {
    strcpy(p_topic_pub, TOPIC_DEFAULT);
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
    if (WIFI_state_connect() == CONNECT_OK && is_init == false)
    {
      is_init = true;

      *s_mqtt_client_data.p_state_system = STATE_WIFI_CONNECTED;
      esp_mqtt_client_start(s_mqtt_client_data.s_MQTT_Client);
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
                               p_dev_infor,
                               s_mqtt_client_data.u8_num_dev);

      // Check validity before copying
      if (generated_str)
      {
        strncpy(p_data_pub, generated_str, sizeof(p_data_pub) - 1);
        p_data_pub[sizeof(p_data_pub) - 1] = '\0'; // Ensure null-termination

        esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                p_topic_pub,
                                (const char *)&p_data_pub,
                                0,
                                1,
                                0);

        s_mqtt_client_data.u8_num_dev = 0;
        memset(p_data_pub, 0, sizeof(p_data_pub));

        free(generated_str); // Free the original allocated string
      }
    }
  }
}

static inline void
APP_MQTT_CLIENT_UpdateDataDev (tracking_infor_tag_t *p_tracking_infor_tag)
{
  for (uint8_t i = 0; i < 6; i++)
  {
    // "%02X" ensures 2 uppercase hex digits
    snprintf(&p_dev_infor[s_mqtt_client_data.u8_num_dev].mac[i * 2],
             3,
             "%02X",
             p_tracking_infor_tag->u8_beacon_addr[i]);
  }

  // Null-terminate the string
  p_dev_infor[s_mqtt_client_data.u8_num_dev].mac[12] = '\0';

  p_dev_infor[s_mqtt_client_data.u8_num_dev].rssi
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
      *s_mqtt_client_data.p_state_system = STATE_WIFI_DISCONNECTED;
      break;
    default:
      ESP_LOGI(TAG, "Other event");
      break;
  }
}
