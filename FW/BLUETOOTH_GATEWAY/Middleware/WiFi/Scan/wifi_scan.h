#ifndef WiFi_SCAN_H_
#define WiFi_SCAN_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  uint8_t WIFI_Scan(char *data_name);
  void    WIFI_Connect(uint8_t *ssid, uint8_t *password);

#ifdef __cplusplus
}
#endif

#endif /* WiFi_SCAN_H_ */