/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_local_database.h"
#include "app_data.h"

#include "environment.h"

#include <string.h>

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_LOCAL_DATABASE"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t     *p_data_mqtt_queue;
  QueueHandle_t     *p_send_data_queue;
  QueueHandle_t     *p_data_local_database_queue;
  SemaphoreHandle_t *p_spi_mutex;
} local_database_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static local_database_t s_local_database;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_LOCAL_DATABASE_Task(void *arg);
static void APP_LOCAL_DATABASE_Delete_UserName(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserRole(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserFace(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserFinger(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserID(uint16_t user_id_delete);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_LOCAL_DATABASE_CreateTask (void)
{
  xTaskCreate(APP_LOCAL_DATABASE_Task, "local db", 1024 * 4, NULL, 13, NULL);
}

void
APP_LOCAL_DATABASE_Init (void)
{
  s_local_database.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;
  s_local_database.p_send_data_queue = &s_data_system.s_send_data_queue;
  s_local_database.p_data_local_database_queue
      = &s_data_system.s_data_local_database_queue;
  s_local_database.p_spi_mutex = &s_data_system.s_spi_mutex;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_LOCAL_DATABASE_Task (void *arg)
{
  DATA_SYNC_t   s_DATA_SYNC;
  const uint8_t state_lookup[4] = {
    1, // face=0, finger=0
    3, // face=0, finger=1
    2, // face=1, finger=0
    4  // face=1, finger=1
  };
  uint16_t u16_id;
  char     user_name_local_task[32];

  while (1)
  {
    if (xQueueReceive(*s_local_database.p_data_local_database_queue,
                      &s_DATA_SYNC,
                      portMAX_DELAY)
        == pdPASS)
    {
      switch (s_DATA_SYNC.u8_data_start)
      {
        case DATA_SYNC_REQUEST_USER_DATA:

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          if (u16_id > user_len)
          {
            s_DATA_SYNC.u8_data_start     = DATA_SYNC_RESPONSE_USER_DATA;
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_DUMMY;
            s_DATA_SYNC.u8_data_length    = 1;
            s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
            xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);
          }
          else
          {
            register uint8_t u8_state
                = state_lookup[(face[u16_id - 1] << 1) | finger[u16_id - 1]];

            memcpy(user_name_local_task, user_name[u16_id - 1], 32);

            s_DATA_SYNC.u8_data_start     = DATA_SYNC_RESPONSE_USER_DATA;
            s_DATA_SYNC.u8_data_packet[0] = u8_state;
            s_DATA_SYNC.u8_data_length    = 1;
            s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
            xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

            char *token = strtok(user_name_local_task, " ");
            while (token != NULL)
            {

              s_DATA_SYNC.u8_data_start = DATA_SYNC_RESPONSE_USERNAME_DATA;
              memcpy(s_DATA_SYNC.u8_data_packet, token, strlen(token));
              s_DATA_SYNC.u8_data_length = strlen(token);
              s_DATA_SYNC.u8_data_stop   = DATA_STOP_FRAME;

              xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

              token = strtok(NULL, " ");
            }

            s_DATA_SYNC.u8_data_start     = DATA_SYNC_RESPONSE_USERNAME_DATA;
            s_DATA_SYNC.u8_data_packet[0] = '\n';
            s_DATA_SYNC.u8_data_packet[1] = '\n';
            s_DATA_SYNC.u8_data_length    = 2;
            s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
            xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);
          }

          break;

        case DATA_SYNC_REQUEST_AUTHENTICATION:

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          register uint8_t u8_state = DATA_SYNC_FAIL;

          if (memcmp(role[u16_id - 1], "admin", sizeof("admin")) == 0)
          {
            u8_state = DATA_SYNC_SUCCESS;
          }

          s_DATA_SYNC.u8_data_start     = DATA_SYNC_RESPONSE_AUTHENTICATION;
          s_DATA_SYNC.u8_data_packet[0] = u8_state;
          s_DATA_SYNC.u8_data_length    = 1;
          s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
          xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case LOCAL_DATABASE_RESPONSE_ENROLL_FACE:

          // Update data local database

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start = DATA_SYNC_RESPONSE_ENROLL_FACE;

          // Notify the status of response to transmit task via queue
          xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case LOCAL_DATABASE_RESPONSE_ENROLL_FINGERPRINT:

          // Update data local database

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start = DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT;

          // Notify the status of response to transmit task via queue
          xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case LOCAL_DATABASE_REQUEST_DELETE_USER_DATA:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          // Update data in psram

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          APP_LOCAL_DATABASE_Delete_UserName(u16_id);
          APP_LOCAL_DATABASE_Delete_UserRole(u16_id);
          APP_LOCAL_DATABASE_Delete_UserFace(u16_id);
          APP_LOCAL_DATABASE_Delete_UserFinger(u16_id);
          APP_LOCAL_DATABASE_Delete_UserID(u16_id);
          user_len--;

          s_DATA_SYNC.u8_data_start = LOCAL_DATABASE_RESPONSE_DELETE_USER_DATA;
          s_DATA_SYNC.u8_data_packet[0] = LOCAL_DATABASE_SUCCESS;
          s_DATA_SYNC.u8_data_length    = 1;
          s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

          xQueueSend(*s_local_database.p_data_mqtt_queue, &s_DATA_SYNC, 0);

          xSemaphoreGive(*s_local_database.p_spi_mutex);

          taskEXIT_CRITICAL(&spi_mux);

          break;

        case LOCAL_DATABASE_USER_DATA:

          taskENTER_CRITICAL(&spi_mux);

          // Update data local database

          taskEXIT_CRITICAL(&spi_mux);

          break;

        default:
          break;
      }
    }
  }
}

static void
APP_LOCAL_DATABASE_Delete_UserName (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  heap_caps_free(user_name[index]);
}

static void
APP_LOCAL_DATABASE_Delete_UserRole (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  heap_caps_free(role[index]);
}

static void
APP_LOCAL_DATABASE_Delete_UserFace (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  face[index] = 0;
}

static void
APP_LOCAL_DATABASE_Delete_UserFinger (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  finger[index] = 0;
}

static void
APP_LOCAL_DATABASE_Delete_UserID (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }
  user_id[index] = 0;
}