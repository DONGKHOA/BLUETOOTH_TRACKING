#ifndef APP_CONTROL_SDCARD_H_
#define APP_CONTROL_SDCARD_H_

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

  void APP_CONTROL_SDCARD_CreateTask(void);
  void APP_CONTROL_SDCARD_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CONTROL_SDCARD_H_ */