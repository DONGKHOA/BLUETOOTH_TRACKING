#ifndef MODBUSRTU_H
#define MODBUSRTU_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "rs485.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

#define MODBUSRTU_FUNC_READ_HOLDING_REGS 0x03 // Read holding registers
#define MODBUSRTU_FUNC_READ_INPUT_REGS   0x04 // Read input registers
#define MODBUSRTU_FUNC_WRITE_SINGLE_REG  0x06 // Write single register
#define MODBUSRTU_FUNC_WRITE_MULTI_REGS  0x10 // Write multiple registers

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/
  typedef struct
  {
    uint8_t  slave_id;  // Slave device address
    uint8_t  function;  // Modbus function code
    uint16_t reg_addr;  // Register address
    uint16_t reg_count; // Number of registers to read/write
  } modbusRTU_request_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void MID_ModbusRTU_InfoMessage(modbusRTU_request_t *request,
                                 uint32_t             u32_slaveid,
                                 uint32_t             u32_function,
                                 uint32_t             u32_reg_addr,
                                 uint32_t             u32_reg_count);

  void MID_ModbusRTU_SendRequest(uart_port_num_t      e_uart_port,
                                 gpio_num_t           e_modbus_re_io,
                                 gpio_num_t           e_modbus_de_io,
                                 modbusRTU_request_t *request);

#ifdef __cplusplus
}
#endif

#endif /* MODBUSRTU_H*/