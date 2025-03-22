#ifndef APP_FINGERPRINT_H_
#define APP_FINGERPRINT_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
/*** UART 2 peripheral *******************************************************/

#define UART_TXD  GPIO_NUM_15
#define UART_RXD  GPIO_NUM_17
#define UART_NUM  UART_NUM_2
#define BAUD_RATE 115200

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_FINGERPRINT_CreateTask(void);
  void APP_FINGERPRINT_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FINGERPRINT_H_ */