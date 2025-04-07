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

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_FINGERPRINT_CreateTask(void);
  void APP_FINGERPRINT_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FINGERPRINT_H_ */