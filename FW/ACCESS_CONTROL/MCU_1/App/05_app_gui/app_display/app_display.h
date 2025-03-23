#ifndef APP_DISPLAY_H_
#define APP_DISPLAY_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

   #define DISP_BUF_SIZE (LV_HOR_RES * 40)

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_DISPLAY_CreateTask(void);
  void APP_DISPLAY_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_DISPLAY_H_ */