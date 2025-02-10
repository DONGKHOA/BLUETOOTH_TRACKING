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
// #include "esp_event.h"
#include "esp_log.h"

// #include "uart.h"
#include "driver/gpio.h"
#include "string.h"

// #include "app_fingerprint.h"
// #include "ds3231.h"
#include "dht22.h"
#include "sdcard.h"
#include "ads1115.h"
#include "mcp4822.h"

#include "rs3485.h"

#include "ff.h"
#include "diskio.h"
#include "sdspi.h"

#include "driver/twai.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define DHT11_PIN GPIO_NUM_4
#define GPIO_NUM  GPIO_NUM_14

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

dht22_data_t dht22;

char *p_path = "hahaaa.txt";
char *p_data = "Hello";

/******************************************************************************
 *  PRIVATE FUNCTIONS
 *****************************************************************************/

static void DHT22_Task(void *pvParameters);
static void SDCard_Task(void *pvParameters);
static void ADS1115_Task(void *pvParameter);
static void MCP4822_Task(void *pvParameter);
static void SDCard_ReadFile(char *p_path);
static void SDCard_WriteFile(char *p_path, char *p_data);
static void RS3485_Task(void *pvParameter);

/******************************************************************************
 *       MAIN FUNCTION
 *****************************************************************************/

void
twai_init ()
{
  // // Cấu hình chân TX và RX
  // twai_general_config_t g_config
  //     = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_19, GPIO_NUM_20, TWAI_MODE_NORMAL);

  // // Cấu hình bộ lọc (chấp nhận tất cả ID)
  // twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // // Cấu hình tốc độ baud 500kbps
  // // twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

  // // Cài đặt TWAI driver
  // if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
  // {
  //   printf("TWAI driver installed\n");
  // }
  // else
  // {
  //   printf("Failed to install TWAI driver\n");
  // }

  // // Bật TWAI driver
  // if (twai_start() == ESP_OK)
  // {
  //   printf("TWAI driver started\n");
  // }
  // else
  // {
  //   printf("Failed to start TWAI driver\n");
  // }
}

void
twai_send_message ()
{
  // twai_message_t message;
  // message.identifier       = 0x123; // ID của frame CAN
  // message.extd             = 0;     // Standard frame (11-bit ID)
  // message.rtr              = 0; // Dữ liệu bình thường (không phải Remote Frame)
  // message.data_length_code = 8; // 8 byte dữ liệu
  // memcpy(message.data, "ESP2CAN!", 8); // Nội dung gửi

  // if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
  // {
  //   printf("Message sent\n");
  // }
  // else
  // {
  //   printf("Failed to send message\n");
  // }
}

void
app_main (void)
{

  // xTaskCreate(MCP4822_Task, "MCP4822_Task", 4096, NULL, 10, NULL);
  // xTaskCreate(ADS1115_Task, "ADS1115_Task", 4096, NULL, 10, NULL);
  twai_init();
  vTaskDelay(pdMS_TO_TICKS(1000)); // Đợi TWAI khởi động

  // Gửi dữ liệu
  
  while (1)
  {
    twai_send_message();
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
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
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
    uint16_t adc_value = DEV_ADS1115_GetData(DEV_ADS1115_CHANNEL_0,
                                             ADS1115_REG_CONFIG_PGA_4_096V);
    float    voltage   = DEV_ADS1115_GetVoltage(adc_value);

    printf("Value: %d, Voltage: %.3f V\n", adc_value, voltage);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void
MCP4822_Task (void *pvParameter)
{
  DEV_MCP4822_Init();
  while (1)
  {
    DEV_MCP4822_SetValue(MCP4822_DAC_A, MCP4822_OUTPUT_GAIN_x2, 3000);
    // DEV_MPC4822_Output();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void
SDCard_ReadFile (char *p_path)
{
  DEV_SDCard_Read_File(p_path);
}

static void
SDCard_WriteFile (char *p_path, char *p_data)
{
  DEV_SDCard_WriteFile(p_path, p_data);
}

static void
RS3485_Task (void *pvParameter)
{
  DEV_RS3485_Init();

  rs3485_request_t request = { .slave_id  = 0x01,
                               .function  = RS3485_FUNC_READ_HOLDING_REGS,
                               .reg_addr  = 0x0010,
                               .reg_count = 1 };

  while (1)
  {
    DEV_RS3485_SendRequest(&request);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
