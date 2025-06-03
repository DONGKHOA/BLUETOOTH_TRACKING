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
create_device_json (char *dev_id, char *dev_version, dev_info_t *p_dev_list)
{
  cJSON *root = cJSON_CreateObject();
  if (root == NULL)
  {
    return NULL;
  }

  cJSON_AddStringToObject(root, "dev_id", dev_id);
  cJSON_AddStringToObject(root, "dev_version", dev_version);

  cJSON *json_list = cJSON_CreateArray();
  if (json_list == NULL)
  {
    cJSON_Delete(root);
    return NULL;
  }

  cJSON_AddItemToObject(root, "dev_list", json_list);
  cJSON *item = cJSON_CreateObject();

  cJSON_AddStringToObject(item, "mac", p_dev_list[0].mac);
  cJSON_AddNumberToObject(item, "rssi", p_dev_list[0].rssi);
  cJSON_AddItemToArray(json_list, item);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str == NULL)
  {
    cJSON_Delete(root);
    return NULL;
  }

  cJSON_Delete(root);

  return json_str;
}