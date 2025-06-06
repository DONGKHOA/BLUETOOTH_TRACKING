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
 *    PUBLIC DATA
 *****************************************************************************/

sdcard_data_t s_sdcard_data;

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
  QueueHandle_t     *p_data_mqtt_queue;
  QueueHandle_t     *p_data_attendance_queue;
  QueueHandle_t     *p_send_data_queue;
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
static UINT    bw;
static FIL     fil, fsrc, fdst;

char *p_file_user_data  = "userData.csv";
char *p_file_attendance = "attendance.csv";
char *p_tempfile        = "temp.csv";

static control_sdcard_t s_control_sdcard;

static bool b_get_data = false;

static DATA_SYNC_t s_DATA_SYNC;

static uint8_t  attendance_number_checkin[16] = { 0 };
static uint32_t attendance_time[16][16]       = { 0 };
static uint16_t attendance_user_id[16]        = { 0 };
static uint8_t  packet_user_id[16]            = { 0 };
static uint32_t attendance_index              = 0;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_CONTROL_SDCARD_Task(void *arg);

static control_sdcard_status_t APP_CONTROL_SDCARD_CheckValid(void);
static control_sdcard_status_t APP_CONTROL_SDCARD_CheckNew(void);
static void                    APP_CONTROL_SDCARD_CreateFile(void);
static void                    APP_CONTROL_SDCARD_WriteUserData(void);
static void                    APP_CONTROL_SDCARD_ReadUserData(void);
static void APP_CONTROL_SDCARD_WriteAttendanceData(char    *new_name,
                                                   int      user_id,
                                                   uint32_t timestamp);
static void APP_CONTROL_SDCARD_ReadAttendanceData(void);
static void APP_CONTROL_SDCARD_RemoveQuotes(char *str);

static void APP_CONTROL_SDCARD_ModifyInfoUserData(
    char *new_name, int user_id, int new_face, int new_finger, char *new_role);

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
  s_control_sdcard.p_data_mqtt_queue   = &s_data_system.s_data_mqtt_queue;
  s_control_sdcard.p_data_attendance_queue
      = &s_data_system.s_data_attendance_queue;
  s_control_sdcard.p_send_data_queue = &s_data_system.s_send_data_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_CONTROL_SDCARD_Task (void *arg)
{
  sdcard_cmd_t      s_sdcard_cmd;
  attendance_data_t s_attendance_data;
  while (1)
  {
    // taskENTER_CRITICAL(&spi_mux);
    xSemaphoreTake(*s_control_sdcard.p_spi_mutex, 100 / portTICK_PERIOD_MS);

    if (APP_CONTROL_SDCARD_CheckValid() == SDCARD_INVALID)
    {
      b_valid_sdcard = false;
      ESP_LOGI(TAG, "SD Card is invalid!");

      // Send command to MCU1 the state of SD Card
      s_DATA_SYNC.u8_data_start     = DATA_SYNC_STATE_SDCARD;
      s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_FAIL;
      s_DATA_SYNC.u8_data_length    = 1;
      s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
      xQueueSend(*s_control_sdcard.p_send_data_queue, &s_DATA_SYNC, 0);

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

        // Send command to MCU1 the state of SD Card
        s_DATA_SYNC.u8_data_start     = DATA_SYNC_STATE_SDCARD;
        s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_SUCCESS;
        s_DATA_SYNC.u8_data_length    = 1;
        s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
        xQueueSend(*s_control_sdcard.p_send_data_queue, &s_DATA_SYNC, 0);

        if (APP_CONTROL_SDCARD_CheckNew() == SDCARD_NEW)
        {
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
        case SDCARD_SYNC_DATA_SERVER:

          s_DATA_SYNC.u8_data_start     = LOCAL_DATABASE_DATA;
          s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_DUMMY;
          s_DATA_SYNC.u8_data_length    = 1;
          s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

          xQueueSend(*s_control_sdcard.p_data_mqtt_queue, &s_DATA_SYNC, 0);

          break;

        case SDCARD_ATTENDANCE_DATA:

          APP_CONTROL_SDCARD_ReadAttendanceData();

          for (int j = 0; j < attendance_index; j++)
          {
            for (int k = 0; k < attendance_number_checkin[j]; k++)
            {
              s_attendance_data.u32_time[k] = attendance_time[j][k];
            }

            int temp                            = packet_user_id[j];
            s_attendance_data.u16_user_id       = attendance_user_id[temp];
            s_attendance_data.u8_number_checkin = attendance_number_checkin[j];
            s_attendance_data.u32_index_packet  = j;

            xQueueSend(*s_control_sdcard.p_data_attendance_queue,
                       &s_attendance_data,
                       0);

            // send command to mqtt
            s_DATA_SYNC.u8_data_start     = LOCAL_DATABASE_DATA_ATTENDANCE;
            s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_DUMMY;
            s_DATA_SYNC.u8_data_length    = 1;
            s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

            xQueueSend(*s_control_sdcard.p_data_mqtt_queue, &s_DATA_SYNC, 0);
          }

          break;

        case SDCARD_ADD_USER_DATA:
          APP_CONTROL_SDCARD_WriteUserData();
          break;

        case SDCARD_ENROLL_FACE:
          APP_CONTROL_SDCARD_ModifyInfoUserData(
              NULL, s_sdcard_data.u16_user_id, 1, -1, NULL);
          break;

        case SDCARD_ENROLL_FINGERPRINT:
          APP_CONTROL_SDCARD_ModifyInfoUserData(
              NULL, s_sdcard_data.u16_user_id, -1, 1, NULL);
          break;

        case SDCARD_ATTENDANCE:
          APP_CONTROL_SDCARD_WriteAttendanceData(s_sdcard_data.user_name,
                                                 s_sdcard_data.u16_user_id,
                                                 s_sdcard_data.u32_time);
          break;

        case SDCARD_DELETE_USER_DATA:
          APP_CONTROL_SDCARD_WriteUserData();
          break;

        case SDCARD_SET_ROLE:
          APP_CONTROL_SDCARD_ModifyInfoUserData(
              NULL, s_sdcard_data.u16_user_id, -1, -1, s_sdcard_data.role);
          break;

        case SDCARD_DELETE_FINGER_USER:
          APP_CONTROL_SDCARD_ModifyInfoUserData(
              NULL, s_sdcard_data.u16_user_id, -1, 0, NULL);
          break;

        case SDCARD_DELETE_FACEID_USER:
          APP_CONTROL_SDCARD_ModifyInfoUserData(
              NULL, s_sdcard_data.u16_user_id, 0, -1, NULL);
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
  char full_path[32];

  FRESULT fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return SDCARD_INVALID;
  }

  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_user_data);

  // Check if the file exists
  fr = f_open(&fil, full_path, FA_READ);
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

  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_user_data);

  // Create user data file
  fr = f_open(&fil, full_path, FA_CREATE_ALWAYS | FA_WRITE);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_attendance);

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
             "%d,\"%s\",%d,%d,\"%s\"\n",
             user_id[i],
             user_name[i],
             face[i],
             finger[i],
             role[i]);

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
  char full_path[32];
  char line[128];

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
    line[strcspn(line, "\r\n")] = '\0';

    // Get ID
    char *token = strtok(line, ",");
    if (!token)
    {
      continue;
    }
    user_id[idx] = atoi(token);

    // Get username
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    APP_CONTROL_SDCARD_RemoveQuotes(token);
    user_name[idx] = strdup(token);

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
    APP_CONTROL_SDCARD_RemoveQuotes(token);
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
APP_CONTROL_SDCARD_ModifyInfoUserData (
    char *new_name, int user_id, int new_face, int new_finger, char *new_role)
{
  char full_path[32];
  char temp_path[32];

  char line[128], new_line[128];

  // Mount the SD card
  if (f_mount(&fs, MOUNT_POINT, 1) != FR_OK)
  {
    printf("f_mount failed!\n");
    return;
  }

  // Open the source file for reading
  // Open the temporary file for writing
  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_user_data);
  snprintf(temp_path, sizeof(temp_path), "%s/%s", MOUNT_POINT, p_tempfile);

  if (f_open(&fsrc, full_path, FA_READ) != FR_OK
      || f_open(&fdst, temp_path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
  {
    printf("f_open failed!\n");
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }
  // Read each line from the source file
  while (f_gets(line, sizeof(line), &fsrc))
  {
    char *id_str = strtok(line, ",\r\n");
    char *name   = strtok(NULL, ",\r\n");
    char *face   = strtok(NULL, ",\r\n");
    char *finger = strtok(NULL, ",\r\n");
    char *role   = strtok(NULL, ",\r\n");

    if (!id_str || !name || !face || !finger || !role)
    {
      printf("Invalid line format: %s\n", line);
      continue;
    }

    int current_id = atoi(id_str);
    int face_val   = atoi(face);
    int finger_val = atoi(finger);

    APP_CONTROL_SDCARD_RemoveQuotes(name);
    APP_CONTROL_SDCARD_RemoveQuotes(role);

    char *final_name = name;
    char *final_role = role;

    if (current_id == user_id)
    {
      if (new_face != -1)
      {
        face_val = new_face;
      }
      if (new_finger != -1)
      {
        finger_val = new_finger;
      }
      if (new_name != NULL)
      {
        final_name = new_name;
      }
      if (new_role != NULL)
      {
        final_role = new_role;
      }
    }

    snprintf(new_line,
             sizeof(new_line),
             "%d,\"%s\",%d,%d,\"%s\"\n",
             current_id,
             final_name,
             face_val,
             finger_val,
             final_role);

    printf("Update line: %s", new_line);

    f_write(&fdst, new_line, strlen(new_line), &bw);
  }

  f_close(&fsrc);
  f_close(&fdst);

  f_unlink(full_path);
  f_rename(temp_path, full_path);
}

static void
APP_CONTROL_SDCARD_WriteAttendanceData (char    *new_name,
                                        int      user_id,
                                        uint32_t timestamp)
{
  char full_path[32];
  char temp_path[32];
  char line[1024], new_line[1024];

  int updated = 0;

  // Mount SD Card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  // Open the source file for reading
  // Open the temporary file for writing
  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_attendance);
  snprintf(temp_path, sizeof(temp_path), "%s/%s", MOUNT_POINT, p_tempfile);

  // Open file in read mode to check if the user ID already exists
  if (f_open(&fsrc, full_path, FA_READ) != FR_OK
      || f_open(&fdst, temp_path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
  {
    printf("f_open failed!\n");
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  // Read the exist line
  while (f_gets(line, sizeof(line), &fsrc))
  {
    line[strcspn(line, "\r\n")] = '\0';

    char *id_str = strtok(line, ",\r\n");
    char *name   = strtok(NULL, ",\r\n");
    char *time   = strtok(NULL, ",\r\n");

    if (!id_str || !name || !time)
    {
      printf("Invalid line format: %s\n", line);
      continue;
    }

    int current_id = atoi(id_str);

    APP_CONTROL_SDCARD_RemoveQuotes(new_name);
    APP_CONTROL_SDCARD_RemoveQuotes(name);

    if (current_id == user_id)
    {
      updated = 1;
      snprintf(new_line, sizeof(new_line), "%d,\"%s\"", current_id, new_name);

      while (time != NULL)
      {
        strncat(new_line, ",", sizeof(new_line) - strlen(new_line) - 1);
        strncat(new_line, time, sizeof(new_line) - strlen(new_line) - 1);
        time = strtok(NULL, ",\r\n");
      }

      char temp[20];
      snprintf(temp, sizeof(temp), ",%lu\n", timestamp);
      strncat(new_line, temp, sizeof(new_line) - strlen(new_line) - 1);

      strncat(new_line, "\n", sizeof(new_line) - strlen(new_line) - 1);
      memset(temp, 0, sizeof(temp));
    }
    else
    {
      snprintf(new_line, sizeof(new_line), "%d,\"%s\"", current_id, name);

      while (time != NULL)
      {
        strncat(new_line, ",", sizeof(new_line) - strlen(new_line) - 1);
        strncat(new_line, time, sizeof(new_line) - strlen(new_line) - 1);
        time = strtok(NULL, ",\r\n");
      }

      strncat(new_line, "\n", sizeof(new_line) - strlen(new_line) - 1);
    }

    printf("Update line: %s\n", new_line);
    if (strlen(new_line) > 0)
    {
      f_write(&fdst, new_line, strlen(new_line), &bw);
    }
  }

  // Write new attendance data
  if (updated == 0)
  {
    snprintf(new_line,
             sizeof(new_line),
             "%d,\"%s\",%lu\n",
             user_id,
             new_name,
             timestamp);
    printf("New attendance line: %s\n", new_line);
    if (strlen(new_line) > 0)
    {
      f_write(&fdst, new_line, strlen(new_line), &bw);
    }
  }

  f_close(&fsrc);
  f_close(&fdst);

  f_unlink(full_path);
  f_rename(temp_path, full_path);
}

static void
APP_CONTROL_SDCARD_ReadAttendanceData (void)
{
  char full_path[32];
  char line[1024];

  uint8_t  user_index    = 0;
  uint32_t checkin_count = 0;

  attendance_index = 0;
  memset(attendance_number_checkin, 0, sizeof(attendance_number_checkin));

  // Mount SD Card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_attendance);

  fr = f_open(&fil, full_path, FA_READ);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }
  // Read file line by line
  while (f_gets(line, sizeof(line), &fil))
  {
    line[strcspn(line, "\r\n")] = '\0';

    // Get ID
    char *token = strtok(line, ",");
    if (!token)
    {
      continue;
    }
    attendance_user_id[user_index] = atoi(token);

    // Skip username
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }

    checkin_count = 0;

    // Get time
    while ((token = strtok(NULL, ",")) != NULL)
    {
      // Check if the packet is full -> Increase attendance index
      if (checkin_count == 16)
      {
        attendance_number_checkin[attendance_index] = checkin_count;
        packet_user_id[attendance_index]            = user_index;
        attendance_index++;

        // Prevent overflow
        if (attendance_index >= 16)
        {
          break;
        }

        // Reset checkin number
        checkin_count = 0;
      }

      attendance_time[attendance_index][checkin_count++]
          = strtoul(token, NULL, 10);
    }

    // Increase the attendance index after checkin complete
    if (checkin_count > 0 && attendance_index < 16)
    {
      attendance_number_checkin[attendance_index] = checkin_count;
      packet_user_id[attendance_index]            = user_index;
      attendance_index++;
    }

    if (attendance_index >= 16)
    {
      break;
    }

    user_index++;
  }

  f_close(&fil);
  f_mount(NULL, MOUNT_POINT, 1);
}

static void
APP_CONTROL_SDCARD_RemoveQuotes (char *str)
{
  // Remove space at the beginning and end of the string
  while (*str == ' ')
  {
    memmove(str, str + 1, strlen(str));
  }

  uint32_t len = strlen(str);
  while (len > 0 && str[len - 1] == ' ')
  {
    str[--len] = '\0';
  }

  // Remove surrounding quotes
  len = strlen(str);
  if (len >= 2 && str[0] == '"' && str[len - 1] == '"')
  {
    memmove(str, str + 1, len - 2);
    str[len - 2] = '\0';
  }
}