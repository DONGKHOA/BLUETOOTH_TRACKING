/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "app_fingerprint.h"
#include "app_data.h"

#include "as608.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_FINGERPRINT"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct fingerprint_data
{
  TimerHandle_t s_timeout;
  uint8_t       buffer1;
  uint8_t       u8_default_address[4];
  uint8_t       u8_default_password[4];
  uint8_t       u8_start_page[2];
  uint8_t       u8_page_number[2];
}fingerprint_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_FINGERPRINT_task(void *arg);
static void APP_FINGERPRINT_Timer_Callback(TimerHandle_t s_timer);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static fingerprint_data_t s_fingerprint_data;

// static const uint8_t buffer1 = 0x01;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_FINGERPRINT_CreateTask (void)
{
  xTaskCreate(
      APP_FINGERPRINT_task, "fingerprint task", 1024 * 4, NULL, 8, NULL);
}

void
APP_FINGERPRINT_Init (void)
{
  static int b;
  const int a = 0;
  a = 1;
  s_fingerprint_data.buffer1 = 0x01;
  // Specify the start page for the search

  s_fingerprint_data.u8_start_page[0] = 0x00;
  s_fingerprint_data.u8_start_page[1] = 0x00;

  // Specify the page range for the search

  s_fingerprint_data.u8_page_number[0] = 0x00;
  s_fingerprint_data.u8_page_number[1] = 0xC8;

  memset(s_fingerprint_data.u8_default_address, 0xFF, 4);
  memset(s_fingerprint_data.u8_default_password, 0x00, 4);

  s_fingerprint_data.s_timeout
      = xTimerCreate("Timeout",
                     pdMS_TO_TICKS(1), // Period 1ms
                     pdTRUE,           // Auto reload
                     NULL,
                     APP_FINGERPRINT_Timer_Callback // Callback function
      );

  xTimerStart(s_fingerprint_data.s_timeout, 0);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_FINGERPRINT_task (void *arg)
{
  uint8_t confirmation_code;

  while (1)
  {
    // Get fingerprint image with retry if error code is 0x02
    confirmation_code
        = DEV_AS608_GenImg(UART_NUM, s_fingerprint_data.u8_default_address);
    if (confirmation_code == 0xFF)
    {
      printf("Error: Timeout\n");
      continue;
    }
    if (confirmation_code == 0x00)
    {
      confirmation_code
          = DEV_AS608_Img2Tz(UART_NUM,
                             s_fingerprint_data.u8_default_address,
                             (uint8_t *)&s_fingerprint_data.buffer1);
      if (confirmation_code == 0x00)
      {
        // Search in the database
        confirmation_code
            = DEV_AS608_Search(UART_NUM,
                               s_fingerprint_data.u8_default_address,
                               (uint8_t *)&s_fingerprint_data.buffer1,
                               s_fingerprint_data.u8_start_page,
                               s_fingerprint_data.u8_page_number);

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
APP_FINGERPRINT_Timer_Callback (TimerHandle_t s_timer)
{
  DEV_AS608_TimeOut();
}