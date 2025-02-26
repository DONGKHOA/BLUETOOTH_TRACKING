/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "uart.h"

/*****************************************************************************
 *      PRIVATE VARIABLES
 *****************************************************************************/

static uart_config_t uart_config = { 0 };

/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/

void
BSP_uartDriverInit (uart_port_num_t e_uart_port)
{
  // using buffer in rx data
  uart_driver_install(e_uart_port, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
  uart_param_config(e_uart_port, &uart_config);
}

void
BSP_uartConfigIO (uart_port_num_t e_uart_port,
                  gpio_num_t      e_tx_pin,
                  gpio_num_t      e_rx_pin)
{
  uart_set_pin(
      e_uart_port, e_tx_pin, e_rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void
BSP_uartConfigParity (uart_parity_t e_parity_bit)
{
  uart_config.parity = e_parity_bit;
}

void
BSP_uartConfigDataLen (uart_word_length_t e_data_len)
{
  uart_config.data_bits = e_data_len;
}

void
BSP_uartConfigStopBits (uart_stop_bits_t e_stop_bit)
{
  uart_config.stop_bits = e_stop_bit;
}

void
BSP_uartConfigBaudrate (uart_baudrate_t e_baudrate)
{
  uart_config.baud_rate = e_baudrate;
}

void
BSP_uartConfigHWFlowCTRL (uart_hw_flowcontrol_t e_flow_control)
{
  uart_config.flow_ctrl = e_flow_control;
}

int
BSP_uartSendData (uart_port_num_t e_uart_port,
                  const uint8_t  *u8_data,
                  size_t          u32_len)
{
  int bytes_sent
      = uart_write_bytes(e_uart_port, (const char *)u8_data, u32_len);
  return bytes_sent;
}

void
BSP_uartReadByte (uart_port_num_t e_uart_port,
                  uint8_t        *u8_data,
                  uint32_t        u32_timeout)
{

  return uart_read_bytes(e_uart_port, u8_data, 1, u32_timeout);
}

esp_err_t
BSP_uartWaitTXDone (uart_port_num_t e_uart_port, uint32_t u32_timeout)
{
  esp_err_t response = uart_wait_tx_done(e_uart_port, u32_timeout);
  return response;
}

esp_err_t
BSP_resetBuffer (uart_port_num_t e_uart_port)
{
  return uart_flush_input(e_uart_port);
}
