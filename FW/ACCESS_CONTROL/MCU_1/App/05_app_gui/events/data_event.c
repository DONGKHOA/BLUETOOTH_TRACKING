/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "ui.h"
#include "ui_events.h"

#include "app_data.h"

 #include "environment.h"

#include <stdbool.h>
#include <stdio.h>

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_Data_List_Timer(lv_timer_t *timer);

/******************************************************************************
 *    PUBLIC DATA
 *****************************************************************************/

uint16_t user_id = 0;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static QueueHandle_t *p_send_data_queue;

static DATA_SYNC_t s_data_sync;

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
EVENT_Data_List (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    p_send_data_queue = &s_data_system.s_send_data_queue;
    b_is_initialize   = true;
  }

  s_data_sync.u8_data_start     = DATA_SYNC_REQUEST_USER_DATA;
  s_data_sync.u8_data_packet[0] = DATA_SYNC_DUMMY;
  s_data_sync.u8_data_length    = 1;
  s_data_sync.u8_data_stop      = DATA_STOP_FRAME;
  // Notify the state of connection to transmit task via queue
  xQueueSend(*p_send_data_queue, &s_data_sync, 0);
}