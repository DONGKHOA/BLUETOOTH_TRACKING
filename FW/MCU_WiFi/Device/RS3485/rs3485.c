/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_timer.h"

#include "gpio.h"
#include "rs3485.h"
#include "crc16.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define RS3485_UART_NUM UART_NUM_2
#define RS3485_TXD_PIN  GPIO_NUM_15
#define RS3485_RXD_PIN  GPIO_NUM_16
#define RS3485_RE_PIN   GPIO_NUM_21
#define RS3485_DE_PIN   GPIO_NUM_38

#define RS3485_BAUDRATE    9600
#define RS3485_DATA_BIT    UART_DATA_8_BITS
#define RS3485_PARITY_BIT  UART_PARITY_EVEN
#define RS3485_HW_FLOWCTRL UART_HW_FLOWCTRL_DISABLE
#define RS3485_STOP_BITS   UART_STOP_BITS_1

#define RS3485_BUF_SIZE 256

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
DEV_RS3485_Init (void)
{
  BSP_uartDriverInit(RS3485_UART_NUM,
                     RS3485_TXD_PIN,
                     RS3485_RXD_PIN,
                     RS3485_BAUDRATE,
                     RS3485_DATA_BIT,
                     RS3485_PARITY_BIT,
                     RS3485_HW_FLOWCTRL,
                     RS3485_STOP_BITS);

  // Default: receive mode
  BSP_gpioSetDirection(RS3485_RE_PIN, GPIO_MODE_OUTPUT);
  BSP_gpioSetDirection(RS3485_DE_PIN, GPIO_MODE_OUTPUT);
  BSP_gpioSetState(RS3485_RE_PIN, 0);
  BSP_gpioSetState(RS3485_DE_PIN, 0);
}

void
DEV_RS3485_SendRequest (rs3485_request_t *request)
{
  uint8_t packet[8];
  packet[0] = request->slave_id;
  packet[1] = request->function;
  packet[2] = (request->reg_addr >> 8) & 0xFF;
  packet[3] = request->reg_addr & 0xFF;

  if (request->function == RS3485_FUNC_WRITE_SINGLE_REG)
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

  BSP_gpioSetState(RS3485_RE_PIN, 1);
  BSP_gpioSetState(RS3485_DE_PIN, 1); // Transmit mode
  vTaskDelay(pdMS_TO_TICKS(5));

  BSP_uartSendData(RS3485_UART_NUM, packet, sizeof(packet));
  BSP_uartWaitTXDone(RS3485_UART_NUM, pdMS_TO_TICKS(100));

  BSP_gpioSetState(RS3485_RE_PIN, 0);
  BSP_gpioSetState(RS3485_DE_PIN, 0); // Receive mode

  printf("Sent Modbus request.\r\n");
}

// Function to receive response from Slave
uint8_t
DEV_RS3485_ReceiveResponse (uint8_t *buffer)
{
  int      /* The `len` variable is used to keep track of the number of bytes read into the `buffer`
  while receiving a response from the Slave device. It is incremented as bytes are read
  from the UART buffer and added to the `buffer` array. The `len` variable is also used to
  determine if the received packet is long enough to be considered valid, as it needs to
  contain at least the Address, Function, and CRC bytes. */
  /* The `len` variable is used to keep track of the length of the data that has been read
  into the `buffer` while receiving a response from the Slave device. It is incremented as
  data is read into the buffer, and it is used to determine when to stop reading data based
  on the timeout condition and the minimum required length of the received packet. */
  len        = 0;
  uint32_t start_time = esp_timer_get_time();

  while (esp_timer_get_time() - start_time < 500000)
  { // 500ms timeout
    int bytes_read
        = BSP_uartReadData(RS3485_UART_NUM, buffer + len, 1, pdMS_TO_TICKS(50));
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