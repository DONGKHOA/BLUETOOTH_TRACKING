#ifndef APP_LOCAL_DATABASE_H_
#define APP_LOCAL_DATABASE_H_

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

  void APP_LOCAL_DATABASE_CreateTask(void);
  void APP_LOCAL_DATABASE_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_LOCAL_DATABASE_H_ */