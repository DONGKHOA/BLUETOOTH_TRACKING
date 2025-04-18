#ifndef RS485_H
#define RS485_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "uart.h"
#include "gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void DEV_RS485_SendData(uart_port_num_t e_uart_port,
                          const uint8_t  *u8_data,
                          size_t          u32_len);

  static inline int DEV_RS485_ReceiveByte (uart_port_num_t e_uart_port,
                                           uint8_t        *u8_data,
                                           uint32_t        u32_timeout)
  {
    return BSP_uartReadByte(e_uart_port, u8_data, u32_timeout);
  }

  static inline void DEV_RS485_ReceiveMode (gpio_num_t e_modbus_re_io,
                                            gpio_num_t e_modbus_de_io)
  {
    BSP_gpioSetState(e_modbus_re_io, 0);
    BSP_gpioSetState(e_modbus_de_io, 0); // Receive mode
  }

  static inline void DEV_RS485_TransmitMode (gpio_num_t e_modbus_re_io,
                                             gpio_num_t e_modbus_de_io)
  {
    BSP_gpioSetState(e_modbus_re_io, 1);
    BSP_gpioSetState(e_modbus_de_io, 1); // Transmit mode
  }

#ifdef __cplusplus
}
#endif

#endif /* RS485_H*/