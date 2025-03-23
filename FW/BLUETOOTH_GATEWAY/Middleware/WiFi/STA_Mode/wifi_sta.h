#ifndef WiFi_STA_H_
#define WiFi_STA_H_

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

  void WIFI_StaInit(void);
  void WIFI_Connect(uint8_t *ssid, uint8_t *pass);

#ifdef __cplusplus
}
#endif

#endif /* WiFi_STA_H_ */