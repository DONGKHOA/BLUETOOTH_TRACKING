/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "ui.h"
#include "ui_events.h"

#include "app_data.h"

#include "environment.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern uint16_t user_id;
extern char     enroll_number_id[8];

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void EVENT_ENROLL_DeletePopup_Timer(lv_timer_t *timer);
static void EVENT_PROCESS_FINGER_ENROLL_DATA_Task(void *arg);

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t *p_send_data_queue;
  QueueHandle_t *p_receive_data_event_queue;
} finger_enroll_event_data_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static TaskHandle_t s_finger_enroll_task_handle;

static finger_enroll_event_data_t s_finger_enroll_event_data;
static DATA_SYNC_t                s_DATA_SYNC;

// static lv_timer_t *timer_finger_enroll;

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
EVENT_Enroll_Finger (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    b_is_initialize = true;
    s_finger_enroll_event_data.p_send_data_queue
        = &s_data_system.s_send_data_queue;
    s_finger_enroll_event_data.p_receive_data_event_queue
        = &s_data_system.s_receive_data_event_queue;

    xTaskCreate(EVENT_PROCESS_FINGER_ENROLL_DATA_Task,
                "process finger enroll task",
                1024 * 4,
                NULL,
                6,
                &s_finger_enroll_task_handle);
  }

  lv_label_set_text(ui_IDTextEnroll4, enroll_number_id);

  vTaskResume(s_finger_enroll_task_handle);

  s_DATA_SYNC.u8_data_start     = DATA_SYNC_ENROLL_FINGERPRINT;
  s_DATA_SYNC.u8_data_packet[0] = (user_id << 8) & 0xFF;
  s_DATA_SYNC.u8_data_packet[1] = user_id & 0xFF;
  s_DATA_SYNC.u8_data_length    = 2;
  s_DATA_SYNC.u8_data_stop      = DATA_STOP_FRAME;
  xQueueSend(*s_finger_enroll_event_data.p_send_data_queue, &s_DATA_SYNC, 0);
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/

static void
EVENT_PROCESS_FINGER_ENROLL_DATA_Task (void *arg)
{
  while (1)
  {
    if (xQueueReceive(*s_finger_enroll_event_data.p_receive_data_event_queue,
                      &s_DATA_SYNC,
                      portMAX_DELAY)
        == pdTRUE)
    {
      switch (s_DATA_SYNC.u8_data_start)
      {
        case DATA_SYNC_RESPONSE_ENROLL_FIRST_FINGERPRINT:
          // Handle enroll fingerprint response
          break;

        case DATA_SYNC_RESPONSE_ENROLL_SECOND_FINGERPRINT:
          // Handle enroll fingerprint response
          break;

        default:
          break;
      }
    }
  }
}
