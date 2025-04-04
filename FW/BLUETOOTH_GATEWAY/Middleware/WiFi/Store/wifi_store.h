#ifndef WiFi_STORE_H_
#define WiFi_STORE_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "esp_err.h"
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  /**
   * The function WIFI_ScanNVS scans for a specific WiFi network SSID and
   * retrieves its password if found.
   *
   * @param ssid The `ssid` parameter is a pointer to an array of characters
   * representing the SSID (Service Set Identifier) of a Wi-Fi network.
   * @param pass The `pass` parameter in the `WIFI_ScanNVS` function is a
   * pointer to a uint8_t array where the password of the WiFi network will be
   * stored if the corresponding SSID is found during the scan.
   *
   * @return The function `WIFI_ScanNVS` returns an `int8_t` value. If the
   * specified SSID is found during the scan, it returns the index of that SSID.
   * If the SSID is not found or if there are no SSIDs available, it returns -1.
   */
  int8_t WIFI_ScanNVS(uint8_t *ssid, uint8_t *pass);

  /**
   * The function WIFI_StoreNVS stores a new WiFi SSID and password in
   * non-volatile storage.
   *
   * @param ssid The `ssid` parameter is a pointer to an array of characters
   * that represents the name of the Wi-Fi network (Service Set Identifier).
   * @param password The `password` parameter in the `WIFI_StoreNVS` function is
   * a pointer to an array of `uint8_t` data type, which is typically used to
   * store a password for a Wi-Fi network.
   */
  void WIFI_StoreNVS(uint8_t *ssid, uint8_t *password);

  uint8_t   WIFI_GetNumSSID(void);
  void      WIFI_SetNumSSID(uint8_t num);
  esp_err_t WIFI_ScanSSID(uint8_t *ssid, uint8_t id, uint8_t len);
  esp_err_t WIFI_ScanPass(uint8_t *pass, uint8_t id, uint8_t len);
  esp_err_t WIFI_SetSSID(uint8_t *ssid, uint8_t id);
  esp_err_t WIFI_SetPass(uint8_t *pass, uint8_t id);

#ifdef __cplusplus
}
#endif

#endif /* WiFi_STORE_H_ */