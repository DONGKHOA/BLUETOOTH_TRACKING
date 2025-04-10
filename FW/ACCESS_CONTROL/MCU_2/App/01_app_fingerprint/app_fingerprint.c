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
  TimerHandle_t       s_timeout;
  QueueHandle_t      *p_send_data_queue;
  QueueHandle_t      *p_fingerprint_queue;
  QueueHandle_t      *p_data_local_database_queue;
  EventGroupHandle_t *p_fingerprint_event;
} fingerprint_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void        APP_FINGERPRINT_task(void *arg);
static void        APP_FINGERPRINT_Timer_Callback(TimerHandle_t s_timer);
static inline void APP_FINGERPRINT_Enroll(void);
static inline void APP_FINGERPRINT_Attendance(void);
static inline void APP_FINGERPRINT_Delete(void);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

DATA_SYNC_t s_DATA_SYNC;

static uint8_t u8_finger_count = 0;

uint16_t u16_finger_user_id;

uint8_t u8_page_id[2] = { 0 };

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
  s_fingerprint_data.p_send_data_queue   = &s_data_system.s_send_data_queue;
  s_fingerprint_data.p_fingerprint_event = &s_data_system.s_fingerprint_event;
  s_fingerprint_data.p_data_local_database_queue
      = &s_data_system.s_data_local_database_queue;
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
  uint8_t  uxBits;
  uint8_t  u8_enroll_confirmation_code;
  uint16_t index;

  DATA_SYNC_t s_DATA_SYNC;

  while (1)
  {
    uxBits = xEventGroupWaitBits(*s_fingerprint_data.p_fingerprint_event,
                                 EVENT_ENROLL_FINGERPRINT,
                                 pdFALSE,
                                 pdFALSE,
                                 portMAX_DELAY);

    if (uxBits & EVENT_ENROLL_FINGERPRINT)
    {
      // APP_FINGERPRINT_Enroll();

      if (u8_finger_count == 0)
      {
        u8_enroll_confirmation_code
            = DEV_AS608_GenImg(UART_FINGERPRINT_NUM, u8_default_address);
        if (u8_enroll_confirmation_code != 0)
        {
          continue;
        }

        u8_enroll_confirmation_code = DEV_AS608_Img2Tz(
            UART_FINGERPRINT_NUM, u8_default_address, buffer1);
        if (u8_enroll_confirmation_code != 0)
        {
          continue;
        }

        u8_finger_count++;

        // Send MCU1 to notify that the fingerprint have store to buffer 1
        s_DATA_SYNC.u8_data_start = DATA_SYNC_RESPONSE_ENROLL_FOUND_FINGERPRINT;
        s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_SUCCESS;
        s_DATA_SYNC.u8_data_length    = 1;
        s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
        xQueueSend(*s_fingerprint_data.p_send_data_queue, &s_DATA_SYNC, 0);
      }
      else if (u8_finger_count == 1)
      {
        u8_enroll_confirmation_code
            = DEV_AS608_GenImg(UART_FINGERPRINT_NUM, u8_default_address);
        if (u8_enroll_confirmation_code != 0)
        {
          continue;
        }

        u8_enroll_confirmation_code = DEV_AS608_Img2Tz(
            UART_FINGERPRINT_NUM, u8_default_address, buffer2);
        if (u8_enroll_confirmation_code != 0)
        {
          continue;
        }

        u8_enroll_confirmation_code
            = DEV_AS608_RegModel(UART_FINGERPRINT_NUM, u8_default_address);
        if (u8_enroll_confirmation_code != 0)
        {
          u8_finger_count = 0;
          xEventGroupClearBits(*s_fingerprint_data.p_fingerprint_event,
                               EVENT_ENROLL_FINGERPRINT);

          continue;
        }

        index = 0;
        while (u16_finger_user_id != user_id[index])
        {

          if (index >= user_len)
          {
            break;
          }

          index++;
        }

        u8_page_id[0] = ((index >> 8) & 0xFF);
        u8_page_id[1] = (index & 0xFF);

        // Process to store finger
        u8_enroll_confirmation_code = DEV_AS608_Store(
            UART_FINGERPRINT_NUM, u8_default_address, buffer1, u8_page_id);
        if (u8_enroll_confirmation_code == 0)
        {
          // Send data to local database to update data in sdcard
          printf("Enroll success! Stored template with ID: %d\r\n",
                 u16_finger_user_id);

          s_DATA_SYNC.u8_data_start
              = LOCAL_DATABASE_RESPONSE_ENROLL_FINGERPRINT;
          s_DATA_SYNC.u8_data_packet[0] = (u16_finger_user_id << 8) & 0xFF;
          s_DATA_SYNC.u8_data_packet[1] = u16_finger_user_id & 0xFF;
          s_DATA_SYNC.u8_data_length    = 2;
          s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;

          xQueueSend(
              *s_fingerprint_data.p_data_local_database_queue, &s_DATA_SYNC, 0);

          u8_finger_count = 0;
          xEventGroupClearBits(*s_fingerprint_data.p_fingerprint_event,
                               EVENT_ENROLL_FINGERPRINT);
        }
        else
        {
          // Send fail to MCU1
          printf("Error: Cannot store template | %d\r\n",
                 u8_enroll_confirmation_code);
          s_DATA_SYNC.u8_data_start     = DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT;
          s_DATA_SYNC.u8_data_packet[0] = DATA_SYNC_FAIL;
          s_DATA_SYNC.u8_data_length    = 1;
          s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
          xQueueSend(*s_fingerprint_data.p_send_data_queue, &s_DATA_SYNC, 0);

          u8_finger_count = 0;
          xEventGroupClearBits(*s_fingerprint_data.p_fingerprint_event,
                               EVENT_ENROLL_FINGERPRINT);
        }
      }
    }
  }
  vTaskDelay(pdMS_TO_TICKS(10));
}

static inline void
APP_FINGERPRINT_Enroll (void)
{
}

static inline void
APP_FINGERPRINT_Attendance (void)
{
}

static inline void
APP_FINGERPRINT_Delete (void)
{
}

static void
APP_FINGERPRINT_Timer_Callback (TimerHandle_t s_timer)
{
  DEV_AS608_TimeOut();
}