#ifndef APP_RTC_H_
#define APP_RTC_H_

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

  void APP_RTC_CreateTask(void);
  void APP_RTC_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_RTC_H_ */