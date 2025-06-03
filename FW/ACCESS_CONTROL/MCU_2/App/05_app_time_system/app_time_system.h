#ifndef APP_TIME_SYSTEM_H_
#define APP_TIME_SYSTEM_H_

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

  void APP_TIME_SYSTEM_CreateTask(void);
  void APP_TIME_SYSTEM_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIME_SYSTEM_H_ */

