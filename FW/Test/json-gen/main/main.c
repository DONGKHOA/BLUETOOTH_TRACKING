#include <stdio.h>
#include <string.h>

#include "cJSON.h"

typedef struct
{
  const char  mac[13]; // Device MAC address (string)
  int         rssi;   // RSSI value (integer)
  const char *ad;     // Advertising data (string, e.g., hex-encoded)
  long long   ts; // Timestamp (e.g., epoch or counter, use 64-bit if needed)
} dev_info_t;

char json[10240];

char *
create_device_json (const char       *dev_wifi_sta_mac,
                    const char       *dev_ble_mac,
                    const char       *dev_sn,
                    const char       *dev_id,
                    const char       *dev_version,
                    const dev_info_t *dev_list,
                    size_t            dev_list_len)
{
  // Create the root JSON object
  cJSON *root = cJSON_CreateObject();
  if (root == NULL)
  {
    return NULL; // Memory allocation failed
  }

  // Add top-level string fields to the JSON object
  cJSON_AddStringToObject(root, "dev_wifi_sta_mac", dev_wifi_sta_mac);
  cJSON_AddStringToObject(root, "dev_ble_mac", dev_ble_mac);
  cJSON_AddStringToObject(root, "dev_sn", dev_sn);
  cJSON_AddStringToObject(root, "dev_id", dev_id);
  cJSON_AddStringToObject(root, "dev_version", dev_version);

  // Create a JSON array for "dev_list"
  cJSON *json_list = cJSON_CreateArray();
  if (json_list == NULL)
  {
    cJSON_Delete(root); // Free root and any added children
    return NULL;
  }
  // Add the "dev_list" array to the root object
  cJSON_AddItemToObject(root, "dev_list", json_list);

  // Iterate over the provided device list and add each entry to the JSON array
  for (size_t i = 0; i < dev_list_len; ++i)
  {
    // Create a JSON object for this list entry
    cJSON *item = cJSON_CreateObject();
    if (item == NULL)
    {
      cJSON_Delete(root); // Free everything allocated so far
      return NULL;
    }
    // Add fields of the device entry to the JSON object
    cJSON_AddStringToObject(item, "mac", dev_list[i].mac);
    cJSON_AddNumberToObject(item, "rssi", dev_list[i].rssi);
    cJSON_AddStringToObject(item, "ad", dev_list[i].ad);
    cJSON_AddNumberToObject(item, "ts", dev_list[i].ts);
    // Add the populated item object to the "dev_list" array
    cJSON_AddItemToArray(json_list, item);
  }

  cJSON_AddNumberToObject(root, "dev_list_len", dev_list_len);
  cJSON_AddNumberToObject(root, "publish_ts", 0);

  // Convert the cJSON object to a JSON string
  // Use cJSON_PrintUnformatted for compact string (cJSON_Print can be used for
  // pretty format)
  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str == NULL)
  {
    cJSON_Delete(root);
    return NULL;
  }

  // Clean up the cJSON object hierarchy to free memory
  cJSON_Delete(root);
  // At this point, json_str contains the JSON text and should be freed by the
  // caller.

  return json_str;
}

void
app_main ()
{
  dev_info_t dev_list[2] = {
    { .mac  = "C300003561E2",
      .rssi = -48,
      .ad   = "0201061AFF4C000215E2C56DB5DFFB48D2B060D0F5A71096E000000000C5",
      .ts   = 0 },
    { .mac  = "C300003561E2",
      .rssi = -50,
      .ad   = "0201061AFF4C000215E3C56DB5DFFB48D2B060D0F5A71096E000000000C5",
      .ts   = 0 }
  };

  // Get the JSON string pointer from the generator
  char *generated_str = create_device_json("E465B840611C",
                                           "E465B840611E",
                                           "e8d6c75034f7211e",
                                           "bb0611c",
                                           "3.0.3",
                                           &dev_list[0],
                                           2);

  // Check validity before copying
  if (generated_str)
  {
    strncpy(json, generated_str, sizeof(json) - 1);
    json[sizeof(json) - 1] = '\0'; // Ensure null-termination
    free(generated_str);           // Free the original allocated string
  }

  printf("%s\r\n", json);
}