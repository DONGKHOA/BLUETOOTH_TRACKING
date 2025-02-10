/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "sn65hvd230dr.h"

/*****************************************************************************
 *      PRIVATE DEFINES
 *****************************************************************************/

#define CAN_MODE      TWAI_MODE_NORMAL
#define TXD_PIN       GPIO_NUM_19
#define RXD_PIN       GPIO_NUM_20
#define TXD_QUEUE_LEN 5
#define RXD_QUEUE_LEN 5
#define INTR_FLAG     ESP_INTR_FLAG_LEVEL1 // lowest priority
#define BITRATE       5

/*****************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/
void
DEV_CAN_Init (void)
{
  BSP_canDriverInit(CAN_MODE,
                    TXD_PIN,
                    RXD_PIN,
                    TXD_QUEUE_LEN,
                    RXD_QUEUE_LEN,
                    INTR_FLAG,
                    BITRATE);
}

uint8_t
DEV_CAN_Trasnmit (uint32_t u32_id,
                  uint32_t u32_extd,
                  uint32_t u32_rtr,
                  uin32_t u32_data,
                  uint8_t  u8_len)
{
  twai_message_t message;
  message.identifier       = u32_id;
  message.extd             = u32_extd;
  message.rtr              = u32_rtr;
  message.data_length_code = u8_len;
  
}