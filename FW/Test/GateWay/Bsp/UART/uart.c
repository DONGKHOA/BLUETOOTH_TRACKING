/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "uart.h"

/*****************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
BSP_uartDriverInit (uart_port_t           u32_uart_port,
                    gpio_num_t            e_tx_pin,
                    gpio_num_t            e_rx_pin,
                    uint32_t              u32_baudrate,
                    uart_word_length_t    e_data_bit,
                    uart_parity_t         e_parity_bit,
                    uart_hw_flowcontrol_t e_flow_control,
                    uart_stop_bits_t      e_stop_bit)
{
  uart_config_t uart_config = {
    .baud_rate = u32_baudrate,
    .data_bits = e_data_bit,
    .parity    = e_parity_bit,
    .stop_bits = e_stop_bit,
    .flow_ctrl = e_flow_control,
  };

  // using buffer in rx data
  uart_driver_install(u32_uart_port, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
  uart_param_config(u32_uart_port, &uart_config);
  uart_set_pin(u32_uart_port,
               e_tx_pin,
               e_rx_pin,
               UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);
}

int
BSP_uartSendData (uart_port_t    u32_uart_port,
                  const uint8_t *u8_data,
                  size_t         u32_len)
{
  int bytes_sent
      = uart_write_bytes(u32_uart_port, (const char *)u8_data, u32_len);
  return bytes_sent;
}

int
BSP_uartReadData (uart_port_t u32_uart_port,
                  uint8_t    *u8_data,
                  size_t      u32_len,
                  uint32_t    u32_timeout)
{
  int bytes_receive
      = uart_read_bytes(u32_uart_port, u8_data, u32_len, u32_timeout);
  return bytes_receive;
}

esp_err_t
BSP_uartWaitTXDone (uart_port_t u32_uart_port, uint32_t u32_timeout)
{
  esp_err_t response
      = uart_wait_tx_done(u32_uart_port, u32_timeout);
  return response;
}

esp_err_t
BSP_resetBuffer (uart_port_t u32_uart_port)
{
  return uart_flush_input(u32_uart_port);
}
