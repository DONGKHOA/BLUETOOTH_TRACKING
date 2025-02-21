/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

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

uint8_t
DEV_CAN_ReceiveMessage (uint32_t *p_id,
                        uint32_t *p_extd,
                        uint32_t *p_rtr,
                        uint8_t  *p_data,
                        uint8_t  *p_len)
{
  twai_message_t receive_message;

  if (BSP_canReceive(&receive_message, pdMS_TO_TICKS(10000)) == ESP_OK)
  {
    printf("Message received\n");
  }
  else
  {
    printf("Failed to receive message\n");
    return 0;
  }

  if (receive_message.extd)
  {
    printf("Message is in Extended Format\n");
  }
  else
  {
    printf("Message is in Standard Format\n");
  }

  printf("ID is %lX\n", receive_message.identifier);
  if (!(receive_message.rtr))
  {
    for (int i = 0; i < receive_message.data_length_code; i++)
    {
      printf("Data byte %d = %X\n", i, receive_message.data[i]);
    }
  }

  // Store data
  *p_id   = receive_message.identifier;
  *p_extd = receive_message.extd;
  *p_rtr  = receive_message.rtr;
  *p_len  = receive_message.data_length_code;

  if (!(receive_message.rtr)) //Not RemoteFrame
  {
    memcpy (p_data, receive_message.data, receive_message.data_length_code);
  }
  return 1;
}