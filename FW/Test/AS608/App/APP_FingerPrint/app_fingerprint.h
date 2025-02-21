#ifndef APP_FINGERPRINT_H_
#define APP_FINGERPRINT_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include "as608.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define UART_TXD GPIO_NUM_16
#define UART_RXD GPIO_NUM_17

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_FingerPrint_CreateTask(void);
  void APP_FingerPrint_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FINGERPRINT_H_ */