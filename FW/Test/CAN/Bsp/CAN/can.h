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
  } can_bit_rate_t;

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  esp_err_t BSP_canDriverInit(void);

  void BSP_canConfigDefault(void);

  void BSP_canConfigMode(twai_mode_t e_can_mode);

  void BSP_canConfigIO(gpio_num_t e_tx_pin, gpio_num_t e_rx_pin);

  void BSP_canConfigQueue(uint32_t u32_tx_queue_len, uint32_t u32_rx_queue_len);

  void BSP_canConfigIntr(int i_intr_flag);

  void BSP_canConfigBitRate(can_bit_rate_t e_bitrate);

  esp_err_t BSP_canStart(void);

  esp_err_t BSP_canStop(void);

  esp_err_t BSP_canTransmit(twai_message_t *p_message, TickType_t u32_timeout);

  esp_err_t BSP_canReceive(twai_message_t *p_message, TickType_t u32_timeout);

  void BSP_RegisterIsr(void (*isr_func)(void *));
  
  void BSP_canConfigInterrupt(void);

#ifdef __cplusplus
}
#endif

#endif