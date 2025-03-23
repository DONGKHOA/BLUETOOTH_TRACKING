/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "wifi_helper.h"

#include "nvs_rw.h"
#include "nvs.h"
#include "nvs_flash.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "WIFI_STORE"

#define NUM_WIFI_NVS "Num_ssid_nvs"
#define NUM_WIFI_KEY "Num_ssid_key"
#define SSID_NVS     "ssid_nvs"
#define PASS_NVS     "pass_nvs"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static uint8_t   WIFI_GetNumSSID(void);
static void      WIFI_SetNumSSID(uint8_t num);
static esp_err_t WIFI_ScanSSID(uint8_t *ssid, uint8_t id, uint8_t len);
static esp_err_t WIFI_ScanPass(uint8_t *pass, uint8_t id, uint8_t len);
static esp_err_t WIFI_SetSSID(uint8_t *ssid, uint8_t id);
static esp_err_t WIFI_SetPass(uint8_t *pass, uint8_t id);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static uint8_t volatile num_wifi = 0;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

int8_t
WIFI_ScanNVS (uint8_t *ssid, uint8_t *pass)
{
  int8_t i;
  uint8_t ssid_temp[32];

  num_wifi = WIFI_GetNumSSID();
  if (num_wifi == 0)
  {
      return -1;
  }

  for (i = 1; i <= num_wifi; i++)
  {
      WIFI_ScanSSID(ssid_temp, i, 32);
      if (memcmp(ssid_temp, ssid, strlen((char *)ssid)) == 0)
      {
          WIFI_ScanPass(pass, i, 32);
          return i;
      }
  }
  return -1;
}

void WIFI_StoreNVS(uint8_t *ssid, uint8_t *password)
{
    num_wifi = WIFI_GetNumSSID();
    num_wifi++;
    WIFI_SetNumSSID(num_wifi);
    WIFI_SetSSID(ssid, num_wifi);
    WIFI_SetPass(password, num_wifi);
}


/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static uint8_t
WIFI_GetNumSSID (void)
{
  uint8_t      num;
  nvs_handle_t nvsHandle;
  nvs_open(NUM_WIFI_NVS, NVS_READWRITE, &nvsHandle);
  esp_err_t err = nvs_get_u8(nvsHandle, NUM_WIFI_KEY, &num);

  if (err == ESP_OK)
  {
    return num;
  }
  else
  {
    nvs_set_u8(nvsHandle, NUM_WIFI_KEY, num);
    return 0;
  }
}

static void
WIFI_SetNumSSID (uint8_t num)
{
  nvs_handle_t nvsHandle;
  nvs_open(NUM_WIFI_NVS, NVS_READWRITE, &nvsHandle);
  nvs_set_u8(nvsHandle, NUM_WIFI_KEY, num);
}

static esp_err_t
WIFI_ScanSSID (uint8_t *ssid, uint8_t id, uint8_t len)
{
  char ssid_key[32];
  sprintf(ssid_key, "%d ssid", id);
  return NVS_ReadString(SSID_NVS, (const char *)ssid_key, (char *)ssid, 32);
}

static esp_err_t
WIFI_ScanPass (uint8_t *pass, uint8_t id, uint8_t len)
{
  char pass_key[32];
  sprintf(pass_key, "%d pass", id);
  return NVS_ReadString(PASS_NVS, (const char *)pass_key, (char *)pass, 32);
}

static esp_err_t
WIFI_SetSSID (uint8_t *ssid, uint8_t id)
{
  char ssid_key[32];
  sprintf(ssid_key, "%d ssid", id);
  return NVS_WriteString(SSID_NVS, (const char *)ssid_key, (const char *)ssid);
}

static esp_err_t
WIFI_SetPass (uint8_t *pass, uint8_t id)
{
  char pass_key[32];
  sprintf(pass_key, "%d pass", id);
  return NVS_WriteString(PASS_NVS, (const char *)pass_key, (const char *)pass);
}