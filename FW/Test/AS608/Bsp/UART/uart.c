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