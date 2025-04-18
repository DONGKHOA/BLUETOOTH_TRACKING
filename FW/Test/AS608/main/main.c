/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "as608.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define UART_TXD  GPIO_NUM_15
#define UART_RXD  GPIO_NUM_17
#define UART_NUM  UART_NUM_1
#define BAUD_RATE 115200

/******************************************************************************
 *  PRIVATE VARIABLE
 *****************************************************************************/

uint8_t buffer1[1] = { 0x01 };
uint8_t buffer2[1] = { 0x02 };

// Specify the start page for the search
uint8_t start_page[2] = { 0x00, 0x00 };
// Specify the page range for the search
uint8_t page_number[2] = { 0x00, 0xC8 };

uint8_t default_address[4]  = { 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t default_password[4] = { 0x00, 0x00, 0x00, 0x00 };

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void Timer_Callback(TimerHandle_t xTimer);
static void TestMain_FingerPrint_Detect_Task(void *pvParameters);
static void TestMain_Fingerprint_Register_Task(void *pvParameters);
static void TestMain_Finger_Delete_All_Task(void *pvParameters);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

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

  BSP_uartConfigIO(UART_NUM, UART_TXD, UART_RXD);
  BSP_uartConfigParity(UART_PARITY_DISABLE);
  BSP_uartConfigDataLen(UART_DATA_8_BITS);
  BSP_uartConfigStopBits(UART_STOP_BITS_1);
  BSP_uartConfigBaudrate(BAUD_RATE);
  BSP_uartConfigHWFlowCTRL(UART_HW_FLOWCTRL_DISABLE);

  BSP_uartDriverInit(UART_NUM);

  xTaskCreate(TestMain_Finger_Delete_All_Task,
              "TestMain_Finger_Delete_All_Task",
              4096,
              NULL,
              10,
              NULL);

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
Timer_Callback (TimerHandle_t xTimer)
{
  DEV_AS608_TimeOut();
}

static void
TestMain_FingerPrint_Detect_Task (void *pvParameters)
{
  uint8_t confirmation_code;

  while (1)
  {
    // Get fingerprint image with retry if error code is 0x02
    confirmation_code = DEV_AS608_GenImg(UART_NUM, default_address);
    if (confirmation_code == 0xFF)
    {
      printf("Error: Timeout\n");
      continue;
    }
    if (confirmation_code == 0x00)
    {
      confirmation_code
          = DEV_AS608_Img2Tz(UART_NUM, default_address, buffer1);
      if (confirmation_code == 0x00)
      {
        // Search in the database
        confirmation_code = DEV_AS608_Search(
            UART_NUM, default_address, buffer1, start_page, page_number);

        switch (confirmation_code)
        {
          case 0:
            printf("Fingerprint detected!\n");
            break;
          case 0x09:
            printf("Fingerprint not matched\n");
            break;
          default:
            printf("Fingerprint not detected!\n");
            break;
        }
      }
      else
      {
        printf("Fingerprint not detected!\n");
      }
    }
    else
    {
      printf("Retrying GenIMG...\n");
    }
  }
}

static void
TestMain_Fingerprint_Register_Task (void *pvParameters)
{
  uint16_t u16_stored_fingerprints = 0;
  uint16_t u16_user_id             = 0;
  uint8_t  confirmation_code       = 0;
  uint8_t  page_id[2]              = { 0 };

  while (1)
  {
    // Get the number of saved fingerprint templates
    DEV_AS608_TempleteNum(
        UART_NUM, default_address, &u16_stored_fingerprints);
    u16_user_id = u16_stored_fingerprints
                  + 1; // add new fingerprint = store templates + 1 (new user)
    page_id[0] = ((u16_user_id >> 8) & 0xFF);
    page_id[1] = (u16_user_id & 0xFF);
    printf("User ID: %d\n", u16_user_id);

    // Get the first fingerprint image
    confirmation_code = DEV_AS608_GenImg(UART_NUM, default_address);
    if (confirmation_code == 0xFF)
    {
      printf("Error: Timeout\n");
      continue;
    }
    if (confirmation_code == 0x00)
    {
      confirmation_code
          = DEV_AS608_Img2Tz(UART_NUM, default_address, buffer1);
      if (confirmation_code == 0x00)
      {
        printf("Remove your finger\n");
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Get the second fingerprint image
        confirmation_code = DEV_AS608_GenImg(UART_NUM, default_address);
        if (confirmation_code == 0x00)
        {
          if (confirmation_code == 0x00)
          {
            confirmation_code
                = DEV_AS608_Img2Tz(UART_NUM, default_address, buffer2);
            if (confirmation_code == 0x00)
            {
              // Merge fingerprint features confirmation_code
              confirmation_code
                  = DEV_AS608_RegModel(UART_NUM, default_address);
              if (confirmation_code == 0)
              {
                // Check that the fingerprint template exists in flash or not
                confirmation_code = DEV_AS608_Search(UART_NUM,
                                                     default_address,
                                                     buffer1,
                                                     start_page,
                                                     page_number);
                if (confirmation_code != 0)
                {
                  confirmation_code = DEV_AS608_Store(
                      UART_NUM, default_address, buffer1, page_id);
                  if (confirmation_code == 0)
                  {
                    printf("Enroll success! Stored template with ID: %d\n",
                           u16_user_id);
                  }
                  else
                  {
                    printf("Error: Cannot store template | %d\n",
                           confirmation_code);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

static void
TestMain_Finger_Delete_All_Task (void *pvParameters)
{
  uint8_t confirmation_code = DEV_AS608_DeleteChar(
      UART_NUM, default_address, start_page, page_number);
  if (confirmation_code != 0)
  {
    printf("Failed to delete all templates");
  }
  printf("Delete all Fingerprint templates");

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
