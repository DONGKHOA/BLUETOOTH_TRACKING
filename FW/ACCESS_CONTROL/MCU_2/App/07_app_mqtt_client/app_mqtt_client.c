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
 *    PUBLIC DATA
 *****************************************************************************/

extern sdcard_data_t s_sdcard_data;

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
  EventGroupHandle_t      *p_flag_time_event;
  QueueHandle_t           *p_data_mqtt_queue;
  QueueHandle_t           *p_send_data_queue;
  QueueHandle_t           *p_data_sdcard_queue;
  QueueHandle_t           *p_data_local_database_queue;
  SemaphoreHandle_t        s_data_subscribe_sem;
  esp_mqtt_client_handle_t s_MQTT_Client;
  state_system_t          *p_state_system;
  bool                     b_connected_server;
  bool                     b_mqtt_client_connected;
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

static DATA_SYNC_t        s_DATA_SYNC;
static mqtt_client_data_t s_mqtt_client_data;
static sdcard_cmd_t       s_sdcard_cmd;
static char               data[1024 * 10];
static char               data_send[1024 * 10];
static int                status;
static int                user_id_delete;
static int                user_id_set_role;
static char               user_role[8];
static char u32_topic_request_server[32]  = "ACCESS_CONTROL/Server/Request";
static char u32_topic_request_client[32]  = "ACCESS_CONTROL/Client/Request";
static char u32_topic_response_server[32] = "ACCESS_CONTROL/Server/Response";
static char u32_topic_response_client[32] = "ACCESS_CONTROL/Client/Response";

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

  s_mqtt_client_data.p_state_system = &s_data_system.s_state_system;

  *s_mqtt_client_data.p_state_system = STATE_WIFI_DISCONNECTED;

  s_mqtt_client_data.p_flag_time_event   = &s_data_system.s_flag_time_event;
  s_mqtt_client_data.p_data_sdcard_queue = &s_data_system.s_data_sdcard_queue;

  s_mqtt_client_data.b_connected_server      = false;
  s_mqtt_client_data.b_mqtt_client_connected = false;

  char room_name[8];
  if (NVS_ReadString("ROOM", ROOM_NVS, room_name, 8) == ESP_OK)
  {
    // Update Topic for MQTT Client
    snprintf(u32_topic_request_server,
             sizeof(u32_topic_request_server),
             "%s/Server/Request",
             room_name);
    snprintf(u32_topic_request_client,
             sizeof(u32_topic_request_client),
             "%s/Client/Request",
             room_name);
    snprintf(u32_topic_response_server,
             sizeof(u32_topic_response_server),
             "%s/Server/Response",
             room_name);
    snprintf(u32_topic_response_client,
             sizeof(u32_topic_response_client),
             "%s/Client/Response",
             room_name);
  }

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

  esp_mqtt_client_register_event(s_mqtt_client_data.s_MQTT_Client,
                                 ESP_EVENT_ANY_ID,
                                 mqtt_event_handler,
                                 NULL);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_MQTT_CLIENT_task (void *arg)
{
  bool is_init = false;

  while (1)
  {
    if (WIFI_state_connect() == CONNECT_OK && is_init == false)
    {
      is_init = true;

      *s_mqtt_client_data.p_state_system = STATE_WIFI_CONNECTED;
      esp_mqtt_client_start(s_mqtt_client_data.s_MQTT_Client);

      // Send state wifi to MCU1
      s_DATA_SYNC.u8_data_start     = DATA_SYNC_STATE_CONNECTION_WIFI;
      s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_SUCCESS;
      s_DATA_SYNC.u8_data_length    = 1;
      s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
      xQueueSend(*s_mqtt_client_data.p_send_data_queue, &s_DATA_SYNC, 0);
    }

    if ((s_mqtt_client_data.b_connected_server == false)
        && (s_mqtt_client_data.b_mqtt_client_connected == true))
    {
      strcpy(data_send, "{\"command\" : \"SYN\"}");
      esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                              u32_topic_request_server,
                              data_send,
                              0,
                              1,
                              0);
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }

    if (s_mqtt_client_data.b_connected_server == true)
    {
      if (xQueueReceive(*s_mqtt_client_data.p_data_mqtt_queue,
                        &s_DATA_SYNC,
                        100 / portTICK_PERIOD_MS)
          == pdPASS)
      {
        switch (s_DATA_SYNC.u8_data_start)
        {
          case DATA_SYNC_REQUEST_ATTENDANCE:

            sprintf(data_send,
                    "{\"command\" : \"ATTENDANCE\", \"id\": %d, \"timestamp\": "
                    "%ld}",
                    s_sdcard_data.u16_user_id,
                    s_sdcard_data.u32_time);
            esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                    u32_topic_request_server,
                                    data_send,
                                    0,
                                    1,
                                    0);

            break;
          case DATA_SYNC_ENROLL_FACE:

            sprintf(data_send,
                    "{\"command\" : \"ENROLL_FACE\", \"id\": %d}",
                    (s_DATA_SYNC.u8_data_packet[0] << 8)
                        | s_DATA_SYNC.u8_data_packet[1]);
            esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                    u32_topic_request_server,
                                    data_send,
                                    0,
                                    1,
                                    0);

            break;
          case DATA_SYNC_ENROLL_FINGERPRINT:

            sprintf(data_send,
                    "{\"command\" : \"ENROLL_FINGERPRINT\", \"id\": %d}",
                    (s_DATA_SYNC.u8_data_packet[0] << 8)
                        | s_DATA_SYNC.u8_data_packet[1]);
            esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                    u32_topic_request_server,
                                    data_send,
                                    0,
                                    1,
                                    0);

            break;

          case LOCAL_DATABASE_DATA:

            char *generated_str = ENCODE_Sync_Data(
                user_id, face, finger, role, user_name, &user_len);

            if (generated_str)
            {
              strncpy(data_send, generated_str, strlen(generated_str) + 1);
              free(generated_str); // Caller must free memory
            }

            esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                    u32_topic_request_server,
                                    data_send,
                                    0,
                                    1,
                                    0);

            break;

          case LOCAL_DATABASE_RESPONSE_DELETE_USER_DATA:

            if (s_DATA_SYNC.u8_data_packet[0] == LOCAL_DATABASE_SUCCESS)
            {
              memcpy(
                  data_send,
                  "{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                  "\"success\"}",
                  sizeof("{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                         "\"success\"}"));
            }
            else
            {
              memcpy(
                  data_send,
                  "{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                  "\"fail\"}",
                  sizeof("{\"command\" : \"DELETE_USER_DATA\", \"response\": "
                         "\"fail\"}"));
            }

            esp_mqtt_client_publish(s_mqtt_client_data.s_MQTT_Client,
                                    u32_topic_response_server,
                                    data_send,
                                    0,
                                    1,
                                    0);
            break;
          default:
            break;
        }
      }
    }

    if (xSemaphoreTake(s_mqtt_client_data.s_data_subscribe_sem,
                       100 / portTICK_PERIOD_MS)
        == pdTRUE)
    {
      switch (DECODE_Command(data))
      {
        case SYNC_CMD:

          s_mqtt_client_data.b_connected_server = true;
          DECODE_Status(data, &status);

          s_sdcard_cmd = SDCARD_SYNC_DATA_SERVER;
          xQueueSend(*s_mqtt_client_data.p_data_sdcard_queue, &s_sdcard_cmd, 0);

          break;

        case ENROLL_FACE_CMD:
          DECODE_Status(data, &status);

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start  = DATA_SYNC_RESPONSE_ENROLL_FACE;
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
          xQueueSend(*s_mqtt_client_data.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case ENROLL_FINGER_CMD:
          DECODE_Status(data, &status);

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start  = DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT;
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

          xQueueSend(*s_mqtt_client_data.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case ATTENDANCE_CMD:
          DECODE_Status(data, &status);

          // Send to MCU1 to notify that the status of attendance
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

          DECODE_Set_Role_Data(data, &user_id_set_role, user_role);

          printf("User Role: %s\n", user_role);

          s_DATA_SYNC.u8_data_start = LOCAL_DATABASE_SET_ROLE;

          s_DATA_SYNC.u8_data_packet[0]
              = (user_id_set_role >> 8) & 0xFF;                    // High
          s_DATA_SYNC.u8_data_packet[1] = user_id_set_role & 0xFF; // Low

          for (int i = 0; i < strlen(user_role) + 1; i++)
          {
            s_DATA_SYNC.u8_data_packet[2 + i] = user_role[i];
            printf("Data Sync: %d\n", s_DATA_SYNC.u8_data_packet[2 + i]);
          }
          s_DATA_SYNC.u8_data_length = 2 + strlen(user_role) + 1;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case DELETE_FINGER_USER:

          DECODE_User_ID(data, &user_id_delete);

          s_DATA_SYNC.u8_data_start = LOCAL_DATABASE_FINGER_DELETE;

          s_DATA_SYNC.u8_data_packet[0] = (user_id_delete >> 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = user_id_delete & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 2;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          break;

        case DELETE_FACEID_USER:
          DECODE_User_ID(data, &user_id_delete);

          s_DATA_SYNC.u8_data_start = LOCAL_DATABASE_FACEID_DELETE;

          s_DATA_SYNC.u8_data_packet[0] = (user_id_delete >> 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = user_id_delete & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 2;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          xQueueSend(
              *s_mqtt_client_data.p_data_local_database_queue, &s_DATA_SYNC, 0);
          break;

        case ADD_USER_DATA_CMD:

          DECODE_Add_User_Data(
              data, (int *)&s_sdcard_data.u16_user_id, s_sdcard_data.user_name);

          // Copy user ID
          user_id[user_len] = s_sdcard_data.u16_user_id;

          // Allocate memory and copy user name
          user_name[user_len]
              = (char *)heap_caps_malloc(MAX_NAME_LEN, MALLOC_CAP_SPIRAM);

          strncpy(
              user_name[user_len], s_sdcard_data.user_name, MAX_NAME_LEN - 1);
          user_name[user_len][MAX_NAME_LEN - 1]
              = '\0'; // Ensure null-termination

          // Allocate memory and copy role
          role[user_len] = (char *)heap_caps_malloc(6, MALLOC_CAP_SPIRAM);
          strncpy(role[user_len], "user", 5);
          role[user_len][5] = '\0'; // Ensure null-termination

          face[user_len]   = 0;
          finger[user_len] = 0;

          user_len++;

          s_sdcard_cmd = SDCARD_ADD_USER_DATA;
          xQueueSend(*s_mqtt_client_data.p_data_sdcard_queue, &s_sdcard_cmd, 0);

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
      s_mqtt_client_data.b_mqtt_client_connected = true;
      esp_mqtt_client_subscribe_single(
          s_mqtt_client_data.s_MQTT_Client, u32_topic_response_client, 0);
      esp_mqtt_client_subscribe_single(
          s_mqtt_client_data.s_MQTT_Client, u32_topic_request_client, 0);

      xEventGroupSetBits(*s_mqtt_client_data.p_flag_time_event,
                         TIME_SOURCE_RTC_OFF);

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

      *s_mqtt_client_data.p_state_system = STATE_WIFI_DISCONNECTED;

      // Send state wifi to MCU1
      s_DATA_SYNC.u8_data_start     = DATA_SYNC_STATE_CONNECTION_WIFI;
      s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_FAIL;
      s_DATA_SYNC.u8_data_length    = 1;
      s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
      xQueueSend(*s_mqtt_client_data.p_send_data_queue, &s_DATA_SYNC, 0);

      s_mqtt_client_data.b_mqtt_client_connected = false;

      xEventGroupSetBits(*s_mqtt_client_data.p_flag_time_event,
                         TIME_SOURCE_RTC_READY);

      break;
    default:
      ESP_LOGI(TAG, "Other event");
      break;
  }
}
