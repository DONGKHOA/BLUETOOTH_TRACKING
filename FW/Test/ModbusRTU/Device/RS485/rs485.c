/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_timer.h"

#include "gpio.h"
#include "rs485.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
DEV_RS485_SendData (uart_port_num_t e_uart_port,
                    gpio_num_t      e_modbus_re_io,
                    gpio_num_t      e_modbus_de_io,
                    const uint8_t  *u8_data,
                    size_t          u32_len)
{
  BSP_gpioSetState(e_modbus_re_io, 1);
  BSP_gpioSetState(e_modbus_de_io, 1); // Transmit mode

  BSP_uartSendData(e_uart_port, u8_data, u32_len);
  BSP_uartWaitTXDone(e_uart_port, pdMS_TO_TICKS(100));

  BSP_gpioSetState(e_modbus_re_io, 0);
  BSP_gpioSetState(e_modbus_de_io, 0); // Receive mode
}

void
DEV_RS485_ReceiveMode (gpio_num_t e_modbus_re_io, gpio_num_t e_modbus_de_io)
{
  BSP_gpioSetState(e_modbus_re_io, 0);
  BSP_gpioSetState(e_modbus_de_io, 0); // Receive mode
}

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/