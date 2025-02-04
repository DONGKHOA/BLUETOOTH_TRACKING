/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_wifi_connect.h"
#include "app_data.h"
#include "wifi.h"

#include <string.h>

/******************************************************************************
 *      DEFINES
 *****************************************************************************/

#define SSID_WIFI "🍑"
#define PASS_WIFI "20112011"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct wifi_connect_data
{
  uint8_t *p_ssid;
  uint8_t *p_pass;
} wifi_connect_data_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static wifi_connect_data_t s_wifi_connect_data;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_WIFI_CONNECT_Init (void)
{
  s_wifi_connect_data.p_pass = s_data_system.u8_pass;
  s_wifi_connect_data.p_ssid = s_data_system.u8_ssid;

  memcpy(s_wifi_connect_data.p_ssid, SSID_WIFI, 5);
  memcpy(s_wifi_connect_data.p_pass, PASS_WIFI, 9);

  WIFI_StaInit();
  WIFI_Connect(s_wifi_connect_data.p_ssid, s_wifi_connect_data.p_pass);
}