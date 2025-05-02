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

char *p_namefile = "attendance.csv";
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

typedef enum
{
  SDCARD_VALID,
  SDCARD_INVALID,
} __attribute__((packed)) local_database_status_t;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void SDCard_ReadFile(char *p_namefile);
static void SDCard_WriteFile(char *p_namefile, char *p_data);
static void SDCard_WriteCSV(char   **user_name,
                            int     *user_id,
                            int     *face,
                            int     *finger,
                            char   **role,
                            uint16_t user_len);
static void SDCard_ReadCSVFile(char     *p_namefile,
                               char    **user_name,
                               int      *user_id,
                               int      *face,
                               int      *finger,
                               char    **role,
                               uint16_t *user_len);

static local_database_status_t APP_LOCAL_DATABASE_SDCard_CheckValid(void);
static void                    SDCard_WritePositionAttendanceCSV(int         id,
                                                                 const char *new_name,
                                                                 const char *new_date,
                                                                 const char *new_time);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

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
  user_len = 10;

  char **user_name = malloc(user_len * sizeof(char *));
  char **role      = malloc(user_len * sizeof(char *));
  int   *user_id   = malloc(user_len * sizeof(int));
  int   *face      = malloc(user_len * sizeof(int));
  int   *finger    = malloc(user_len * sizeof(int));

  // user_name[0] = strdup("Dong Thanh Khoa");
  // user_id[0]   = 1;
  // face[0]      = 1;
  // finger[0]    = 1;
  // role[0]      = strdup("user");

  // user_name[1] = strdup("Nguyen Van A");
  // user_id[1]   = 2;
  // face[1]      = 0;
  // finger[1]    = 1;
  // role[1]      = strdup("admin");

  // SDCard_WriteCSV(user_name, user_id, face, finger, role, user_len);

  // SDCard_ReadCSVFile(
  //     p_namefile, user_name, user_id, face, finger, role, &user_len);
  APP_LOCAL_DATABASE_SDCard_CheckValid();

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
SDCard_WriteCSV (char   **user_name,
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
SDCard_ReadCSVFile (char     *p_namefile,
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
  *p_user_len = idx;
  printf("User length: %d\n", *p_user_len);

  f_close(&fil);
  f_mount(NULL, MOUNT_POINT, 1);
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

// Local database send ID, Name and Attendance time
static void
SDCard_WritePositionAttendanceCSV (int         id,
                                   const char *new_name,
                                   const char *new_date,
                                   const char *new_time)
{
  FIL  fsrc, fdst;
  char full_path[128];
  char temp_path[128];
  char line[256], new_line[256];
  ;

  int current_line_id;

  // Mount SD Card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  // Open the source file for reading
  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_namefile);
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
    char *id_str = strtok(line, ",\r\n");
    char *name   = strtok(NULL, ",\r\n");
    char *date   = strtok(NULL, ",\r\n");
    char *time   = strtok(NULL, ",\r\n");

    if (!id_str || !name || !date || !time)
    {
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
    }
    else
    {
      // Reconstruct original line
      snprintf(new_line,
               sizeof(new_line),
               "%s,%s,%s,%s\n",
               id_str,
               name,
               date,
               time);
    }

    f_write(&fdst, new_line, strlen(new_line), &bw);
  }

  f_close(&fsrc);
  f_close(&fdst);

  // Replace old file
  f_unlink(full_path);            // Delete original
  f_rename(temp_path, full_path); // Rename temp to original
}