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
  /**
   * @brief Initialize the UART driver for the specified port.
   *
   * @param e_uart_port The UART port number to initialize.
   */
  void BSP_uartDriverInit(uart_port_num_t e_uart_port);

  /**
   * @brief Configure the UART IO pins for the specified port.
   *
   * @param e_uart_port The UART port number to configure.
   * @param e_tx_pin The GPIO pin number for TX.
   * @param e_rx_pin The GPIO pin number for RX.
   */
  void BSP_uartConfigIO(uart_port_num_t e_uart_port,
                        gpio_num_t      e_tx_pin,
                        gpio_num_t      e_rx_pin);

  /**
   * @brief Configure the UART parity bit.
   *
   * @param e_parity_bit The parity bit configuration.
   */
  void BSP_uartConfigParity(uart_parity_t e_parity_bit);

  /**
   * @brief Configure the UART data length.
   *
   * @param e_data_len The data length configuration.
   */
  void BSP_uartConfigDataLen(uart_word_length_t e_data_len);

  /**
   * @brief Configure the UART stop bits.
   *
   * @param e_stop_bit The stop bit configuration.
   */
  void BSP_uartConfigStopBits(uart_stop_bits_t e_stop_bit);

  /**
   * @brief Configure the UART baud rate.
   *
   * @param e_baudrate The baud rate configuration.
   */
  void BSP_uartConfigBaudrate(uart_baudrate_t e_baudrate);

  /**
   * @brief Configure the UART hardware flow control.
   *
   * @param e_flow_control The hardware flow control configuration.
   */
  void BSP_uartConfigHWFlowCTRL(uart_hw_flowcontrol_t e_flow_control);

  /**
   * @brief Send data over the specified UART port.
   *
   * @param e_uart_port The UART port number to send data.
   * @param u8_data Pointer to the data buffer to send.
   * @param u32_len Length of the data to send.
   * @return int The number of bytes sent, or a negative error code.
   */
  static inline int BSP_uartSendData (uart_port_num_t e_uart_port,
                                      const uint8_t  *u8_data,
                                      size_t          u32_len)
  {
    return uart_write_bytes(e_uart_port, (const char *)u8_data, u32_len);
  }

  /**
   * @brief Read data from the specified UART port.
   *
   * @param e_uart_port The UART port number to read data.
   * @param u8_data Pointer to the buffer to store received data.
   * @param u32_timeout Timeout for the read operation in milliseconds.
   * @return int The number of bytes read, or a negative error code.
   */
  static inline int BSP_uartReadByte (uart_port_num_t e_uart_port,
                                      uint8_t        *u8_data,
                                      uint32_t        u32_timeout)
  {
    return uart_read_bytes(e_uart_port, u8_data, 1, u32_timeout);
  }

  /**
   * @brief Wait until the UART transmission is done.
   *
   * @param e_uart_port The UART port number to wait for.
   * @param u32_timeout Timeout for the wait operation in milliseconds.
   * @return esp_err_t ESP_OK on success, or an error code on failure.
   */
  static inline esp_err_t BSP_uartWaitTXDone (uart_port_num_t e_uart_port,
                                              uint32_t        u32_timeout)
  {
    return uart_wait_tx_done(e_uart_port, u32_timeout);
  }

  /**
   * @brief Reset the UART buffer for the specified port.
   *
   * @param e_uart_port The UART port number to reset the buffer.
   * @return esp_err_t ESP_OK on success, or an error code on failure.
   */
  static inline esp_err_t BSP_resetBuffer (uart_port_num_t e_uart_port)
  {
    return uart_flush_input(e_uart_port);
  }

#ifdef __cplusplus
}
#endif

#endif