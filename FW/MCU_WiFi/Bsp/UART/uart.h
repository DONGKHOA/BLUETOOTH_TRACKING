#ifndef UART_H_
#define UART_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/uart.h"
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
    UART_PORT_NUM_0 = 0,
    UART_PORT_NUM_1 = 1,
    UART_PORT_NUM_2 = 2
  } uart_port_num_t;

  typedef enum
  {
    UART_BAUDRATE_4800   = 4800,
    UART_BAUDRATE_9600   = 9600,
    UART_BAUDRATE_19200  = 19200,
    UART_BAUDRATE_38400  = 38400,
    UART_BAUDRATE_57600  = 57600,
    UART_BAUDRATE_115200 = 115200,
    UART_BAUDRATE_230400 = 230400,
    UART_BAUDRATE_460800 = 460800,
    UART_BAUDRATE_921600 = 921600
  } uart_baudrate_t;

  /*****************************************************************************
   *      PUBLIC DEFINES
   *****************************************************************************/

#define RX_BUF_SIZE 1024

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/
  void BSP_uartDriverInit(uart_port_t           u32_uart_port,
                          gpio_num_t            e_tx_pin,
                          gpio_num_t            e_rx_pin,
                          uint32_t              u32_baudrate,
                          uart_word_length_t    e_data_bit,
                          uart_parity_t         e_parity_bit,
                          uart_hw_flowcontrol_t e_flow_control,
                          uart_stop_bits_t      e_stop_bit);

  // void BSP_uartDriverInit(uart_port_num_t e_uart_port);

  // void BSP_uartConfigIO(uart_port_num_t e_uart_port,
  //                       gpio_num_t      e_tx_pin,
  //                       gpio_num_t      e_rx_pin);

  // void BSP_uartConfigParity(uart_parity_t e_parity_bit);

  // void BSP_uartConfigDataLen(uart_word_length_t e_data_len);

  // void BSP_uartConfigStopBits(uart_stop_bits_t e_stop_bit);

  // void BSP_uartConfigBaudrate(uart_baudrate_t e_baudrate);

  int BSP_uartSendData(uart_port_num_t e_uart_port,
                       const uint8_t  *u8_data,
                       size_t          u32_len);

  int BSP_uartReadData(uart_port_num_t e_uart_port,
                       uint8_t        *u8_data,
                       size_t          u32_len,
                       uint32_t        u32_timeout);

  esp_err_t BSP_uartWaitTXDone(uart_port_num_t e_uart_port,
                               uint32_t        u32_timeout);

  esp_err_t BSP_resetBuffer(uart_port_num_t e_uart_port);

#ifdef __cplusplus
}
#endif

#endif