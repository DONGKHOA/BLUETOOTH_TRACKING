/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_timer.h"

#include "gpio.h"
#include "rs485.h"
#include "crc16.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define RS485_UART_NUM UART_NUM_2
#define RS485_TXD_PIN  GPIO_NUM_15
#define RS485_RXD_PIN  GPIO_NUM_16
#define RS485_RE_PIN   GPIO_NUM_39
#define RS485_DE_PIN   GPIO_NUM_38

#define RS485_BAUDRATE    9600
#define RS485_DATA_BIT    UART_DATA_8_BITS
#define RS485_PARITY_BIT  UART_PARITY_EVEN
#define RS485_HW_FLOWCTRL UART_HW_FLOWCTRL_DISABLE
#define RS485_STOP_BITS   UART_STOP_BITS_1

#define RS485_BUF_SIZE 256

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
DEV_RS485_Init (void)
{
  BSP_uartDriverInit(RS485_UART_NUM,
                     RS485_TXD_PIN,
                     RS485_RXD_PIN,
                     RS485_BAUDRATE,
                     RS485_DATA_BIT,
                     RS485_PARITY_BIT,
                     RS485_HW_FLOWCTRL,
                     RS485_STOP_BITS);

  // Default: receive mode
  BSP_gpioSetDirection(RS485_RE_PIN, GPIO_MODE_OUTPUT, GPIO_PULL_DOWN);
  BSP_gpioSetDirection(RS485_DE_PIN, GPIO_MODE_OUTPUT, GPIO_PULL_DOWN);
  BSP_gpioSetState(RS485_RE_PIN, 0);
  BSP_gpioSetState(RS485_DE_PIN, 0);
}

void
DEV_RS485_SendRequest (rs485_request_t *request)
{
  uint8_t packet[8];
  packet[0] = request->slave_id;
  packet[1] = request->function;
  packet[2] = (request->reg_addr >> 8) & 0xFF;
  packet[3] = request->reg_addr & 0xFF;

  if (request->function == RS485_FUNC_WRITE_SINGLE_REG)
  {
    packet[4] = (request->reg_value >> 8) & 0xFF;
    packet[5] = request->reg_value & 0xFF;
  }
  else
  {
    packet[4] = (request->reg_count >> 8) & 0xFF;
    packet[5] = request->reg_count & 0xFF;
  }

  uint16_t crc = MID_CheckCRC16(packet, 6);
  packet[6]    = crc & 0xFF;
  packet[7]    = (crc >> 8) & 0xFF;

  BSP_gpioSetState(RS485_RE_PIN, 1);
  BSP_gpioSetState(RS485_DE_PIN, 1); // Transmit mode
  vTaskDelay(pdMS_TO_TICKS(5));

  BSP_uartSendData(RS485_UART_NUM, packet, sizeof(packet));
  BSP_uartWaitTXDone(RS485_UART_NUM, pdMS_TO_TICKS(100));

  BSP_gpioSetState(RS485_RE_PIN, 0);
  BSP_gpioSetState(RS485_DE_PIN, 0); // Receive mode

  printf("Sent Modbus request.\r\n");
}

// Function to receive response from Slave
uint8_t
DEV_RS485_ReceiveResponse (uint8_t *buffer)
{
  int len        = 0;
  uint32_t start_time = esp_timer_get_time();

  while (esp_timer_get_time() - start_time < 500000)
  { // 500ms timeout
    int bytes_read
        = BSP_uartReadData(RS485_UART_NUM, buffer + len, 1, pdMS_TO_TICKS(50));
    if (bytes_read > 0)
    {
      len += bytes_read;
    }
    else if (len > 3)
    { // At least Address, Function, and CRC
      break;
    }
  }

  if (len < 5)
  { // Packet too short
    printf("Received packet is invalid (too short).");
    return false;
  }

  uint16_t received_crc   = (buffer[len - 1] << 8) | buffer[len - 2];
  uint16_t calculated_crc = MID_CheckCRC16(buffer, len - 2);

  if (received_crc == calculated_crc)
  {
    printf("Successfully received Modbus packet.");
    return true;
  }
  else
  {
    printf("CRC error: Expected 0x%04X, received 0x%04X",
           calculated_crc,
           received_crc);
    return false;
  }
}
/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/