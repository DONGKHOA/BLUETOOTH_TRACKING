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

/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/