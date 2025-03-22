#ifndef SN65HVD230DR_H_
#define SN65HVD230DR_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *      PUBLIC FUNCTIONS
   ***************************************************************************/

  /**
   * The function `DEV_CAN_SendMessage` sends a CAN message with specified
   * parameters and returns a success status.
   *
   * @param u32_id The `u32_id` parameter is the identifier of the message to be
   * sent over the CAN bus. It is a 32-bit unsigned integer representing the
   * message ID.
   * @param u32_extd The `u32_extd` parameter in the `DEV_CAN_SendMessage`
   * function is used to specify whether the message is an extended frame or a
   * standard frame. A value of 0 indicates a standard frame, while a non-zero
   * value indicates an extended frame.
   * @param u32_rtr The `u32_rtr` parameter in the `DEV_CAN_SendMessage`
   * function is used to specify whether the message is a remote transmission
   * request (RTR). A value of 0 indicates a data frame, while a value of 1
   * indicates an RTR frame.
   * @param p_data The `p_data` parameter in the `DEV_CAN_SendMessage` function
   * is a pointer to an array of `uint8_t` type, which represents the data to be
   * transmitted in the CAN message. The function uses `memcpy` to copy this
   * data into the `transmit_message.data` field
   * @param u8_len The `u8_len` parameter in the `DEV_CAN_SendMessage` function
   * represents the length of the data array `p_data` that you want to send in
   * the CAN message. It specifies the number of bytes to be copied from the
   * `p_data` array to the `transmit_message.data
   *
   * @return The function `DEV_CAN_SendMessage` returns a `uint8_t` value,
   * either 1 if the message was successfully sent or 0 if the message failed to
   * send.
   */
  uint8_t DEV_CAN_SendMessage(uint32_t u32_id,
                              uint32_t u32_extd,
                              uint32_t u32_rtr,
                              uint8_t *p_data,
                              uint8_t  u8_len);

#ifdef __cplusplus
}
#endif

#endif