#ifndef UART_H_
#define UART_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/uart.h"
#include "driver/gpio.h"

/*****************************************************************************
 *      DEFINES
 *****************************************************************************/

#define RX_BUF_SIZE 1024

/*****************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

/**
 * The function initializes a UART driver with specified configurations such as
 * baud rate, data bits, parity, flow control, and stop bits.
 *
 * @param u32_uart_port The `u32_uart_port` parameter specifies which UART port
 * to initialize. It could be something like `UART_NUM_0` or `UART_NUM_1`
 * depending on the specific UART hardware you are working with.
 * @param e_tx_pin The `e_tx_pin` parameter in the `uartDriverInit` function
 * refers to the GPIO pin number that is used for transmitting data in UART
 * communication. This pin is connected to the transmit (TX) line of the UART
 * interface and is responsible for sending data out from the microcontroller to
 * another device.
 * @param e_rx_pin The `e_rx_pin` parameter in the `uartDriverInit` function is
 * the GPIO pin number to which the receive (RX) pin of the UART interface is
 * connected. This pin is used for receiving data from an external device or
 * another UART transmitter.
 * @param u32_baudrate Baudrate refers to the number of symbols or signal
 * changes transmitted per second in a communication channel. It is typically
 * measured in bits per second (bps) or baud. The baudrate parameter in the
 * function `uartDriverInit` specifies the desired transmission speed for the
 * UART communication.
 * @param e_data_bit The `e_data_bit` parameter in the `uartDriverInit` function
 * specifies the number of data bits per frame. It determines the size of the
 * data being transmitted or received over the UART communication. The options
 * for `data_bit` typically include values like `UART_DATA_5_BITS`, `UART_DATA
 * @param e_parity_bit The `e_parity_bit` parameter in the `uartDriverInit`
 * function specifies the type of parity used for error-checking in UART
 * communication. It can be one of the following values:
 * @param e_flow_control Flow control is a mechanism used in communication
 * protocols to manage the flow of data between two devices. It can be either
 * hardware flow control or software flow control.
 * @param e_stop_bit The `e_stop_bit` parameter in the `uartDriverInit` function
 * refers to the number of stop bits used in UART communication. It specifies
 * the number of bits that signal the end of a data frame. Common values for
 * `stop_bit` include `UART_STOP_BITS_1` for one stop bit
 */
void BSP_uartDriverInit(uart_port_t           u32_uart_port,
                        gpio_num_t            e_tx_pin,
                        gpio_num_t            e_rx_pin,
                        uint32_t              u32_baudrate,
                        uart_word_length_t    e_data_bit,
                        uart_parity_t         e_parity_bit,
                        uart_hw_flowcontrol_t e_flow_control,
                        uart_stop_bits_t      e_stop_bit);

/**
 * The function `BSP_uartSendData` sends data over a UART port by writing the
 * specified data bytes.
 *
 * @param u32_uart_port The `uart_port` parameter is the UART port or interface
 * through which the data will be sent. It is typically a hardware-specific
 * identifier that specifies which UART peripheral to use for communication.
 * @param u8_data The `data` parameter in the `BSP_uartSendData` function is a
 * pointer to a character array (string) that contains the data to be sent via
 * UART (Universal Asynchronous Receiver-Transmitter) communication.
 * @param u32_len The `len` parameter in the `BSP_uartSendData` function
 * specifies
 */
int BSP_uartSendData(uart_port_t    u32_uart_port,
                     const uint8_t *u8_data,
                     size_t         u32_len);

/**
 * @brief Reads data from the specified UART port.
 *
 * This function reads a specified number of bytes from the given UART port
 * and stores them in the provided buffer. The function will wait for the
 * specified timeout duration for the data to be available.
 *
 * @param u32_uart_port The UART port to read from.
 * @param u8_data Pointer to the buffer where the read data will be stored.
 * @param u32_len The number of bytes to read.
 * @param u32_timeout The timeout duration in milliseconds to wait for data.
 *
 * @return The number of bytes actually read, or a negative value if an error
 * occurred.
 */
int BSP_uartReadData(uart_port_t u32_uart_port,
                     uint8_t    *u8_data,
                     size_t      u32_len,
                     uint32_t    u32_timeout);

/**
 * @brief Waits for the UART transmission to complete.
 *
 * This function waits until the UART transmission is finished or the specified
 * timeout period elapses.
 *
 * @param u32_uart_port The UART port number to wait for the transmission to
 * complete.
 * @param u32_timeout The maximum time to wait for the transmission to complete,
 * in milliseconds.
 *
 * @return
 *     - ESP_OK: Transmission completed successfully.
 *     - ESP_ERR_TIMEOUT: The operation timed out before the transmission
 * completed.
 *     - ESP_FAIL: Other errors.
 */
esp_err_t BSP_uartWaitTXDone(uart_port_t u32_uart_port, uint32_t u32_timeout);

/**
 * @brief Resets the UART buffer for the specified UART port.
 *
 * This function flushes the input buffer of the given UART port, effectively
 * resetting the buffer and discarding any data that has been received but not
 * yet read.
 *
 * @param u32_uart_port The UART port to reset the buffer for.
 *                      This parameter should be of type uart_port_t.
 *
 * @return
 *     - ESP_OK: Success
 *     - ESP_FAIL: Error flushing the input buffer
 */
esp_err_t BSP_resetBuffer(uart_port_t u32_uart_port);
#endif