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
  SDCARD_NEW,
  SDCARD_OLD,
  SDCARD_UNKNOWN
} __attribute__((packed)) control_sdcard_status_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static FRESULT fr;
static FATFS   fs;
static FIL     fil;
static UINT    bw;
static UINT    br;

char *p_file_user_data  = "userData.csv";
char *p_file_attendance = "attendance.csv";

static control_sdcard_t s_control_sdcard;

static bool b_get_data = false;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_CONTROL_SDCARD_Task(void *arg);

static control_sdcard_status_t APP_CONTROL_SDCARD_CheckValid(void);
static control_sdcard_status_t APP_CONTROL_SDCARD_CheckNew(void);
static void                    APP_CONTROL_SDCARD_CreateFile(void);
static void                    APP_CONTROL_SDCARD_WriteUserData(void);
static void                    APP_CONTROL_SDCARD_ReadUserData(void);
static void                    APP_CONTROL_SDCARD_WriteAttendanceData(void);
static void                    APP_CONTROL_SDCARD_ReadAttendanceData(void);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_CONTROL_SDCARD_CreateTask (void)
{
  xTaskCreate(APP_CONTROL_SDCARD_Task, "control sd", 1024 * 20, NULL, 11, NULL);
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
    // taskENTER_CRITICAL(&spi_mux);
    xSemaphoreTake(*s_control_sdcard.p_spi_mutex, 100 / portTICK_PERIOD_MS);

    if (APP_CONTROL_SDCARD_CheckValid() == SDCARD_INVALID)
    {
      b_valid_sdcard = false;
      ESP_LOGI(TAG, "SD Card is invalid!");
      vTaskDelay(500 / portTICK_PERIOD_MS);
      goto exit_critical_section;
    }
    else
    {
      b_valid_sdcard = true;
      ESP_LOGI(TAG, "SD Card is valid!");
      if (b_get_data == false)
      {
        b_get_data = true;
        if (APP_CONTROL_SDCARD_CheckNew() == SDCARD_NEW)
        {
          printf("SD Card is new!\n");
          APP_CONTROL_SDCARD_CreateFile();
        }
        else if (APP_CONTROL_SDCARD_CheckNew() == SDCARD_OLD)
        {
          APP_CONTROL_SDCARD_ReadUserData();
        }
      }
    }

    if (xQueueReceive(*s_control_sdcard.p_data_sdcard_queue,
                      &s_sdcard_cmd,
                      100 / portTICK_PERIOD_MS)
        == pdPASS)
    {

      switch (s_sdcard_cmd)
      {
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
    }

  exit_critical_section:
    xSemaphoreGive(*s_control_sdcard.p_spi_mutex);
    // taskEXIT_CRITICAL(&spi_mux);
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

static control_sdcard_status_t
APP_CONTROL_SDCARD_CheckNew (void)
{
  FRESULT fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return SDCARD_INVALID;
  }

  // Check if the file exists
  fr = f_open(&fil, p_file_user_data, FA_READ);
  if (fr == FR_OK)
  {
    f_close(&fil);
    f_mount(NULL, MOUNT_POINT, 1);
    return SDCARD_OLD;
  }

  f_mount(NULL, MOUNT_POINT, 1);
  return SDCARD_NEW;
}

static void
APP_CONTROL_SDCARD_CreateFile (void)
{
  char full_path[128];

  FRESULT fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_user_data);

  // Create user data file
  fr = f_open(&fil, full_path, FA_CREATE_ALWAYS | FA_WRITE);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_attendance);

  // Create attendance file
  fr = f_open(&fil, full_path, FA_CREATE_ALWAYS | FA_WRITE);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_close(&fil);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  f_close(&fil);
  f_mount(NULL, MOUNT_POINT, 1);
}

static void
APP_CONTROL_SDCARD_WriteUserData (void)
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

  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_user_data);

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
APP_CONTROL_SDCARD_ReadUserData (void)
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

  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_user_data);

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

static void
APP_CONTROL_SDCARD_WriteAttendanceData (void)
{
}

static void
APP_CONTROL_SDCARD_ReadAttendanceData (void)
{
}