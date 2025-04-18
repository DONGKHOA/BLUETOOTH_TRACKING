/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "wifi_helper.h"

#include "esp_wifi.h"
#include "esp_log.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "WIFI_CONFIG"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static uint8_t isDuplicate(char *str, char *substrings[], int count);
static void    deleteDuplicateSubstrings(char *str, char *result);
static void    print_auth_mode(int authmode);
static void    print_cipher_type(int pairwise_cipher, int group_cipher);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static char ssid_name[1024];

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

uint8_t
WIFI_Scan (char *data_name)
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

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

// Function to check if a substring has appeared in the two-dimensional array or
// not
static uint8_t
isDuplicate (char *str, char *substrings[], int count)
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

// Function to remove duplicate substrings from the substring array and store in
// a new string
static void
deleteDuplicateSubstrings (char *str, char *result)
{
  char *token;
  char *delim = "\n";
  char *substrings[100]; // Two-dimensional array to store non-duplicate
                         // substrings
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
    strcat(result,
           substrings[i]); // Concatenate the substring into the new string
    strcat(result, "\n");  // Add '\n' character after each substring
  }
}

static void
print_auth_mode (int authmode)
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

static void
print_cipher_type (int pairwise_cipher, int group_cipher)
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