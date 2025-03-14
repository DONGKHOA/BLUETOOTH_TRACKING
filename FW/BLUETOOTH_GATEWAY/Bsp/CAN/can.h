#ifndef CAN_H_
#define CAN_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/twai.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************
   *      PUBLIC TYPEDEFS
   *****************************************************************************/

  typedef enum
  {
    CAN_25KBITS,  // 0
    CAN_50KBITS,  // 1
    CAN_100KBITS, // 2
    CAN_125KBITS, // 3
    CAN_250KBITS, // 4
    CAN_500KBITS, // 5
    CAN_800KBITS, // 6
    CAN_1MBITS    // 7
  } can_config_bit_rate_t;

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  void BSP_canDriverInit(twai_mode_t           e_can_mode,
                         gpio_num_t            e_tx_pin,
                         gpio_num_t            e_rx_pin,
                         uint32_t              u32_tx_queue_len,
                         uint32_t              u32_rx_queue_len,
                         int                   i_intr_flag,
                         can_config_bit_rate_t e_bitrate);

  esp_err_t BSP_canStart(void);

  esp_err_t BSP_canStop(void);

  esp_err_t BSP_canTransmit(twai_message_t *p_message, TickType_t u32_timeout);

  esp_err_t BSP_canReceive(twai_message_t *p_message, TickType_t u32_timeout);

#ifdef __cplusplus
}
#endif

#endif