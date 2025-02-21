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
#include "esp_log.h"

#include "sn65hvd230dr.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

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

char *p_text = "ESP2CAN";

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void IRAM_ATTR my_custom_isr(void *arg);
static void           TestMain_CAN_Init(void);
static void           TestMain_CAN_SendMessage(void);
static void           TestMain_CAN_ReceiveMessage(void);
/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  TestMain_CAN_Init();
  while (1)
  {
    // TestMain_CAN_ReceiveMessage();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
TestMain_CAN_Init (void)
{

  BSP_canConfigDefault();
  BSP_canConfigMode(CAN_MODE);
  BSP_canConfigIO(TXD_PIN, RXD_PIN);
  BSP_canConfigQueue(TXD_QUEUE_LEN, RXD_QUEUE_LEN);
  // BSP_canConfigIntr(INTR_FLAG);
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
  BSP_canConfigInterrupt ();
  BSP_RegisterIsr(my_custom_isr);
}

static void IRAM_ATTR
my_custom_isr (void *arg)
{
  printf("High-layer ISR executed: Processing received CAN data...\n");
}

// There are functions, not tasks
static void
TestMain_CAN_SendMessage (void)
{
  DEV_CAN_SendMessage(
      CAN_ID, CAN_EXTD, CAN_RTR, (uint8_t *)p_text, strlen(p_text));
}

static void
TestMain_CAN_ReceiveMessage (void)
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