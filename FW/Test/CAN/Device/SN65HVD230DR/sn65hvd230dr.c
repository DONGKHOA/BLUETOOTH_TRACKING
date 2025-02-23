/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "sdkconfig.h"

#include "sn65hvd230dr.h"

/*****************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

uint8_t
DEV_CAN_SendMessage (uint32_t u32_id,
                     uint32_t u32_extd,
                     uint32_t u32_rtr,
                     uint8_t *p_data,
                     uint8_t  u8_len)
{
  twai_message_t transmit_message;
  transmit_message.identifier       = u32_id;
  transmit_message.extd             = u32_extd;
  transmit_message.rtr              = u32_rtr;
  transmit_message.data_length_code = u8_len;

  memcpy(transmit_message.data, p_data, u8_len);

  if (BSP_canTransmit(&transmit_message, pdMS_TO_TICKS(1000)) == ESP_OK)
  {
    printf("Message sent\n");
    return 1;
  }
  else
  {
    printf("Failed to send message\n");
    return 0;
  }
}

/*****************************************************************************
 *      PRIVATE FUNCTION
 *****************************************************************************/