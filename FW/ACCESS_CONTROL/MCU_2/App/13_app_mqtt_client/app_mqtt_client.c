/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_mqtt_client.h"
#include "app_data.h"

#include "esp_log.h"
#include "mqtt_client.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_MQTT_CLIENT"

#define URL "mqtt://broker.emqx.io"

#define MQTT_CONNECTED BIT0

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct mqtt_client_data
{
  QueueHandle_t           *p_data_mqtt_queue;
  QueueHandle_t            s_data_subscribe_queue;
  EventGroupHandle_t       s_mqtt_event;
  esp_mqtt_client_handle_t s_MQTT_Client;
} mqtt_client_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_MQTT_CLIENT_task(void *arg);
static void mqtt_event_handler(void            *handler_args,
                               esp_event_base_t base,
                               int32_t          event_id,
                               void            *event_data);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static mqtt_client_data_t s_mqtt_client_data;
static char               data[1024 * 10];

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_MQTT_CLIENT_CreateTask (void)
{
  xTaskCreate(APP_MQTT_CLIENT_task, "mqtt task", 1024 * 2, NULL, 8, NULL);
}

void
APP_MQTT_CLIENT_Init (void)
{
  s_mqtt_client_data.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;
  s_mqtt_client_data.s_mqtt_event      = xEventGroupCreate();
  s_mqtt_client_data.s_data_subscribe_queue
      = xQueueCreate(2, sizeof(char) * 1024 * 10);

  esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = URL,
  };

  s_mqtt_client_data.s_MQTT_Client = esp_mqtt_client_init(&mqtt_cfg);

  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(s_mqtt_client_data.s_MQTT_Client,
                                 MQTT_EVENT_ANY,
                                 mqtt_event_handler,
                                 NULL);
  esp_mqtt_client_start(s_mqtt_client_data.s_MQTT_Client);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_MQTT_CLIENT_task (void *arg)
{
  EventBits_t bits;
  while (1)
  {
    bits = xEventGroupWaitBits(
        s_mqtt_client_data.s_mqtt_event, MQTT_CONNECTED, pdFALSE, pdFALSE, 0);
    if ((bits & MQTT_CONNECTED) == 0)
    {
      continue;
    }

    esp_mqtt_client_subscribe_single(s_mqtt_client_data.s_MQTT_Client, "", 0);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
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
      xEventGroupSetBits(s_mqtt_client_data.s_mqtt_event, MQTT_CONNECTED);

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
      xEventGroupClearBits(s_mqtt_client_data.s_mqtt_event, MQTT_CONNECTED);

      break;
    default:
      ESP_LOGI(TAG, "Other event");
      break;
  }
}
