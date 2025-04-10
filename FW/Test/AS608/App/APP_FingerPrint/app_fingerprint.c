/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_fingerprint.h"

/******************************************************************************
 *  PRIVATE VARIABLE
 *****************************************************************************/
#define UART_NUM  UART_NUM_2
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
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_FingerPrint_Detect_Task(void *pvParameters);
static void APP_Fingerprint_Register_Task(void *pvParameters);
static void APP_Finger_Delete_All_Task(void *pvParameters);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_FingerPrint_CreateTask (void)
{
  xTaskCreate(APP_FingerPrint_Detect_Task,
              "APP_FingerPrint_Detect_Task",
              4096,
              NULL,
              10,
              NULL);
}

void
APP_FingerPrint_Init (void)
{
  // BSP_uartDriverInit(UART_NUM,
  //                    UART_TXD,
  //                    UART_RXD,
  //                    BAUD_RATE,
  //                    UART_DATA_8_BITS,
  //                    UART_PARITY_DISABLE,
  //                    UART_HW_FLOWCTRL_DISABLE,
  //                    UART_STOP_BITS_1);

  BSP_uartConfigIO(UART_NUM, UART_TXD, UART_RXD);
  BSP_uartConfigParity(UART_PARITY_DISABLE);
  BSP_uartConfigDataLen(UART_DATA_8_BITS);
  BSP_uartConfigStopBits(UART_STOP_BITS_1);
  BSP_uartConfigBaudrate(BAUD_RATE);
  BSP_uartConfigHWFlowCTRL(UART_HW_FLOWCTRL_DISABLE);

  BSP_uartDriverInit(UART_NUM);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_FingerPrint_Detect_Task (void *pvParameters)
{
  uint8_t u8_confirmation_code;

  while (1)
  {
    while (DEV_AS608_GenImg(UART_PORT_NUM_2, default_address) != 0)
    {
      printf("Fingerprint not detected!\n");
    }

    u8_confirmation_code
        = DEV_AS608_Img2Tz(UART_PORT_NUM_2, default_address, buffer1);
    if (u8_confirmation_code != 0)
    {
      continue;
    }

    u8_confirmation_code = DEV_AS608_Search(
        UART_PORT_NUM_2, default_address, buffer1, start_page, page_number);
    if (u8_confirmation_code == 0)
    {
      printf("Fingerprint detected!\n");
    }
    else
    {
      printf("Fingerprint not detected!\n");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
static void
APP_Fingerprint_Register_Task (void *pvParameters)
{
  uint16_t u16_stored_fingerprints = 0;
  uint8_t  u8_user_id              = 0;
  uint8_t  u8_confirmation_code    = 0;
  uint8_t  u8_page_id[2]           = { 0 };

  while (1)
  {
    DEV_AS608_TempleteNum(
        UART_PORT_NUM_2, default_address, &u16_stored_fingerprints);
    u8_user_id = u16_stored_fingerprints + 1;

    u8_page_id[0] = ((u8_user_id >> 8) & 0xFF);
    u8_page_id[1] = (u8_user_id & 0xFF);
    printf("User ID: %d\n", u8_user_id);

    while (DEV_AS608_GenImg(UART_PORT_NUM_2, default_address) != 0)
    {
      printf("Fingerprint not detected!\n");
    }

    DEV_AS608_Img2Tz(UART_PORT_NUM_2, default_address, buffer1);

    printf("Please remove your finger\n");
    vTaskDelay(pdMS_TO_TICKS(3000));

    while (DEV_AS608_GenImg(UART_PORT_NUM_2, default_address) != 0)
    {
      printf("Fingerprint not detected!\n");
    }

    DEV_AS608_Img2Tz(UART_PORT_NUM_2, default_address, buffer2);

    u8_confirmation_code = DEV_AS608_RegModel(UART_PORT_NUM_2, default_address);
    if (u8_confirmation_code == 0)
    {
      u8_confirmation_code = DEV_AS608_Search(
          UART_PORT_NUM_2, default_address, buffer1, start_page, page_number);
      if (u8_confirmation_code == 0)
      {
        printf("Fingerprint detected!\n");
      }
      else
      {
        printf("Fingerprint not detected!\n");
        u8_confirmation_code = DEV_AS608_Store(
            UART_PORT_NUM_2, default_address, buffer1, u8_page_id);
        if (u8_confirmation_code == 0)
        {
          printf("Enroll success! Stored template with ID: %d", u8_user_id);
        }
        else
        {
          printf("Error: Cannot store template | %d", u8_confirmation_code);
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}
static void
APP_Finger_Delete_All_Task (void *pvParameters)
{
  uint8_t u8_confirmation_code = DEV_AS608_DeleteChar(
      UART_PORT_NUM_2, default_address, start_page, page_number);
  if (u8_confirmation_code != 0)
  {
    printf("Failed to delete all templates");
  }
  printf("all Fingerprint templates");

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}