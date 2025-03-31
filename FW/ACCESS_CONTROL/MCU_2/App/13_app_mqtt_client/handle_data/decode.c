/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "cJSON.h"
#include "esp_log.h"

#include <string.h>

#include "handle_data.h"

#define TAG "DECODE"

void
DECODE_User_Data (char *json_str, int *user_id, char **user_name, int user_len)
{
  cJSON *root = cJSON_Parse(json_str);

  // Get "user_len"
  cJSON *user_len_item = cJSON_GetObjectItemCaseSensitive(root, "user_len");
  if (!cJSON_IsNumber(user_len_item))
  {
    ESP_LOGE(TAG, "user_len is missing or not a number");
    cJSON_Delete(root);
    return;
  }
  user_len = user_len_item->valueint;

  // Get "user_data" array
  cJSON *user_data = cJSON_GetObjectItemCaseSensitive(root, "user_data");

  for (int i = 0; i < user_len; i++)
  {
    cJSON *user = cJSON_GetArrayItem(user_data, i);
    if (!cJSON_IsObject(user))
    {
      ESP_LOGE(TAG, "User %d is not an object", i);
      continue;
    }

    // Extract user fields
    cJSON *id_item     = cJSON_GetObjectItemCaseSensitive(user, "id");
    cJSON *name_item   = cJSON_GetObjectItemCaseSensitive(user, "name");
    cJSON *face_item   = cJSON_GetObjectItemCaseSensitive(user, "face");
    cJSON *finger_item = cJSON_GetObjectItemCaseSensitive(user, "finger");

    // Validate data types
    if (!cJSON_IsNumber(id_item) || !cJSON_IsString(name_item)
         || !cJSON_IsNumber(face_item)
        || !cJSON_IsNumber(finger_item))
    {
      ESP_LOGE(TAG, "Invalid data types in user %d", i);
      continue;
    }

    // Access values
    user_id[i]   = id_item->valueint;
    strcpy(*user_name[i], name_item->valuestring);
    // *user_name[i]   = name_item->valuestring;
    // int         face   = face_item->valueint;
    // int         finger = finger_item->valueint;

    // // Use the extracted data (example: log to console)
    // ESP_LOGI(TAG,
    //          "User %d: ID=%d, Name=%s, Face=%d, Finger=%d",
    //          i,
    //          id,
    //          name,
    //          face,
    //          finger);
  }
}