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
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "esp_log.h"

#include "uart.h"
#include "driver/gpio.h"
#include "string.h"

// #include "app_fingerprint.h"
// #include "ds3231.h"
#include "spi.h"
#include "dht22.h"
// #include "sdcard.h"
#include "ads1115.h"
#include "mcp4822.h"
#include "ff.h"
#include "sdspi.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

// #define HSPI_HOST SPI2_HOST
#define DHT11_PIN GPIO_NUM_4
#define GPIO_NUM  GPIO_NUM_14

#define BUFFER_SIZE 128 * 1024

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

dht22_data_t dht22;
char        *p_filename = "test.txt";
char        *p_data     = "Hello ESP32 FATFS!";

static FATFS g_sFatFs; // Biến chứa thông tin FATFS cho 1 volume
static FIL   g_sFile;  // Biến đại diện cho 1 file

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void DHT22_Task(void *pvParameters);
static void SDCard_Task(void *pvParameters);
static void ADS1115_Task(void *pvParameter);
static void MCP4822_Task(void *pvParameter);
/******************************************************************************
 *       MAIN FUNCTION
 *****************************************************************************/

int
bufsize (char *buf)
{
  int i = 0;
  while (*buf++ != '\0')
  {
    i++;
  }
  return i;
}

void
test_fatfs_write_file (void)
{
  FRESULT     fr; // biến nhận code trả về của FATFS
  UINT        bw; // số byte thực tế đã ghi
  const char *test_str = "Hello from FATFS!\r\n";

  // 1) Mount SD vào volume "0:"
  //    Tham số thứ 3 = 1 nghĩa là mount ngay (0 thì delay mount)
  fr = f_mount(&g_sFatFs, "0:", 1);
  if (fr != FR_OK)
  {
    printf("f_mount failed, error = %d\n", fr);
    return;
  }
  printf("f_mount OK.\n");

  // 2) Mở (hoặc tạo mới) file test.txt trong thẻ
  //    FA_WRITE | FA_CREATE_ALWAYS => ghi + tạo file mới
  fr = f_open(&g_sFile, "0:/test.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if (fr == FR_OK)
  {
    printf("File opened/created successfully.\n");

    // 3) Ghi chuỗi test_str vào file
    fr = f_write(&g_sFile, test_str, strlen(test_str), &bw);
    if ((fr == FR_OK) && (bw == strlen(test_str)))
    {
      printf("Write OK: \"%s\"\n", test_str);
    }
    else
    {
      printf("Write error, fr=%d, bw=%u\n", fr, bw);
    }

    // 4) Đóng file
    f_close(&g_sFile);
    printf("File closed.\n");
  }
  else
  {
    printf("f_open failed, error = %d\n", fr);
  }

  // 5) Unmount khi không còn xài
  f_mount(NULL, "0:", 1);
  printf("Unmount done.\n");
}

void
app_main (void)
{
  // FRESULT fr = FR_NOT_READY;
  // FATFS   SDFatFs_main; /* File system object for SD card logical drive */
  // FIL     MyFile;       /* File object */

  // char buffer[BUFFER_SIZE]; // to store strings..
  // UINT br, bw;

  // uint32_t total_size    = 1UL * 1024 * 1024 * 1024; // 1GB
  // uint32_t bytes_written = 0;

  // memset(buffer, 0xFF, BUFFER_SIZE); // Fill buffer with data

  // fr = f_mount(&SDFatFs_main, "", 1);
  // fr = f_open(&MyFile, "file3.txt", FA_CREATE_ALWAYS | FA_WRITE);

  // /* Writing text */
  // while (bytes_written < total_size)
  // {
  //   fr = f_write(&MyFile, buffer, bufsize(buffer), &bw);

  //   bytes_written += bw;
  // }
  // f_close(&MyFile);

  // uint8_t response = BSP_sdSpiInit();
  // printf("%d\n", response);

  test_fatfs_write_file();

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/
static void
DHT22_Task (void *pvParameters)
{
  DEV_DHT22_Init(&dht22, DHT11_PIN);
  while (1)
  {
    DEV_DHT22_GetData(&dht22, DHT11_PIN);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void
SDCard_Task (void *pvParameters)
{

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void
ADS1115_Task (void *pvParameter)
{
  DEV_ADS1115_Init();
  BSP_gpioOutputInit(GPIO_NUM);
  BSP_gpioSetState(GPIO_NUM, 1);
  while (1)
  {
    uint16_t adc_value = DEV_ADS1115_GetData(DEV_ADS1115_CHANNEL_1,
                                             ADS1115_REG_CONFIG_PGA_4_096V);
    float    voltage   = DEV_ADS1115_GetVoltage(adc_value);

    printf("Value: %d, Voltage: %.3f V", adc_value, voltage);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void
MCP4822_Task (void *pvParameter)
{
  DEV_MCP4822_Init();
  while (1)
  {
    DEV_MCP4822_SetValue(MCP4822_DAC_A, MCP4822_OUTPUT_GAIN_x2, 2048);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}