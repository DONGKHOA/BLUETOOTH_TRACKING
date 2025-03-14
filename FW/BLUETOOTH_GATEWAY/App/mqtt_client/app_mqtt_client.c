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

#define TAG "APP_MQTT"
#define URL "mqtt://broker.emqx.io"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct mqtt_client_data
{
  QueueHandle_t           *p_data_mqtt_queue;
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

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

/**
 * The function creates a new task for an MQTT client in an embedded C
 * application.
 */
void
APP_MQTT_CLIENT_CreateTask (void)
{
  xTaskCreate(APP_MQTT_CLIENT_task, "mqtt task", 1024 * 2, NULL, 8, NULL);
}

/**
 * The function `APP_MQTT_CLIENT_Init` initializes an MQTT client with a
 * specified broker address URI and registers an event handler for MQTT events.
 */
void
APP_MQTT_CLIENT_Init (void)
{
  s_mqtt_client_data.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;

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
  int8_t i8_buffer;
  char   c_data_pub[8];
  while (1)
  {
    if (xQueueReceive(
            *s_mqtt_client_data.p_data_mqtt_queue, &i8_buffer, portMAX_DELAY)
        == pdPASS)
    {
      sprintf(c_data_pub, "%d", i8_buffer);
      esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                              "GL-IoT/dataTopic/E465B84062D2",
                              c_data_pub,
                              0,
                              1,
                              0);
    }
  }
}

/**
 * The function `mqtt_event_handler` handles different MQTT events and logs
 * corresponding messages based on the event type.
 *
 * @param handler_args The `handler_args` parameter in the `mqtt_event_handler`
 * function is a pointer to any additional arguments that you may want to pass
 * to the event handler function. It allows you to provide context or additional
 * data that the event handler might need to perform its tasks. In the provided
 * code snippet, this
 * @param base The `base` parameter in the `mqtt_event_handler` function
 * represents the event base from which the event was dispatched. It is of type
 * `esp_event_base_t`. This parameter helps identify the category or type of
 * event that occurred.
 * @param event_id The `event_id` parameter in the `mqtt_event_handler` function
 * represents the specific event that has occurred in the MQTT event loop. It is
 * used to determine the type of event that is being handled within the
 * switch-case statement in the function. The `event_id` is an integer value
 * that corresponds
 * @param event_data In the provided code snippet, the `event_data` parameter is
 * a pointer to the data associated with the event that triggered the
 * `mqtt_event_handler` function. This data could contain information relevant
 * to the event being processed, such as message payloads, connection details,
 * or other event-specific data.
 */
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

  // esp_mqtt_event_handle_t event = event_data;

  switch ((esp_mqtt_event_id_t)event_id)
  {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
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
      break;
    default:
      ESP_LOGI(TAG, "Other event");
      break;
  }
}
