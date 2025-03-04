#ifndef APP_READ_DATA_H_
#define APP_READ_DATA_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include "ads1115.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_ReadData_CreateTask(void);
  void APP_ReadData_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_READ_DATA_H_ */