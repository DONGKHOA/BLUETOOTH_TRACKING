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

#include "app_fingerprint.h"
// #include "ds3231.h"
#include "dht22.h"
#include "sdcard.h"
#include "ads1115.h"
#include "mcp4822.h"

#include "ff.h"
#include "diskio.h"
#include "sdspi.h"

#include "driver/twai.h"
#include "freertos/timers.h"

#include "app_rtc.h"
#include "app_read_data.h"
#include "sn65hvd230dr.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define DHT11_PIN GPIO_NUM_4
#define GPIO_NUM  GPIO_NUM_14

#define CAN_MODE      TWAI_MODE_NO_ACK
#define TXD_PIN       GPIO_NUM_19
#define RXD_PIN       GPIO_NUM_20
#define TXD_QUEUE_LEN 1024
#define RXD_QUEUE_LEN 1024
#define INTR_FLAG     ESP_INTR_FLAG_LEVEL1 // lowest priority
#define CAN_BITRATE   5

#define CAN_ID   0x123
#define CAN_EXTD 0
#define CAN_RTR  0

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

// dht22_data_t dht22;

// char *p_path = "hahaaa.txt";
// char *p_data = "Hello";

uint32_t count = 0;

char *p_text = "ESP2CAN";

uint8_t default_address_1[4]  = { 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t default_password_1[4] = { 0x00, 0x00, 0x00, 0x00 };
/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

// static void DHT22_Task(void *pvParameters);
// static void SDCard_Task(void *pvParameters);
// static void MCP4822_Task(void *pvParameter);
// static void SDCard_ReadFile(char *p_path);
// static void SDCard_WriteFile(char *p_path, char *p_data);
// static void RS3485_Task(void *pvParameter);

static void Timer_Callback(TimerHandle_t xTimer);
/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
twai_init (void)
{

  BSP_canConfigDefault();
  BSP_canConfigMode(CAN_MODE);
  BSP_canConfigIO(TXD_PIN, RXD_PIN);
  BSP_canConfigQueue(TXD_QUEUE_LEN, RXD_QUEUE_LEN);
  BSP_canConfigIntr(INTR_FLAG);
  BSP_canConfigBitRate(CAN_BITRATE);

  // Cài đặt TWAI driver
  if (BSP_canDriverInit() == ESP_OK)
  {
    printf("TWAI driver installed\n");
  }
  else
  {
    printf("Failed to install TWAI driver\n");
  }

  // Bật TWAI driver
  if (BSP_canStart() == ESP_OK)
  {
    printf("TWAI driver started\n");
  }
  else
  {
    printf("Failed to start TWAI driver\n");
  }
}

void
twai_send_message (void)
{
  DEV_CAN_SendMessage(
      CAN_ID, CAN_EXTD, CAN_RTR, (uint8_t *)p_text, strlen(p_text));
}

void
twai_receive_message (void)
{
  uint32_t id, extd, rtr;
  uint8_t  data[8]; // Buffer để lưu dữ liệu nhận được
  uint8_t  len;

  if (DEV_CAN_ReceiveMessage(&id, &extd, &rtr, data, &len))
  {
    printf("Received message");
  }
  else
  {
    printf("Failed to receive message.\n");
  }
}
void
app_main (void)
{
  // Create Soft-Timer
  TimerHandle_t timer = xTimerCreate("Timer",
                                     pdMS_TO_TICKS(1), // Period 1ms
                                     pdTRUE,           // Auto reload
                                     NULL,
                                     Timer_Callback // Callback function
  );

  xTimerStart(timer, 0);

  // APP_ReadData_Init();
  // APP_RTC_Init();
  APP_FingerPrint_Init();

  // APP_ReadData_CreateTask();
  // APP_RTC_CreateTask();
  // APP_FingerPrint_CreateTask();

  twai_init();
  while (1)
  {
    DEV_AS608_VfyPwd(UART_PORT_NUM_2, default_address_1, default_password_1);
    twai_receive_message();
    twai_send_message();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
Timer_Callback (TimerHandle_t xTimer)
{
  // DEV_ADS1115_TimeOut();
  DEV_AS608_TimeOut();
}

// static void
// DHT22_Task (void *pvParameters)
// {
//   DEV_DHT22_Init(&dht22, DHT11_PIN);
//   while (1)
//   {
//     DEV_DHT22_GetData(&dht22, DHT11_PIN);

//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// static void
// SDCard_Task (void *pvParameters)
// {

//   while (1)
//   {
//     {
//       vTaskDelay(pdMS_TO_TICKS(1000));
//     }
//   }
// }

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

// static void
// RS3485_Task (void *pvParameter)
// {
//   DEV_RS3485_Init();

//   rs3485_request_t request = { .slave_id  = 0x01,
//                                .function  = RS3485_FUNC_READ_HOLDING_REGS,
//                                .reg_addr  = 0x0010,
//                                .reg_count = 1 };

//   while (1)
//   {
//     DEV_RS3485_SendRequest(&request);
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// static void
// SDCard_ReadFile (char *p_path)
// {
//   DEV_SDCard_Read_File(p_path);
// }

// static void
// SDCard_WriteFile (char *p_path, char *p_data)
// {
//   DEV_SDCard_WriteFile(p_path, p_data);
// }
