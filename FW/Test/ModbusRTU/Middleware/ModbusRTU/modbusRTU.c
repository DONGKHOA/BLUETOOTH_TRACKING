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
 *  PRIVATE TYPEDEFS
 *****************************************************************************/

typedef enum
{
  STATE_HEADING = 0,
  STATE_DATA,
} modbus_state_t;

/******************************************************************************
 *  PRIVATE DATA
 *****************************************************************************/

// modbusRTU_request_t  modbus_request;
modbusRTU_rec_data_t modbus_data;
modbus_state_t       e_modbus_state = STATE_HEADING;

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
  uint16_t crc = MID_EncodeCRC16(packet, 6);
  packet[6]    = crc & 0xFF;        // LSB
  packet[7]    = (crc >> 8) & 0xFF; // MSB

  DEV_RS485_TransmitMode(e_modbus_re_io, e_modbus_de_io);
  DEV_RS485_SendData(e_uart_port, packet, sizeof(packet));

  printf("Sent Modbus request.\r\n");
}

void
MID_ModbusRTU_ReceiveResponse (uart_port_num_t      e_uart_port,
                               modbusRTU_request_t *request)
{
  int      Bytes              = 0;
  uint8_t  u8_byte_count      = 0;
  uint8_t  u8_expected_bytes  = 0;
  uint16_t u16_received_crc   = 0;
  uint16_t u16_calculated_crc = 0;

  while (1)
  {
    switch (e_modbus_state)
    {
      case STATE_HEADING:

        Bytes = DEV_RS485_ReceiveByte(
            e_uart_port,
            &modbus_data.u8_data_rec[modbus_data.u8_index_data_rec],
            pdMS_TO_TICKS(1));

        if (Bytes > 0)
        {

          modbus_data.u8_index_data_rec++;

          if (modbus_data.u8_data_rec[0] == request->slave_id)
          {
            if (modbus_data.u8_data_rec[1] == request->function)
            {
              e_modbus_state = STATE_DATA;
            }
            else
            {
              modbus_data.u8_index_data_rec = 0;
              continue;
            }
          }
          else
          {
            modbus_data.u8_index_data_rec = 0;
            continue;
          }
        }
        break;

      case STATE_DATA:

        Bytes = DEV_RS485_ReceiveByte(
            e_uart_port,
            &modbus_data.u8_data_rec[modbus_data.u8_index_data_rec],
            pdMS_TO_TICKS(1));

        if (Bytes > 0)
        {
          modbus_data.u8_index_data_rec++;

          // Check the number of data need to read via Byte Count
          if (modbus_data.u8_index_data_rec == 3)
          {
            u8_byte_count = modbus_data.u8_data_rec[2];
            u8_expected_bytes
                = 3 + u8_byte_count + 2; // Slave ID (1), Function (1), Byte
                                         // Count (1), Data (N*8), CRC (2)
          }

          if (modbus_data.u8_index_data_rec >= u8_expected_bytes)
          {
            // Extract CRC from last two bytes
            u16_received_crc
                = (modbus_data.u8_data_rec[modbus_data.u8_index_data_rec - 1]
                   << 8)
                  | modbus_data.u8_data_rec[modbus_data.u8_index_data_rec - 2];

            u16_calculated_crc = MID_EncodeCRC16(
                modbus_data.u8_data_rec, modbus_data.u8_index_data_rec - 2);

            if (u16_received_crc == u16_calculated_crc)
            {
              // Valid CRC: Data had been received successfully
              for (int i = 0; i < u8_expected_bytes; i++)
              {
                printf("Data receive: %d", modbus_data.u8_data_rec[i]);
              }
              modbus_data.u8_index_data_rec = 0;
              e_modbus_state                = STATE_HEADING;
            }
            else
            {
              memset(
                  modbus_data.u8_data_rec, 0, sizeof(modbus_data.u8_data_rec));
              modbus_data.u8_index_data_rec = 0;
              e_modbus_state                = STATE_HEADING;

              // Requese the data again
            }
          }
        }
        break;

      default:
        break;
    }
  }
}



/******************************************************************************
 *   PRIVATE FUNCTION
 *****************************************************************************/