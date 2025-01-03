/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "wifi.h"
#include "nvs_rw.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_ip_addr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

/******************************************************************************
 *      DEFINES
 *****************************************************************************/

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MAXIMUM_RETRY      10
#define TAG                "WIFI CONNECT"

#define NUM_WIFI_NVS "Num_ssid_nvs"
#define NUM_WIFI_KEY "Num_ssid_key"
#define SSID_NVS     "ssid_nvs"
#define PASS_NVS     "pass_nvs"

#define SEND_CONNECT_WIFI_UNSUCCESSFUL_BIT (1 << 2)

/******************************************************************************
 *  STATIC VARIABLES
 *****************************************************************************/

static WIFI_Status_t state_connected_wifi = CONNECT_FAIL;
static uint8_t       s_retry_num          = 0;
static uint8_t volatile num_wifi          = 0;
static EventGroupHandle_t s_wifi_event_group;
static char               ssid_name[1024];

/******************************************************************************
 *   STATIC FUNCTIONS
 *****************************************************************************/

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (s_retry_num < MAXIMUM_RETRY)
    {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    }
    else
    {
      state_connected_wifi = CONNECT_FAIL;
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGE(TAG, "connect to the AP fail");
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    state_connected_wifi     = CONNECT_OK;
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));

    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

static uint8_t WIFI_GetNumSSID(void)
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

static void WIFI_SetNumSSID(uint8_t num)
{
  nvs_handle_t nvsHandle;
  nvs_open(NUM_WIFI_NVS, NVS_READWRITE, &nvsHandle);
  nvs_set_u8(nvsHandle, NUM_WIFI_KEY, num);
}

static esp_err_t WIFI_ScanSSID(uint8_t *ssid, uint8_t id, uint8_t len)
{
  char ssid_key[32];
  sprintf(ssid_key, "%d ssid", id);
  return NVS_ReadString(SSID_NVS, (const char *)ssid_key, (char *)ssid, 32);
}

static esp_err_t WIFI_ScanPass(uint8_t *pass, uint8_t id, uint8_t len)
{
  char pass_key[32];
  sprintf(pass_key, "%d pass", id);
  return NVS_ReadString(PASS_NVS, (const char *)pass_key, (char *)pass, 32);
}

static esp_err_t WIFI_SetSSID(uint8_t *ssid, uint8_t id)
{
  char ssid_key[32];
  sprintf(ssid_key, "%d ssid", id);
  return NVS_WriteString(SSID_NVS, (const char *)ssid_key, (const char *)ssid);
}

static esp_err_t WIFI_SetPass(uint8_t *pass, uint8_t id)
{
  char pass_key[32];
  sprintf(pass_key, "%d pass", id);
  return NVS_WriteString(PASS_NVS, (const char *)pass_key, (const char *)pass);
}

static void print_auth_mode(int authmode)
{
  switch (authmode)
  {
  case WIFI_AUTH_OPEN:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
    break;
  case WIFI_AUTH_OWE:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OWE");
    break;
  case WIFI_AUTH_WEP:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
    break;
  case WIFI_AUTH_WPA_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
    break;
  case WIFI_AUTH_WPA2_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
    break;
  case WIFI_AUTH_WPA_WPA2_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
    break;
  case WIFI_AUTH_ENTERPRISE:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_ENTERPRISE");
    break;
  case WIFI_AUTH_WPA3_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
    break;
  case WIFI_AUTH_WPA2_WPA3_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
    break;
  case WIFI_AUTH_WPA3_ENT_192:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_ENT_192");
    break;
  default:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
    break;
  }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
  switch (pairwise_cipher)
  {
  case WIFI_CIPHER_TYPE_NONE:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
    break;
  case WIFI_CIPHER_TYPE_WEP40:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
    break;
  case WIFI_CIPHER_TYPE_WEP104:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
    break;
  case WIFI_CIPHER_TYPE_TKIP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
    break;
  case WIFI_CIPHER_TYPE_CCMP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
    break;
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
    break;
  case WIFI_CIPHER_TYPE_AES_CMAC128:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_AES_CMAC128");
    break;
  case WIFI_CIPHER_TYPE_SMS4:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_SMS4");
    break;
  case WIFI_CIPHER_TYPE_GCMP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP");
    break;
  case WIFI_CIPHER_TYPE_GCMP256:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP256");
    break;
  default:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
    break;
  }

  switch (group_cipher)
  {
  case WIFI_CIPHER_TYPE_NONE:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
    break;
  case WIFI_CIPHER_TYPE_WEP40:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
    break;
  case WIFI_CIPHER_TYPE_WEP104:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
    break;
  case WIFI_CIPHER_TYPE_TKIP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
    break;
  case WIFI_CIPHER_TYPE_CCMP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
    break;
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
    break;
  case WIFI_CIPHER_TYPE_SMS4:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_SMS4");
    break;
  case WIFI_CIPHER_TYPE_GCMP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP");
    break;
  case WIFI_CIPHER_TYPE_GCMP256:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP256");
    break;
  default:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
    break;
  }
}

// Function to check if a substring has appeared in the two-dimensional array or not
static uint8_t isDuplicate(char *str, char *substrings[], int count)
{
  for (int i = 0; i < count; i++)
  {
    if (strcmp(str, substrings[i]) == 0)
    {
      return 1; // Duplicate
    }
  }
  return 0; // Not duplicate
}

// Function to remove duplicate substrings from the substring array and store in a new string
static void deleteDuplicateSubstrings(char *str, char *result)
{
  char *token;
  char *delim = "\n";
  char *substrings[100]; // Two-dimensional array to store non-duplicate substrings
  int count = 0;         // Number of substrings added to the array

  // Parse the original string into substrings and store them in the array
  token = strtok(str, delim);
  while (token != NULL)
  {
    if (!isDuplicate(token, substrings, count))
    {
      substrings[count++] = token;
    }
    token = strtok(NULL, delim);
  }

  // Reconstruct the new string from the non-duplicate substrings
  result[0] = '\0'; // Initialize the new string
  for (int i = 0; i < count; i++)
  {
    strcat(result, substrings[i]); // Concatenate the substring into the new string
    strcat(result, "\n");          // Add '\n' character after each substring
  }
}

/******************************************************************************
 *   GLOBAL FUNCTIONS
 *****************************************************************************/

/**
 * The function `WIFI_StaInit` initializes the WiFi station mode on an ESP32 device.
 */
void WIFI_StaInit(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_wifi_stop();
}

/**
 * The function `WIFI_Scan` scans for nearby WiFi networks, retrieves their information, removes
 * duplicates, and returns the total number of unique networks found.
 *
 * @param data_name The `data_name` parameter in the `WIFI_Scan` function is a pointer to a uint8_t
 * array where the scanned Wi-Fi SSID names will be stored after processing. The function scans for
 * available Wi-Fi networks, extracts the SSID names, removes duplicates, and stores the unique
 *
 * @return The function `WIFI_Scan` returns the total number of WiFi networks scanned and stored in the
 * `data_name` buffer after removing any duplicate SSIDs.
 */
uint8_t WIFI_Scan(char *data_name)
{

  int i;
  memset(ssid_name, '\0', sizeof(ssid_name));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  uint16_t         number = SCAN_LIST_SIZE;
  wifi_ap_record_t ap_info[SCAN_LIST_SIZE];
  uint16_t         ap_count = 0;
  memset(ap_info, 0, sizeof(ap_info));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_wifi_scan_start(NULL, true);

  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

  uint16_t ssid_name_pos = 0;
  uint8_t  buffer[32];
  uint16_t total_number_wifi = 0;

  for (i = 0; i < number; i++)
  {
    uint16_t temp_pos = 0;

    memset(buffer, '\0', sizeof(buffer));
    memcpy(buffer, ap_info[i].ssid, strlen((char *)ap_info[i].ssid) + 1);
    buffer[strlen((char *)buffer)] = '\n';

    while (buffer[temp_pos] != '\n')
    {
      *(ssid_name + ssid_name_pos) = buffer[temp_pos];
      ssid_name_pos++;
      temp_pos++;
    }
    *(ssid_name + ssid_name_pos) = '\n';
    ssid_name_pos++;

    ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
    ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
    print_auth_mode(ap_info[i].authmode);
    if (ap_info[i].authmode != WIFI_AUTH_WEP)
    {
      print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
    }
    ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
  }

  deleteDuplicateSubstrings(ssid_name, data_name);

  for (i = 0; i < strlen(data_name); i++)
  {
    if (data_name[i] == '\n')
    {
      data_name[i] = '\r';
      total_number_wifi++;
    }
  }
  data_name[i] = '\0';

  return total_number_wifi;
}

/**
 * The function `WIFI_Connect` attempts to connect to a WiFi network using the provided SSID and
 * password, handling authentication modes and event notifications.
 *
 * @param ssid The `ssid` parameter in the `WIFI_Connect` function is a pointer to an array of
 * characters representing the SSID (Service Set Identifier) of the Wi-Fi network you want to connect
 * to. The SSID is essentially the name of the Wi-Fi network.
 * @param password The code you provided is a function `WIFI_Connect` that attempts to connect to a
 * WiFi network using the provided SSID and password. The function initializes the WiFi configuration,
 * sets up event handlers, and then tries to connect to the network.
 *
 * @return The function `WIFI_Connect` returns a value of type `WIFI_Status_t`, which is an enumeration
 * type. The possible return values are:
 * - `CONNECT_OK` if the connection to the Wi-Fi network was successful.
 * - `CONNECT_FAIL` if the connection to the Wi-Fi network failed.
 * - `UNEXPECTED_EVENT` if an unexpected event occurred during the connection process.
 */
WIFI_Status_t WIFI_Connect(uint8_t *ssid, uint8_t *password)
{
  esp_wifi_stop();

  s_wifi_event_group = xEventGroupCreate();

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&config);

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                      &event_handler, NULL, &instance_any_id);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                      &event_handler, NULL, &instance_got_ip);

  wifi_config_t wifi_config =
        {
            .sta =
                {
                    /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
                     * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
                     * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
                     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
                     */
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                    .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
                },
        };

  for (uint8_t i = 0; i < 32; i++)
  {
    wifi_config.sta.ssid[i]     = *(ssid + i);
    wifi_config.sta.password[i] = *(password + i);
  }

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();

  ESP_LOGI(TAG, "Wifi_init_station finished.");

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
   * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we
   * can test which event actually happened. */
  if (bits & WIFI_CONNECTED_BIT)
  {
    s_retry_num = 0;
    ESP_LOGI(TAG, "Connected to ap SSID:%s password:%s", wifi_config.sta.ssid,
             wifi_config.sta.password);
    return CONNECT_OK;
  }
  else if (bits & WIFI_FAIL_BIT)
  {
    s_retry_num = 0;
    ESP_LOGE(TAG, "Failed to connect to SSID:%s, password:%s",
             wifi_config.sta.ssid, wifi_config.sta.password);
    return CONNECT_FAIL;
  }
  else
  {
    s_retry_num = 0;
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    return UNEXPECTED_EVENT;
  }
}

/**
 * The function WIFI_ScanNVS scans for a specific WiFi network SSID and
 * retrieves its password if found.
 *
 * @param ssid The `ssid` parameter is a pointer to an array of characters
 * representing the SSID (Service Set Identifier) of a Wi-Fi network.
 * @param pass The `pass` parameter in the `WIFI_ScanNVS` function is a pointer
 * to a uint8_t array where the password of the WiFi network will be stored if
 * the corresponding SSID is found during the scan.
 *
 * @return The function `WIFI_ScanNVS` returns an `int8_t` value. If the
 * specified SSID is found during the scan, it returns the index of that SSID.
 * If the SSID is not found or if there are no SSIDs available, it returns -1.
 */
int8_t WIFI_ScanNVS(uint8_t *ssid, uint8_t *pass)
{
  int8_t  i;
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

/**
 * The function WIFI_StoreNVS stores a new WiFi SSID and password in non-volatile storage.
 *
 * @param ssid The `ssid` parameter is a pointer to an array of characters that represents the name of
 * the Wi-Fi network (Service Set Identifier).
 * @param password The `password` parameter in the `WIFI_StoreNVS` function is a pointer to an array of
 * `uint8_t` data type, which is typically used to store a password for a Wi-Fi network.
 */
void WIFI_StoreNVS(uint8_t *ssid, uint8_t *password)
{
  num_wifi = WIFI_GetNumSSID();
  num_wifi++;
  WIFI_SetNumSSID(num_wifi);
  WIFI_SetSSID(ssid, num_wifi);
  WIFI_SetPass(password, num_wifi);
}

WIFI_Status_t WIFI_state_connect(void)
{
  return state_connected_wifi;
}