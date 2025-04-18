#ifndef APP_HANDLE_WIFI_H_
#define APP_HANDLE_WIFI_H_

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

  void APP_HANDLE_WIFI_CreateTask(void);
  void APP_HANDLE_WIFI_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_HANDLE_WIFI_H_ */