/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "app_data_transmit.h"
#include "app_data.h"

#include "can.h"
#include "sn65hvd230dr.h"

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
typedef struct ble_ibeacon_data
{
  QueueHandle_t *p_location_tag_queue;
} data_transmit_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_DATA_TRANSMIT_task(void *arg);
static void APP_DATA_TRANSMIT_ConvertDataType(
    location_infor_tag_t *p_location_tag, uint8_t *p_data);

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
  data_transmit_data.p_location_tag_queue = &s_data_system.s_location_tag_queue;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_DATA_TRANSMIT_task (void *arg)
{
  location_infor_tag_t s_location_tag;
  uint8_t              u8_location_infor_tag[sizeof(location_infor_tag_t)];
  while (1)
  {
    if (xQueueReceive(*data_transmit_data.p_location_tag_queue,
                      &s_location_tag,
                      (TickType_t)10)
        == pdPASS)
    {
      APP_DATA_TRANSMIT_ConvertDataType(&s_location_tag, u8_location_infor_tag);
      DEV_CAN_SendMessage(CAN_ID,
                          CAN_EXTD,
                          CAN_RTR,
                          (uint8_t *)u8_location_infor_tag,
                          sizeof(u8_location_infor_tag));
    }
  }
}

static void
APP_DATA_TRANSMIT_ConvertDataType (location_infor_tag_t *p_location_tag,
                                   uint8_t              *p_data)
{
  memcpy(p_data, p_location_tag, sizeof(location_infor_tag_t));
}