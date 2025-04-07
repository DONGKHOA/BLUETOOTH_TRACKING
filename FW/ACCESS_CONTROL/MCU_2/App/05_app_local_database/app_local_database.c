/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_local_database.h"
#include "app_data.h"

#include "environment.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_LOCAL_DATABASE"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t *p_data_mqtt_queue;
  QueueHandle_t *p_send_data_queue;
  QueueHandle_t *p_data_local_database_queue;
} local_database_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static local_database_t s_local_database;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_LOCAL_DATABASE_Task(void *arg);

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
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_LOCAL_DATABASE_Task (void *arg)
{
  DATA_SYNC_t s_DATA_SYNC;

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

          break;

        case DATA_SYNC_REQUEST_AUTHENTICATION:

          break;

        case LOCAL_DATABASE_RESPONSE_ENROLL_FACE:

          break;

        case LOCAL_DATABASE_RESPONSE_ENROLL_FINGERPRINT:

          break;

        case LOCAL_DATABASE_RESPONSE_DELETE_USER_DATA:

          break;

        default:
          break;
      }
    }
  }
}