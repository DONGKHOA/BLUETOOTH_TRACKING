#ifndef RS3485_H
#define RS3485_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "uart.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

#define RS3485_FUNC_READ_HOLDING_REGS 0x03 // Đọc thanh ghi giữ
#define RS3485_FUNC_READ_INPUT_REGS   0x04 // Đọc thanh ghi input
#define RS3485_FUNC_WRITE_SINGLE_REG  0x06 // Ghi một thanh ghi
#define RS3485_FUNC_WRITE_MULTI_REGS  0x10 // Ghi nhiều thanh ghi

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct
  {
    uint8_t  slave_id;  // Địa chỉ thiết bị Slave
    uint8_t  function;  // Mã lệnh Modbus
    uint16_t reg_addr;  // Địa chỉ thanh ghi
    uint16_t reg_value; // Giá trị ghi vào (nếu có)
    uint16_t reg_count; // Số lượng thanh ghi cần đọc/ghi
  } rs3485_request_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void DEV_RS3485_Init(void);

  // Gửi yêu cầu Modbus
  void DEV_RS3485_SendRequest(rs3485_request_t *request);

  // Nhận phản hồi từ Slave
  uint8_t DEV_RS3485_ReceiveResponse(uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* RS3485_H*/