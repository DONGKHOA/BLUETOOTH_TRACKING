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

static void
init_new (void)
{
  
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

static void
test_send_cmd0_new (void)
{
  // 1) CS lên cao
  gpio_set_level(CS_PIN, 1);

  // 2) Gửi 80 clock dummy
  uint8_t ff_buf[10];
  memset(ff_buf, 0xFF, sizeof(ff_buf));

  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = 10 * 8; // 80 bit
  t.tx_buffer = ff_buf; // Gửi 10 byte 0xFF
  t.rx_buffer = NULL;   // Không cần đọc
  spi_device_polling_transmit(spi_handle, &t);

  // 3) Gửi CMD0
  // CMD0 = 0x40, argument=0, CRC=0x95
  uint8_t cmd0[6] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };

  // Kéo CS=0
  gpio_set_level(CS_PIN, 0);

  // Gửi 6 byte CMD0
  memset(&t, 0, sizeof(t));
  t.length    = 6 * 8; // 6 byte
  t.tx_buffer = cmd0;
  t.rx_buffer = NULL; // Chỉ gửi
  spi_device_polling_transmit(spi_handle, &t);

  // Đọc tối đa 8 byte response
  // Ở half-duplex, ta gửi 8 byte dummy => đọc 8 byte
  uint8_t tx_dummy[8], rx_resp[8];
  memset(tx_dummy, 0xFF, sizeof(tx_dummy));
  memset(&t, 0, sizeof(t));
  t.length    = 8 * 8; // 8 byte
  t.tx_buffer = tx_dummy;
  t.rx_buffer = rx_resp;
  spi_device_polling_transmit(spi_handle, &t);

  // Thả CS
  gpio_set_level(CS_PIN, 1);

  // In kết quả
  for (int i = 0; i < 8; i++)
  {
    printf("Resp[%d] = 0x%02X\n", i, rx_resp[i]);
  }
}


void
app_main (void)
{ 
  // test_fatfs_write_file();

  // init_new();

  // Cấu hình GPIO cho CS
  // gpio_set_direction(CS_PIN, GPIO_MODE_OUTPUT);
  // gpio_set_level(CS_PIN, 1);

  // // 1) Cấu hình SPI bus
  // spi_bus_config_t buscfg = { .miso_io_num     = MISO_PIN,
  //                             .mosi_io_num     = MOSI_PIN,
  //                             .sclk_io_num     = SCLK_PIN,
  //                             .quadwp_io_num   = -1,
  //                             .quadhd_io_num   = -1,
  //                             .max_transfer_sz = 4096 };
  // spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_DISABLED);

  // // 2) add device: SPI_DEVICE_HALFDUPLEX + mode=0 + freq=100k
  // spi_device_interface_config_t devcfg
  //     = { .clock_speed_hz = 100000, // 100 kHz init
  //         .mode           = 0,      // mode 0
  //         .spics_io_num   = -1,     // tự điều khiển CS
  //         .queue_size     = 1,
  //         .flags          = SPI_DEVICE_HALFDUPLEX };
  // spi_bus_add_device(HSPI_HOST, &devcfg, &spi_handle);

  // test_send_cmd0_new();

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