#ifndef SN65HVD230DR_H_
#define SN65HVD230DR_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "can.h"

/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/
uint8_t DEV_CAN_SendMessage(uint32_t u32_id,
                            uint32_t u32_extd,
                            uint32_t u32_rtr,
                            uint8_t *p_data,
                            uint8_t  u8_len);

uint8_t DEV_CAN_ReceiveMessage(uint32_t *p_id,
                               uint32_t *p_extd,
                               uint32_t *p_rtr,
                               uint8_t  *p_data,
                               uint8_t  *p_len);

#endif