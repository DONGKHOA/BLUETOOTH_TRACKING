/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#include "app_mqtt_client.h"
#include "app_data.h"
#include "handle_data/handle_data.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "environment.h"

#include "wifi_helper.h"

#include "nvs_rw.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG            "APP_MQTT_CLIENT"
#define URL_DEFAULT    "mqtt://broker.emqx.io"
#define MQTT_CONNECTED BIT0

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct mqtt_client_data
{
  QueueHandle_t           *p_data_mqtt_queue;
  QueueHandle_t           *p_send_data_queue;
  QueueHandle_t           *p_data_local_database_queue;
  SemaphoreHandle_t        s_data_subscribe_sem;
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
static int                status;
static int                user_id_delete;
static char *p_topic_request_server  = "ACCESS_CONTROL/Server/Request";
static char *p_topic_request_client  = "ACCESS_CONTROL/Client/Request";
static char *p_topic_response_server = "ACCESS_CONTROL/Server/Response";
static char *p_topic_response_client = "ACCESS_CONTROL/Client/Response";

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_MQTT_CLIENT_CreateTask (void)
{
  xTaskCreate(APP_MQTT_CLIENT_task, "mqtt task", 1024 * 4, NULL, 8, NULL);
}

void
APP_MQTT_CLIENT_Init (void)
{
  s_mqtt_client_data.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;
  s_mqtt_client_data.p_send_data_queue = &s_data_system.s_send_data_queue;
  s_mqtt_client_data.p_data_local_database_queue
      = &s_data_system.s_data_local_database_queue;
  s_mqtt_client_data.s_data_subscribe_sem = xSemaphoreCreateBinary();

  char mqtt_server[64];

  if (NVS_ReadString("MQTT", MQTTSERVER_NVS, mqtt_server) == ESP_OK)
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

  esp_mqtt_client_register_event(s_mqtt_client_data.s_MQTT_Client,
                                 ESP_EVENT_ANY_ID,
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
  DATA_SYNC_t s_DATA_SYNC;
  finger_cmd  e_finger_cmd;
  uint8_t     is_init = 0;

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

    if (xQueueReceive(*s_mqtt_client_data.p_data_mqtt_queue,
                      &s_DATA_SYNC,
                      100 / portTICK_PERIOD_MS)
        == pdPASS)
    {
      switch (s_DATA_SYNC.u8_data_start)
      {
        case DATA_SYNC_ENROLL_FACE:

          char data[128];

          sprintf(data,
                  "{\"command\" : \"ENROLL_FACE\", \"id\": %d}",
                  (s_DATA_SYNC.u8_data_packet[0] << 8)
                      | s_DATA_SYNC.u8_data_packet[1]);
          esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                  p_topic_request_server,
                                  data,
                                  0,
                                  1,
                                  0);

          break;
        case DATA_SYNC_ENROLL_FINGERPRINT:

        
        char data[128];

        sprintf(data,
                "{\"command\" : \"ENROLL_FINGERPRINT\", \"id\": %d}",
                (s_DATA_SYNC.u8_data_packet[0] << 8)
                    | s_DATA_SYNC.u8_data_packet[1]);
        esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                p_topic_request_server,
                                data,
                                0,
                                1,
                                0);

          break;

        case DATA_SYNC_REQUEST_ATTENDANCE:

          esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                  p_topic_request_server,
                                  "{\"command\" : \"ATTENDANCE\"}",
                                  0,
                                  1,
                                  0);

          break;
        case LOCAL_DATABASE_RESPONSE_DELETE_USER_DATA:

          char data[128];
          if (s_DATA_SYNC.u8_data_packet[0] == LOCAL_DATABASE_SUCCESS)
          {
            memcpy(data,
                   "{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                   "\"success\"}",
                   sizeof("{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                          "\"success\"}"));
          }
          else
          {
            memcpy(data,
                   "{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                   "\"fail\"}",
                   sizeof("{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                          "\"fail\"}"));
          }

          esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                  p_topic_response_server,
                                  data,
                                  0,
                                  1,
                                  0);
          break;
        default:
          break;
      }
    }

    if (xSemaphoreTake(s_mqtt_client_data.s_data_subscribe_sem,
                       100 / portTICK_PERIOD_MS)
        == pdTRUE)
    {
      switch (DECODE_Command(data))
      {
        case USER_DATA_CMD:

          for (uint16_t i = 0; i < user_len; i++)
          {
            heap_caps_free(user_name[i]);
            heap_caps_free(role[i]);
          }

          // Decode the user data from the server response
          DECODE_User_Data(
              data, user_id, face, finger, role, user_name, &user_len);

          // Send user len to the queue local database
          s_DATA_SYNC.u8_data_start     = LOCAL_DATABASE_USER_DATA;
          s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_DUMMY;
          s_DATA_SYNC.u8_data_length    = 1;
          s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case ENROLL_FACE_CMD:
          DECODE_Status(data, &status);

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start  = LOCAL_DATABASE_RESPONSE_ENROLL_FACE;
          s_DATA_SYNC.u8_data_length = 1;
          s_DATA_SYNC.u8_data_stop   = DATA_STOP_FRAME;
          if (status == 1)
          {
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_SUCCESS;
          }
          else
          {
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_FAIL;
          }
          // Notify the status of response to local database task via queue
          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case ENROLL_FINGER_CMD:
          DECODE_Status(data, &status);

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start
              = LOCAL_DATABASE_RESPONSE_ENROLL_FINGERPRINT;
          s_DATA_SYNC.u8_data_length = 1;
          s_DATA_SYNC.u8_data_stop   = DATA_STOP_FRAME;
          if (status == 1)
          {
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_SUCCESS;
          }
          else
          {
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_FAIL;
          }
          // Notify the status of response to local database task via queue
          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case ATTENDANCE_CMD:
          DECODE_Status(data, &status);

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start  = DATA_SYNC_RESPONSE_ATTENDANCE;
          s_DATA_SYNC.u8_data_length = 1;
          s_DATA_SYNC.u8_data_stop   = DATA_STOP_FRAME;
          if (status == 1)
          {
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_SUCCESS;
          }
          else
          {
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_FAIL;
          }
          // Notify the status of response to transmit task via queue
          xQueueSend(*s_mqtt_client_data.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case DELETE_USER_DATA_CMD:

          DECODE_User_ID(data, &user_id_delete);

          s_DATA_SYNC.u8_data_start = LOCAL_DATABASE_REQUEST_DELETE_USER_DATA;

          s_DATA_SYNC.u8_data_packet[0] = (user_id_delete >> 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = user_id_delete & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 2;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          // Notify the status of response to local database task via queue
          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case SET_ROLE:

          DECODE_User_ID(data, &user_id_delete);

          s_DATA_SYNC.u8_data_start = LOCAL_SET_ROLE;

          s_DATA_SYNC.u8_data_packet[0] = (user_id_delete >> 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = user_id_delete & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 2;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case DELETE_FINGER_USER:

          DECODE_User_ID(data, &user_id_delete);

          s_DATA_SYNC.u8_data_start = LOCAL_FINGER_DELETE;

          s_DATA_SYNC.u8_data_packet[0] = (user_id_delete >> 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = user_id_delete & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 2;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case DELETE_FACEID_USER:
          DECODE_User_ID(data, &user_id_delete);

          s_DATA_SYNC.u8_data_start = LOCAL_FACEID_DELETE;

          s_DATA_SYNC.u8_data_packet[0] = (user_id_delete >> 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = user_id_delete & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 2;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);
          break;

        default:
          break;
      }
    }
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

  esp_mqtt_event_handle_t event = event_data;

  switch ((esp_mqtt_event_id_t)event_id)
  {
    case MQTT_EVENT_CONNECTED:

      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      esp_mqtt_client_subscribe_single(
          s_mqtt_client_data.s_MQTT_Client, p_topic_response_client, 0);
      esp_mqtt_client_subscribe_single(
          s_mqtt_client_data.s_MQTT_Client, p_topic_request_client, 0);
      esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                              p_topic_request_server,
                              "{\"command\" : \"USER_DATA\"}",
                              0,
                              1,
                              0);
      break;
    case MQTT_EVENT_SUBSCRIBED:

      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");

      break;
    case MQTT_EVENT_PUBLISHED:

      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");

      break;
    case MQTT_EVENT_DATA:

      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      sprintf(data, "%.*s", event->data_len, event->data);
      printf("%s\r\n", data);
      xSemaphoreGive(s_mqtt_client_data.s_data_subscribe_sem);

      break;
    case MQTT_EVENT_DISCONNECTED:

      ESP_LOGE(TAG, "MQTT_EVENT_DISCONNECTED");

      break;
    default:
      ESP_LOGI(TAG, "Other event");
      break;
  }
}
