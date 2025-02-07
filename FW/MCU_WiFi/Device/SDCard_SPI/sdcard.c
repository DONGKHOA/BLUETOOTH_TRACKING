/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdcard.h"
#include "ff.h"
#include "diskio.h"

/*****************************************************************************
 *   PRIVATE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *   PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/

FRESULT fr;
FATFS   fs;
FIL     fil;
UINT    bw;
UINT    br;

#define MOUNT_POINT "0:" // Định nghĩa mount point

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/
void
DEV_SDCard_Init (void)
{
  BSP_sdSpiInit(HSPI_HOST,
                MISO_PIN,
                MOSI_PIN,
                SCLK_PIN,
                MAX_TRANSFER_SZ,
                CS_PIN,
                DMA_CHANNEL,
                SD_INIT_FREQ,
                SD_WORK_FREQ,
                SPI_MODE);
}

void
SDCard_ReadFile (void)
{
  static char buffer[128]; // Buffer để chứa dữ liệu đọc

  fr = f_mount(&fs, "0:", 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
    return;
  }

  fr = f_open(&fil, "0:/hello5.txt", FA_READ);
  if (fr == FR_OK)
  {
    printf("File opened successfully!\n");

    do
    {
      memset(buffer, 0, sizeof(buffer));
      fr = f_read(&fil, buffer, sizeof(buffer) - 1, &br);
      if (fr == FR_OK)
      {
        printf("%s", buffer);
      }
    } while (br > 0);

    f_close(&fil);
    printf("File read complete!\n");
  }
  else
  {
    printf("f_open failed! error=%d\n", fr);
  }

  f_mount(NULL, "0:", 1);
}

void
DEV_SDCard_WriteFile (const char *p_filename, const char *p_data)
{

  char full_path[128];

  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
  }

  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_filename);

  fr = f_open(&fil, full_path, FA_WRITE | FA_CREATE_ALWAYS);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
  }

  fr = f_write(&fil, p_data, strlen(p_data), &bw);
  if (fr == FR_OK && bw == strlen(p_data))
  {
    printf("Ghi file thành công: %s\n", full_path);
  }
  else
  {
    printf("f_write failed! error=%d, written=%u\n", fr, bw);
  }

  f_close(&fil);

  f_mount(NULL, MOUNT_POINT, 1);
}

void
DEV_SDCard_Read_File (const char *p_path)
{
  char full_path[128];
  char line[128];

  fr = f_mount(&fs, MOUNT_POINT, 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed! error=%d\n", fr);
  }

  snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_path);

  fr = f_open(&fil, full_path, FA_READ);
  if (fr != FR_OK)
  {
    printf("f_open failed! error=%d\n", fr);
    f_mount(NULL, MOUNT_POINT, 1);
  }

  memset(line, 0, sizeof(line));
  fr = f_read(&fil, line, sizeof(line) - 1, &br);
  if (fr == FR_OK)
  {
    line[br] = '\0';
    printf("Read from file: %s\n", line);
  }
  else
  {
    printf("f_read failed! error=%d\n", fr);
  }

  f_close(&fil);

  f_mount(NULL, MOUNT_POINT, 1);
}

/*****************************************************************************
 *   PRIVATE FUNCTIONS
 *****************************************************************************/