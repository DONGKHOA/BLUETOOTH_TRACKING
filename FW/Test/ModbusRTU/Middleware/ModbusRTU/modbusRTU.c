/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "esp_timer.h"

#include "crc16.h"
#include "modbusRTU.h"

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
MID_ModbusRTU_InfoMessage (modbusRTU_request_t *request,
                           uint32_t             u32_slaveid,
                           uint32_t             u32_function,
                           uint32_t             u32_reg_addr,
                           uint32_t             u32_reg_count)
{
  request->slave_id  = u32_slaveid;
  request->function  = u32_function;
  request->reg_addr  = u32_reg_addr;
  request->reg_count = u32_reg_count;
}

void
MID_ModbusRTU_SendRequest (uart_port_num_t      e_uart_port,
                           gpio_num_t           e_modbus_re_io,
                           gpio_num_t           e_modbus_de_io,
                           modbusRTU_request_t *request)
{
  uint8_t packet[8];
  packet[0] = request->slave_id;
  packet[1] = request->function;
  packet[2] = (request->reg_addr >> 8) & 0xFF;
  packet[3] = request->reg_addr & 0xFF;

  if (request->function == MODBUSRTU_FUNC_WRITE_SINGLE_REG)
  {
    packet[4] = (request->reg_count >> 8) & 0xFF;
    packet[5] = request->reg_count & 0xFF;
  }
  else
  {
    packet[4] = (request->reg_count >> 8) & 0xFF;
    packet[5] = request->reg_count & 0xFF;
  }

  // Little Endian
  uint16_t crc = MID_CheckCRC16(packet, 6);
  packet[6]    = crc & 0xFF;        // LSB
  packet[7]    = (crc >> 8) & 0xFF; // MSB

  DEV_RS485_SendData(
      e_uart_port, e_modbus_re_io, e_modbus_de_io, packet, sizeof(packet));

  printf("Sent Modbus request.\r\n");
}

// // Function to receive response from Slave
// uint8_t
// MID_ModbusRTU_ReceiveResponse (uint8_t *buffer)
// {
//   int      len        = 0;
//   uint32_t start_time = esp_timer_get_time();

//   while (esp_timer_get_time() - start_time < 500000)
//   { // 500ms timeout
//     int bytes_read
//         = BSP_uartReadData(RS485_UART_NUM, buffer + len, 1,
//         pdMS_TO_TICKS(50));
//     if (bytes_read > 0)
//     {
//       len += bytes_read;
//     }
//     else if (len > 3)
//     { // At least Address, Function, and CRC
//       break;
//     }
//   }

//   if (len < 5)
//   { // Packet too short
//     printf("Received packet is invalid (too short).");
//     return false;
//   }

//   uint16_t received_crc   = (buffer[len - 1] << 8) | buffer[len - 2];
//   uint16_t calculated_crc = MID_CheckCRC16(buffer, len - 2);

//   if (received_crc == calculated_crc)
//   {
//     printf("Successfully received Modbus packet.");
//     return true;
//   }
//   else
//   {
//     printf("CRC error: Expected 0x%04X, received 0x%04X",
//            calculated_crc,
//            received_crc);
//     return false;
//   }
// }
/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/