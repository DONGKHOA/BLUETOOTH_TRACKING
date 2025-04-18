#ifndef APP_STATUS_LED_H_
#define APP_STATUS_LED_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_STATUS_LED_CreateTask(void);
  void APP_STATUS_LED_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_STATUS_LED_H_ */