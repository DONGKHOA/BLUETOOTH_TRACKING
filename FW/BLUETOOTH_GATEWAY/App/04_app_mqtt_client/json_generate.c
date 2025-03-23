/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "json_generate.h"
#include "cJSON.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "JSON_GENERATE"

/******************************************************************************
 *    PRIVATE FUNCTION
 *****************************************************************************/

/******************************************************************************
 *    PUBLIC FUNCTION
 *****************************************************************************/

char *
create_device_json (char       *dev_id,
                    char       *dev_version,
                    dev_info_t *dev_list,
                    uint8_t     dev_list_len)
{
  // Create the root JSON object
  cJSON *root = cJSON_CreateObject();
  if (root == NULL)
  {
    return NULL; // Memory allocation failed
  }

  // Add top-level string fields to the JSON object
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
  for (uint8_t i = 0; i < dev_list_len; ++i)
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
    // Add the populated item object to the "dev_list" array
    cJSON_AddItemToArray(json_list, item);
  }

  cJSON_AddNumberToObject(root, "dev_list_len", dev_list_len);

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