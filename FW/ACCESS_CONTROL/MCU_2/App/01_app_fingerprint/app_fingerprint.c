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
  TimerHandle_t  s_timeout;
  QueueHandle_t *p_fingerprint_queue;
} fingerprint_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_FINGERPRINT_task(void *arg);
static void APP_FINGERPRINT_Timer_Callback(TimerHandle_t s_timer);
static inline void APP_FINGERPRINT_Timer_Callback

    /******************************************************************************
     *    PRIVATE DATA
     *****************************************************************************/

    static fingerprint_data_t s_fingerprint_data;

static uint8_t buffer1[1] = { 0x01 };
static uint8_t buffer2[1] = { 0x02 };

// Specify the start page for the search
static uint8_t u8_start_page[2] = { 0x00, 0x00 };
// Specify the page range for the search
static uint8_t u8_page_number[2] = { 0x00, 0xC8 };

static uint8_t u8_default_address[4]  = { 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t u8_default_password[4] = { 0x00, 0x00, 0x00, 0x00 };

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
  s_fingerprint_data.p_fingerprint_queue = &s_data_system.s_fingerprint_queue;
  buffer1                                = 0x01;
  // Specify the start page for the search

  u8_start_page[0] = 0x00;
  u8_start_page[1] = 0x00;

  // Specify the page range for the search

  u8_page_number[0] = 0x00;
  u8_page_number[1] = 0xC8;

  memset(u8_default_address, 0xFF, 4);
  memset(u8_default_password, 0x00, 4);

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
  uint8_t    confirmation_code;
  finger_cmd e_finger_cmd;

  while (1)
  {
    if (xQueueReceive(*s_fingerprint_data.p_fingerprint_queue,
                      &e_finger_cmd,
                      portMAX_DELAY)
        == pdTRUE)
    {
      switch (e_finger_cmd)
      {
        case FINGER_ENROLL:

          break;

        case FINGER_ATTENDANCE:

          break;

        case FINGER_DELETE:

          break;

        default:
          break;
      }
    }
  }
}

static void
APP_FINGERPRINT_Timer_Callback (TimerHandle_t s_timer)
{
  DEV_AS608_TimeOut();
}