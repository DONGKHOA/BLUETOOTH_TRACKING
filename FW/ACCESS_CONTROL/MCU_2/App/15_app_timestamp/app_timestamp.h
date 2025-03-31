#ifndef APP_TIMESTAMP_H_
#define APP_TIMESTAMP_H_

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

  void APP_TIMESTAMP_CreateTask(void);
  void APP_TIMESTAMP_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIMESTAMP_H_ */