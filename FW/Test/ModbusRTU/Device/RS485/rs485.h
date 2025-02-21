#ifndef RS485_H
#define RS485_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "uart.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void DEV_RS485_SendData(uart_port_num_t e_uart_port,
                          gpio_num_t      e_modbus_re_io,
                          gpio_num_t      e_modbus_de_io,
                          const uint8_t  *u8_data,
                          size_t          u32_len);

  uint8_t DEV_RS485_ReceiveResponse(uart_port_num_t e_uart_port,
                                    uint8_t        *u8_data,
                                    size_t          u32_len,
                                    uint32_t        u32_timeout);

  void DEV_RS485_ReceiveMode(gpio_num_t e_modbus_re_io,
                             gpio_num_t e_modbus_de_io);

#ifdef __cplusplus
}
#endif

#endif /* RS485_H*/