// #ifndef RS3485_H
// #define RS3485_H

// /******************************************************************************
//  *      INCLUDES
//  *****************************************************************************/

// #include <stdint.h>
// #include "uart.h"

// #ifdef __cplusplus
// extern "C"
// {
// #endif

//   /****************************************************************************
//    *   PUBLIC DEFINES
//    ***************************************************************************/

// #define RS3485_FUNC_READ_HOLDING_REGS 0x03 // Read holding registers
// #define RS3485_FUNC_READ_INPUT_REGS   0x04 // Read input registers
// #define RS3485_FUNC_WRITE_SINGLE_REG  0x06 // Write single register
// #define RS3485_FUNC_WRITE_MULTI_REGS  0x10 // Write multiple registers

//   /****************************************************************************
//    *   PUBLIC TYPEDEFS
//    ***************************************************************************/
//   typedef struct
//   {
//     uint8_t  slave_id;  // Slave device address
//     uint8_t  function;  // Modbus function code
//     uint16_t reg_addr;  // Register address
//     uint16_t reg_value; // Value to write (if any)
//     uint16_t reg_count; // Number of registers to read/write
//   } rs3485_request_t;

//   /****************************************************************************
//    *   PUBLIC FUNCTION
//    ***************************************************************************/

//   /**
//    * @brief Initializes the RS3485 device.
//    *
//    * This function sets up the necessary configurations and initializes the RS3485 device
//    * for communication. It should be called before any other operations are performed on the device.
//    */
//   void DEV_RS3485_Init(void);

//   /**
//    * @brief Sends a Modbus request.
//    *
//    * This function sends a Modbus request using the RS3485 protocol.
//    *
//    * @param request Pointer to the rs3485_request_t structure containing the request information to be sent.
//    */
  
//   void DEV_RS3485_SendRequest(rs3485_request_t *request);

//   /**
//    * @brief Receives a response from the RS3485 device.
//    *
//    * This function reads data from the RS3485 device into the provided buffer.
//    *
//    * @param buffer Pointer to the buffer where the received data will be stored.
//    * @return uint8_t The number of bytes received.
//    */
//   uint8_t DEV_RS3485_ReceiveResponse(uint8_t *buffer);

// #ifdef __cplusplus
// }
// #endif

// #endif /* RS3485_H*/