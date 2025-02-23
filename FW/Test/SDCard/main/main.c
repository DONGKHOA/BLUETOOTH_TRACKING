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

char *p_namefile = "test2.txt";
char *p_data     = "Hello";

FRESULT fr;
FATFS   fs;
FIL     fil;
UINT    bw;
UINT    br;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void SDCard_ReadFile(char *p_namefile);
static void SDCard_WriteFile(char *p_namefile, char *p_data);

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
  BSP_spiDMADriverInit(HSPI_HOST, 3);

  // Write file to SD Card
  SDCard_WriteFile(p_namefile, p_data);

  // Read file from SD Card
  SDCard_ReadFile(p_namefile);

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
SDCard_ReadFile (char *p_namefile)
{
  char full_path[128];
  char buffer[128];

  // Mount SD Card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
  }

  // Create a full path to the file
  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_namefile);

  // Open the file at read mode (FA_READ)
  fr = f_open(&fil, full_path, FA_READ);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1); // If not successful, unmount the SD card
  }

  // Reset the buffer
  // Read the file with maximum size of buffer buffer - 1, reverse a place for
  // null-terminator
  memset(buffer, 0, sizeof(buffer));
  fr = f_read(&fil, buffer, sizeof(buffer) - 1, &br);
  if (fr == FR_OK)
  {
    buffer[br] = '\0'; // Null-terminate the string
    printf("Read from file: %s\n", buffer);
  }
  else
  {
    printf("f_read failed! error=%d\n", fr);
  }

  f_close(&fil);
}

static void
SDCard_WriteFile (char *p_namefile, char *p_data)
{
  char full_path[128];

  // Mount SD Card
  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
  }

  // Create a full path to the file
  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_namefile);

  // Open the file at write mode (FA_WRITE) and create a new file if it does not
  // exist (FA_CREATE_ALWAYS)
  fr = f_open(&fil, full_path, FA_WRITE | FA_CREATE_ALWAYS);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
  }

  // Write the data to the file
  fr = f_write(&fil, p_data, strlen(p_data), &bw);
  if (fr == FR_OK && bw == strlen(p_data))
  {
    printf("Write file successfully: %s\n", full_path);
  }
  else
  {
    printf("Write file failed! error=%d, written=%u\n", fr, bw);
  }

  f_close(&fil);
}
