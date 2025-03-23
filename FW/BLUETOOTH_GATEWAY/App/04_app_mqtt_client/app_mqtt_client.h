#ifndef APP_MQTT_CLIENT_H_
#define APP_MQTT_CLIENT_H_

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

  void APP_MQTT_CLIENT_CreateTask(void);
  void APP_MQTT_CLIENT_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MQTT_CLIENT_H_ */