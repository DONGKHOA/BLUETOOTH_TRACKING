/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "cJSON.h"
#include "esp_log.h"

#include <string.h>

#include "handle_data.h"

#include "app_data.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "ENCODE"

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

char *
ENCODE_Sync_Data (int      *user_id,
                  int      *face,
                  int      *finger,
                  char    **role,
                  char    **user_name,
                  uint16_t *user_len)
{
  cJSON *root = cJSON_CreateObject();

  // Add command and user_len (CORRECTED FIELD)
  cJSON_AddStringToObject(root, "command", "USER_DATA");
  cJSON_AddNumberToObject(root, "user_len", *user_len);

  cJSON *json_list = cJSON_CreateArray();
  cJSON_AddItemToObject(root, "user_data", json_list);

  for (uint16_t i = 0; i < *user_len; i++)
  {
    cJSON *item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "id", user_id[i]);
    cJSON_AddStringToObject(item, "name", user_name[i]);
    cJSON_AddNumberToObject(item, "face", face[i]);
    cJSON_AddNumberToObject(item, "finger", finger[i]);
    cJSON_AddStringToObject(item, "role", role[i]);
    cJSON_AddItemToArray(json_list, item);
  }

  // Assign to caller's pointer (CORRECTED)
  char *json_str = cJSON_PrintUnformatted(root);

  if (json_str == NULL)
  {
    cJSON_Delete(root);
    return NULL;
  }

  cJSON_Delete(root);
  return json_str;
}