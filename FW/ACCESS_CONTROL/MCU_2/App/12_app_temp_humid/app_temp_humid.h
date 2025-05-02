#ifndef APP_TEMP_HUMID_H_
#define APP_TEMP_HUMID_H_

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

  void APP_TEMP_HUMID_CreateTask(void);
  void APP_TEMP_HUMID_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TEMP_HUMID_H_ */