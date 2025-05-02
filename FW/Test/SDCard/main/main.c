/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "main.h"

#include "ff.h"
#include "diskio.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define MOUNT_POINT "0:" // Mount point

/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

spi_device_handle_t spi_sdcard_handle;

char *p_file_user_data  = "userData.csv";
char *p_file_attendance = "attendance.csv";

char *p_tempfile = "temp.csv";
char *p_data     = "Hello";

char   **user_name;
int     *user_id;
int     *face;
int     *finger;
char   **role;
uint16_t user_len = 0;

FRESULT fr;
FATFS   fs;
FIL     fil;
UINT    bw;
UINT    br;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void SDCard_WriteUserData(char   **user_name,
                                 int     *user_id,
                                 int     *face,
                                 int     *finger,
                                 char   **role,
                                 uint16_t user_len);
static void SDCard_ReadUserData(char     *p_namefile,
                                char    **user_name,
                                int      *user_id,
                                int      *face,
                                int      *finger,
                                char    **role,
                                uint16_t *user_len);

static void SDCard_WritePositionUserData(char *new_name,
                                         int   user_id,
                                         int  *new_face,
                                         int  *new_finger,
                                         char *new_role);

static void SDCard_WriteAttendance(char **user_name,
                                   int   *user_id,
                                   char  *date,
                                   char  *time);

static void SDCard_WritePositionAttendance(int         id,
                                           const char *new_name,
                                           const char *new_date,
                                           const char *new_time);

static void APP_SDCard_Task(void *arg);
static void remove_quotes(char *str);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
APP_SDCard_CreateTask (void)
{
  xTaskCreate(APP_SDCard_Task, "sdcard task", 1024 * 20, NULL, 14, NULL);
}

void
app_main (void)
{
  // Init SPI
  BSP_spiConfigIO(MISO_PIN, MOSI_PIN, SCLK_PIN);
  BSP_spiConfigCS(CS_PIN);
  BSP_spiMaxTransferSize(MAX_TRANSFER_SZ);
  BSP_spiConfigDefault();
  BSP_spiClockSpeed(SPI_CLOCK_400KHz);
  BSP_spiTransactionQueueSize(1);
  BSP_spiConfigMode(SPI_MODE);
  BSP_spiDMADriverInit(&spi_sdcard_handle, HSPI_HOST, 3);

  // Init SD Card
  user_len = 2;

  user_name = malloc(user_len * sizeof(char *));
  role      = malloc(user_len * sizeof(char *));
  user_id   = malloc(user_len * sizeof(int));
  face      = malloc(user_len * sizeof(int));
  finger    = malloc(user_len * sizeof(int));

  APP_SDCard_CreateTask();
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_SDCard_Task (void *arg)
{
  uint8_t init = 0;

  user_name[0] = strdup("Dong Thanh Khoa");
  user_id[0]   = 1;
  face[0]      = 1;
  finger[0]    = 1;
  role[0]      = strdup("user");

  user_name[1] = strdup("Nguyen Van A");
  user_id[1]   = 2;
  face[1]      = 0;
  finger[1]    = 1;
  role[1]      = strdup("admin");

  int face_value   = 0;
  int finger_value = 0;

  // SDCard_WriteUserData(user_name, user_id, face, finger, role, user_len);
  // SDCard_WritePositionUserData(NULL, 1, NULL, &finger_value, "admin");
  // SDCard_WritePositionAttendance(1, "Nguyen Van B", "2023-10-01",
  // "08:00:00");

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void
SDCard_WriteUserData (char   **user_name,
                      int     *user_id,
                      int     *face,
                      int     *finger,
                      char   **role,
                      uint16_t user_len)
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
SDCard_ReadUserData (char     *p_namefile,
                     char    **user_name,
                     int      *user_id,
                     int      *face,
                     int      *finger,
                     char    **role,
                     uint16_t *p_user_len)
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
    // Get ID
    char *token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    user_id[idx] = atoi(token);

    // Get username
    token = strtok(line, ",");
    if (!token)
    {
      continue;
    }
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
  *p_user_len = idx;
  printf("User length: %d\n", *p_user_len);

  f_close(&fil);
  f_mount(NULL, MOUNT_POINT, 1);
}

static void
SDCard_WritePositionUserData (
    char *new_name, int user_id, int *new_face, int *new_finger, char *new_role)
{
  FIL  fsrc, fdst;
  char full_path[32];
  char temp_path[32];
  char line[128], new_line[128];
  char face_buf[2], finger_buf[2];

  // Mount SD Card
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

    remove_quotes(name);
    remove_quotes(role);

    // Check if the current line ID matches the user ID to be updated
    // If it does, update the line with new values
    if (current_id == user_id)
    {
      // If the new values are NULL, keep the original values
      // Otherwise, use the new values
      face_buf[0] = new_face ? '0' + *new_face : face[0];
      face_buf[1] = '\0';

      finger_buf[0] = new_finger ? '0' + *new_finger : finger[0];
      finger_buf[1] = '\0';

      // Format the new line with updated values
      snprintf(new_line,
               sizeof(new_line),
               "%d,\"%s\",%s,%s,\"%s\"\n",
               user_id,
               new_name ? new_name : name,
               face_buf,
               finger_buf,
               new_role ? new_role : role);
    }
    else
    {
      // If not new value, keep the original line
      snprintf(new_line,
               sizeof(new_line),
               "%s,\"%s\",%s,%s,\"%s\"\n",
               id_str,
               name,
               face,
               finger,
               role);
    }
    printf("Update line: %s", new_line);
    f_write(&fdst, new_line, strlen(new_line), &bw);
  }

  f_close(&fsrc);
  f_close(&fdst);

  f_unlink(full_path);
  f_rename(temp_path, full_path);
}

// Local database send ID, Name and Attendance time
static void
SDCard_WritePositionAttendance (int         id,
                                const char *new_name,
                                const char *new_date,
                                const char *new_time)
{
  FIL  fsrc, fdst;
  char full_path[128];
  char temp_path[128];
  char line[256], new_line[256];

  int current_line_id;

  // Mount SD Card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  // Open the source file for reading
  snprintf(
      full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_file_attendance);
  snprintf(temp_path, sizeof(temp_path), "%s/%s", MOUNT_POINT, p_tempfile);

  // Open source file for reading
  fr = f_open(&fsrc, full_path, FA_READ);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  // Open temporary file for writing
  fr = f_open(&fdst, temp_path, FA_WRITE | FA_CREATE_ALWAYS);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
    return;
  }

  while (f_gets(line, sizeof(line), &fsrc))
  {
    printf("Read line: %s", line);
    char *id_str = strtok(line, ",\r\n");
    char *name   = strtok(NULL, ",\r\n");
    char *date   = strtok(NULL, ",\r\n");
    char *time   = strtok(NULL, ",\r\n");

    if (!id_str || !name || !date || !time)
    {
      printf("Invalid line format: %s\n", line);
      continue;
    }

    current_line_id = atoi(id_str);

    if (current_line_id == id)
    {
      const char *final_name = new_name ? new_name : name;
      const char *final_date = new_date ? new_date : date;
      const char *final_time = new_time ? new_time : time;

      snprintf(new_line,
               sizeof(new_line),
               "%d,%s,%s,%s\n",
               id,
               final_name,
               final_date,
               final_time);

      printf("Update line: %s", new_line);
    }
    f_write(&fdst, new_line, strlen(new_line), &bw);
  }

  f_close(&fsrc);
  f_close(&fdst);

  // Replace old file
  f_unlink(full_path);            // Delete original
  f_rename(temp_path, full_path); // Rename temp to original
}

static void
SDCard_WriteAttendance (char **user_name, int *user_id, char *date, char *time)
{
}

static void
SDCard_ReadAttendance (char **user_name, int *user_id, char *date, char *time)
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
    // Get ID
    char *token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    user_id[idx] = atoi(token);

    // Get username
    token = strtok(line, ",");
    if (!token)
    {
      continue;
    }
    user_name[idx] = strdup(token);

    // Get date
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    date[idx] = atoi(token);

    // Get time
    token = strtok(NULL, ",");
    if (!token)
    {
      continue;
    }
    time[idx] = atoi(token);

    printf("Row %d: name=%s, id=%d, date=%d, time=%d, role=%s\n",
           idx,
           user_name[idx],
           user_id[idx],
           date[idx],
           time[idx]);

    idx++;
  }

  f_close(&fil);
  f_mount(NULL, MOUNT_POINT, 1);
}

static void
remove_quotes (char *str)
{
  if (str[0] == '"' && str[strlen(str) - 1] == '"')
  {
    memmove(str, str + 1, strlen(str)); // remove first quote
    str[strlen(str) - 1] = '\0';        // remove last quote
  }
}