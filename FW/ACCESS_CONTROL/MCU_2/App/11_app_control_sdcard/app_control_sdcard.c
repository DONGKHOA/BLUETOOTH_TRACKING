/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>
#include <stdio.h>

#include "app_control_sdcard.h"
#include "app_data.h"

#include "environment.h"

#include "esp_log.h"

#include "ff.h"
#include "diskio.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG         "APP_CONTROL_SDCARD"
#define MOUNT_POINT "0:" // Mount point

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t     *p_data_sdcard_queue;
  SemaphoreHandle_t *p_spi_mutex;
} control_sdcard_t;

typedef enum
{
  SDCARD_VALID,
  SDCARD_INVALID,
} __attribute__((packed)) control_sdcard_status_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static FRESULT fr;
static FATFS   fs;
static FIL     fil;
static UINT    bw;
static UINT    br;

char *p_namefile = "test2.csv";

static control_sdcard_t s_control_sdcard;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_CONTROL_SDCARD_Task(void *arg);

static control_sdcard_status_t APP_CONTROL_SDCARD_CheckValid(void);
static void                    APP_CONTROL_SDCARD_WriteCSV(void);
static void                    APP_CONTROL_SDCARD_ReadCSVFile(void);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_CONTROL_SDCARD_CreateTask (void)
{
  xTaskCreate(APP_CONTROL_SDCARD_Task, "control sd", 1024 * 10, NULL, 11, NULL);
}

void
APP_CONTROL_SDCARD_Init (void)
{
  s_control_sdcard.p_spi_mutex         = &s_data_system.s_spi_mutex;
  s_control_sdcard.p_data_sdcard_queue = &s_data_system.s_data_sdcard_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_CONTROL_SDCARD_Task (void *arg)
{
  sdcard_cmd_t s_sdcard_cmd;
  while (1)
  {
    if (xQueueReceive(*s_control_sdcard.p_data_sdcard_queue,
                      &s_sdcard_cmd,
                      portMAX_DELAY)
        == pdPASS)
    {
      taskENTER_CRITICAL(&spi_mux);
      xSemaphoreTake(*s_control_sdcard.p_spi_mutex, portMAX_DELAY);

      switch (s_sdcard_cmd)
      {
        case SDCARD_USER_DATA:
          break;

        case SDCARD_ENROLL_FACE:
          break;

        case SDCARD_ENROLL_FINGERPRINT:
          break;

        case SDCARD_ATTENDANCE:
          break;

        case SDCARD_DELETE_USER_DATA:
          break;

        case SDCARD_SET_ROLE:
          break;

        case SDCARD_DELETE_FINGER_USER:
          break;

        case SDCARD_DELETE_FACEID_USER:
          break;

        default:
          break;
      }

      xSemaphoreGive(*s_control_sdcard.p_spi_mutex);
      taskEXIT_CRITICAL(&spi_mux);
    }
  }
}

static control_sdcard_status_t
APP_CONTROL_SDCARD_CheckValid (void)
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
APP_CONTROL_SDCARD_WriteCSV (void)
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
APP_CONTROL_SDCARD_ReadCSVFile (void)
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