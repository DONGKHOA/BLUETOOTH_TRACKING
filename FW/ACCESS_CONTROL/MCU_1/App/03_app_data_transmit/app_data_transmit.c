/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "app_data_transmit.h"
#include "app_data.h"

#include "can.h"
#include "sn65hvd230dr.h"

#include "environment.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_DATA_TRANSMIT"

#define CAN_ID   0x123
#define CAN_EXTD 0
#define CAN_RTR  0

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

/**
 * @brief Structure defining the data of app_data_transmit.
 */
typedef struct data_transmit_data
{
  DATA_SYNC_t    s_data_trans;
  QueueHandle_t *p_send_data_queue;
} data_transmit_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_DATA_TRANSMIT_task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static data_transmit_data_t data_transmit_data;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_DATA_TRANSMIT_CreateTask (void)
{
  xTaskCreate(
      APP_DATA_TRANSMIT_task, "data transmit task", 1024 * 2, NULL, 13, NULL);
}
void
APP_DATA_TRANSMIT_Init (void)
{
  data_transmit_data.p_send_data_queue = s_data_system.s_send_data_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_DATA_TRANSMIT_task (void *arg)
{
  while (1)
  {
  }
}