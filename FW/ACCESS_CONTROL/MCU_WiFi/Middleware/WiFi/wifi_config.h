#ifndef WiFi_CONFIG_H_
#define WiFi_CONFIG_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SCAN_LIST_SIZE      50
#define LIMIT_STORE_WIFI    10

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  uint8_t WIFI_Scan(char *data_name);

#ifdef __cplusplus
}
#endif

#endif /* WiFi_CONFIG_H_ */