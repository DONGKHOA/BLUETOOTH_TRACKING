/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "app_local_database.h"
#include "app_data.h"

#include "environment.h"

#include "esp_log.h"

#include "ff.h"
#include "diskio.h"

/******************************************************************************
 *    PUBLIC DATA
 *****************************************************************************/

extern uint16_t u16_finger_user_id;

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG         "APP_LOCAL_DATABASE"
#define MOUNT_POINT "0:" // Mount point

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t      *p_data_mqtt_queue;
  QueueHandle_t      *p_send_data_queue;
  QueueHandle_t      *p_data_local_database_queue;
  EventGroupHandle_t *p_fingerprint_event;
  SemaphoreHandle_t  *p_spi_mutex;
} local_database_t;

typedef enum
{
  SDCARD_VALID,
  SDCARD_INVALID,
} __attribute__((packed)) local_database_status_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static local_database_t s_local_database;

static FRESULT fr;
static FATFS   fs;
static FIL     fil;
static UINT    bw;
static UINT    br;

char *p_namefile = "test2.csv";

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_LOCAL_DATABASE_Task(void *arg);
static void APP_LOCAL_DATABASE_Delete_UserName(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserRole(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserFace(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserFinger(uint16_t user_id_delete);
static void APP_LOCAL_DATABASE_Delete_UserID(uint16_t user_id_delete);

static local_database_status_t APP_LOCAL_DATABASE_SDCard_CheckValid(void);
static void                    APP_LOCAL_DATABASE_SDCard_WriteCSV(void);
static void                    APP_LOCAL_DATABASE_SDCard_ReadCSVFile(void);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_LOCAL_DATABASE_CreateTask (void)
{
  xTaskCreate(APP_LOCAL_DATABASE_Task, "local db", 1024 * 10, NULL, 11, NULL);
}

void
APP_LOCAL_DATABASE_Init (void)
{
  s_local_database.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;
  s_local_database.p_send_data_queue = &s_data_system.s_send_data_queue;
  s_local_database.p_data_local_database_queue
      = &s_data_system.s_data_local_database_queue;
  s_local_database.p_spi_mutex         = &s_data_system.s_spi_mutex;
  s_local_database.p_fingerprint_event = &s_data_system.s_fingerprint_event;

  uint8_t sdcard_valid = APP_LOCAL_DATABASE_SDCard_CheckValid();
  printf("SDCard valid: %d\n", sdcard_valid);
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
  uint16_t index;
  char     user_name_local_task[32];
  bool     is_valid = true;

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

          index    = 0;
          is_valid = true;
          while (u16_id != user_id[index])
          {

            if (index >= user_len)
            {
              is_valid = false;
              break;
            }

            index++;
          }

          if (!is_valid)
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
                = state_lookup[(face[index] << 1) | finger[index]];

            memcpy(user_name_local_task, user_name[index], 32);

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

          index    = 0;
          is_valid = true;
          while (u16_id != user_id[index])
          {

            if (index >= user_len)
            {
              is_valid = false;
              break;
            }

            index++;
          }

          if (!is_valid)
          {
            s_DATA_SYNC.u8_data_start     = DATA_SYNC_RESPONSE_USER_DATA;
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_DUMMY;
            s_DATA_SYNC.u8_data_length    = 1;
            s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
            xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);
          }
          else
          {
            register uint8_t u8_state = DATA_SYNC_FAIL;

            if (memcmp(role[index], "admin", sizeof("admin")) == 0)
            {
              u8_state = DATA_SYNC_SUCCESS;
            }

            s_DATA_SYNC.u8_data_start     = DATA_SYNC_RESPONSE_AUTHENTICATION;
            s_DATA_SYNC.u8_data_packet[0] = u8_state;
            s_DATA_SYNC.u8_data_length    = 1;
            s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
            xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);
          }

          break;

        case DATA_SYNC_ENROLL_FACE:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          // Update data in psram

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          index    = 0;
          is_valid = true;
          while (u16_id != user_id[index])
          {

            if (index >= user_len)
            {
              is_valid = false;
              break;
            }

            index++;
          }

          if (!is_valid)
          {
            break;
          }

          face[index] = 1;

          xQueueSend(*s_local_database.p_data_mqtt_queue, &s_DATA_SYNC, 0);

          break;

        case DATA_SYNC_ENROLL_FINGERPRINT:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          // Update data in psram

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          index    = 0;
          is_valid = true;
          while (u16_id != user_id[index])
          {

            if (index >= user_len)
            {
              is_valid = false;
              break;
            }

            index++;
          }

          if (!is_valid)
          {
            break;
          }

          finger[index] = 1;

          xQueueSend(*s_local_database.p_data_mqtt_queue, &s_DATA_SYNC, 0);

          break;

        case LOCAL_DATABASE_REQUEST_DELETE_USER_DATA:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          // Update data in psram

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          index    = 0;
          is_valid = true;
          while (u16_id != user_id[index])
          {

            if (index >= user_len)
            {
              is_valid = false;
              break;
            }

            index++;
          }

          if (!is_valid)
          {
            break;
          }

          APP_LOCAL_DATABASE_Delete_UserName(u16_id);
          APP_LOCAL_DATABASE_Delete_UserRole(u16_id);
          APP_LOCAL_DATABASE_Delete_UserFace(u16_id);
          APP_LOCAL_DATABASE_Delete_UserFinger(u16_id);
          APP_LOCAL_DATABASE_Delete_UserID(u16_id);
          user_len--;

          u16_finger_user_id = u16_id;
          xEventGroupSetBits(*s_local_database.p_fingerprint_event,
                             EVENT_DELETE_FINGERPRINT);

          s_DATA_SYNC.u8_data_start     = DATA_SYNC_DELETE_FACE_ID;
          s_DATA_SYNC.u8_data_packet[0] = (index << 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = index & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 1;
          s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

          xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case LOCAL_DATABASE_USER_DATA:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          break;

        case LOCAL_DATABASE_SET_ROLE:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          // Update data in psram

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          index    = 0;
          is_valid = true;
          while (u16_id != user_id[index])
          {

            if (index >= user_len)
            {
              is_valid = false;
              break;
            }

            index++;
          }

          if (!is_valid)
          {
            break;
          }

          break;

        case LOCAL_DATABASE_FINGER_DELETE:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          // Update data in psram
          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          APP_LOCAL_DATABASE_Delete_UserFinger(u16_id);
          // Update in fingerprint hardware

          u16_finger_user_id = u16_id;
          xEventGroupSetBits(*s_local_database.p_fingerprint_event,
                             EVENT_DELETE_FINGERPRINT);

          break;

        case LOCAL_DATABASE_FACEID_DELETE:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          // Update data in psram

          u16_id = (s_DATA_SYNC.u8_data_packet[0] << 8)
                   | s_DATA_SYNC.u8_data_packet[1];

          index    = 0;
          is_valid = true;
          while (u16_id != user_id[index])
          {

            if (index >= user_len)
            {
              is_valid = false;
              break;
            }

            index++;
          }

          if (!is_valid)
          {
            break;
          }

          face[index] = 0;

          // Send data to the queue for transmission to MCU1
          s_DATA_SYNC.u8_data_start = DATA_SYNC_DELETE_FACE_ID;

          s_DATA_SYNC.u8_data_packet[0] = (index >> 8) & 0xFF; // High
          s_DATA_SYNC.u8_data_packet[1] = index & 0xFF;        // Low
          s_DATA_SYNC.u8_data_length    = 2;

          s_DATA_SYNC.u8_data_stop = DATA_STOP_FRAME;

          // Notify the status of response to transmit task via queue
          xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

          break;

        case LOCAL_DATABASE_RESPONSE_ATTENDANCE:

          taskENTER_CRITICAL(&spi_mux);
          xSemaphoreTake(*s_local_database.p_spi_mutex, portMAX_DELAY);

          // Update data in sdcard

          xSemaphoreGive(*s_local_database.p_spi_mutex);
          taskEXIT_CRITICAL(&spi_mux);

          // Update data in psram

          s_DATA_SYNC.u8_data_start = DATA_SYNC_RESPONSE_ATTENDANCE;

          xQueueSend(*s_local_database.p_send_data_queue, &s_DATA_SYNC, 0);

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

  if (index == (user_len - 1))
  {
    heap_caps_free(user_name[index]);
    return;
  }

  for (uint16_t i = index + 1; i < user_len; i++)
  {
    memcpy(user_name[i - 1], user_name[i], 32);
  }

  heap_caps_free(user_name[user_len - 1]);
  return;
}

static void
APP_LOCAL_DATABASE_Delete_UserRole (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  if (index == (user_len - 1))
  {
    heap_caps_free(role[index]);
    return;
  }

  for (uint16_t i = index + 1; i < user_len; i++)
  {
    memcpy(role[i - 1], role[i], 6);
  }

  heap_caps_free(role[user_len - 1]);
  return;
}

static void
APP_LOCAL_DATABASE_Delete_UserFace (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  if (index == (user_len - 1))
  {
    face[index] = 0;
    return;
  }

  for (uint16_t i = index + 1; i < user_len; i++)
  {
    face[i - 1] = face[i];
  }

  face[user_len - 1] = 0;
}

static void
APP_LOCAL_DATABASE_Delete_UserFinger (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  if (index == (user_len - 1))
  {
    finger[index] = 0;
    return;
  }

  for (uint16_t i = index + 1; i < user_len; i++)
  {
    finger[i - 1] = finger[i];
  }

  finger[user_len - 1] = 0;
}

static void
APP_LOCAL_DATABASE_Delete_UserID (uint16_t user_id_delete)
{
  uint16_t index = 0;
  while (user_id_delete != user_id[index])
  {
    index++;
  }

  if (index == (user_len - 1))
  {
    user_id[index] = 0;
    return;
  }

  for (uint16_t i = index + 1; i < user_len; i++)
  {
    user_id[i - 1] = user_id[i];
  }

  user_id[user_len - 1] = 0;
}

static local_database_status_t
APP_LOCAL_DATABASE_SDCard_CheckValid (void)
{
  FRESULT fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return SDCARD_INVALID;
  }

  f_mount(NULL, MOUNT_POINT, 1);
  return SDCARD_VALID;
}

static void
APP_LOCAL_DATABASE_SDCard_WriteCSV (void)
{
  char full_path[128];
  char row_buffer[256];

  // Mount SD Card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_namefile);

  // Open file in write mode to overwrite or create
  fr = f_open(&fil, full_path, FA_WRITE | FA_CREATE_ALWAYS);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  for (int i = 0; i < user_len; i++)
  {
    // Format each row into CSV
    snprintf(row_buffer,
             sizeof(row_buffer),
             "\"%s\",%d,%d,%d,\"%s\",%d\n",
             user_name[i],
             user_id[i],
             face[i],
             finger[i],
             role[i],
             i);

    fr = f_write(&fil, row_buffer, strlen(row_buffer), &bw);
    if (fr != FR_OK || bw != strlen(row_buffer))
    {
      printf("Write failed for index %d! error=%d, written=%u\n", i, fr, bw);
    }
  }

  f_close(&fil);
  f_mount(NULL, MOUNT_POINT, 1);

  printf("All user rows written to %s\n", full_path);
}

static void
APP_LOCAL_DATABASE_SDCard_ReadCSVFile (void)
{
  char full_path[128];
  char line[256];

  int idx = 0;

  // Mount the SD card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_namefile);

  fr = f_open(&fil, full_path, FA_READ);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  // Read file line by line
  while (f_gets(line, sizeof(line), &fil) != NULL)
  {
    // Get username
    char *token = strtok(line, ",");
    if (!token)
    {
      continue;
    }
    user_name[idx] = strdup(token);

    // Get ID
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    user_id[idx] = atoi(token);

    // Get face
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    face[idx] = atoi(token);

    // Get finger
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    finger[idx] = atoi(token);

    // Get role
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    role[idx] = strdup(token);

    printf("Row %d: name=%s, id=%d, face=%d, finger=%d, role=%s\n",
           idx,
           user_name[idx],
           user_id[idx],
           face[idx],
           finger[idx],
           role[idx]);

    idx++;
  }
  user_len = idx;
  printf("User length: %d\n", user_len);

  f_close(&fil);
  f_mount(NULL, MOUNT_POINT, 1);
}