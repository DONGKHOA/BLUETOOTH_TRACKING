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

#define TAG          "DECODE"
#define MAX_NAME_LEN 32

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

CommandType
DECODE_Command (char *json_string)
{
  cJSON *root = cJSON_Parse(json_string);

  cJSON *cmd_item = cJSON_GetObjectItemCaseSensitive(root, "command");
  if (cJSON_IsString(cmd_item) && (cmd_item->valuestring != NULL))
  {
    if (strcmp(cmd_item->valuestring, "USER_DATA") == 0)
    {
      return USER_DATA_CMD;
    }
    if (strcmp(cmd_item->valuestring, "ENROLL_FACE") == 0)
    {
      return ENROLL_FACE_CMD;
    }
    if (strcmp(cmd_item->valuestring, "ENROLL_FINGERPRINT") == 0)
    {
      return ENROLL_FINGER_CMD;
    }
    if (strcmp(cmd_item->valuestring, "ATTENDANCE") == 0)
    {
      return ATTENDANCE_CMD;
    }
    if (strcmp(cmd_item->valuestring, "DELETE_USER_DATA") == 0)
    {
      return DELETE_USER_DATA_CMD;
    }
    if (strcmp(cmd_item->valuestring, "SET_ROLE") == 0)
    {
      return SET_ROLE;
    }
    if (strcmp(cmd_item->valuestring, "DELETE_FINGER_USER") == 0)
    {
      return DELETE_FINGER_USER;
    }
    if (strcmp(cmd_item->valuestring, "DELETE_FACEID_USER") == 0)
    {
      return DELETE_FACEID_USER;
    }
  }

  cJSON_Delete(root);

  return UNKNOWN_CMD;
}

void
DECODE_User_Data (char     *json_str,
                  int      *user_id,
                  int      *face,
                  int      *finger,
                  char    **role,
                  char    **user_name,
                  uint16_t *user_len)
{
  cJSON *root = cJSON_Parse(json_str);

  if (!root)
  {
    ESP_LOGE(TAG, "Failed to parse JSON");
    return;
  }

  // Get "user_len"
  cJSON *user_len_item = cJSON_GetObjectItemCaseSensitive(root, "user_len");
  if (!cJSON_IsNumber(user_len_item))
  {
    ESP_LOGE(TAG, "user_len is missing or not a number");
    cJSON_Delete(root);
    return;
  }

  *user_len = (uint16_t)user_len_item->valueint;

  // Get "user_data" array
  cJSON *user_data = cJSON_GetObjectItemCaseSensitive(root, "user_data");
  if (!cJSON_IsArray(user_data))
  {
    ESP_LOGE(TAG, "user_data is missing or not an array");
    cJSON_Delete(root);
    return;
  }

  for (int i = 0; i < *user_len; i++)
  {
    cJSON *user = cJSON_GetArrayItem(user_data, i);
    if (!cJSON_IsObject(user))
    {
      ESP_LOGE(TAG, "User %d is not an object", i);
      continue;
    }

    // Extract fields
    cJSON *id_item     = cJSON_GetObjectItemCaseSensitive(user, "id");
    cJSON *name_item   = cJSON_GetObjectItemCaseSensitive(user, "name");
    cJSON *face_item   = cJSON_GetObjectItemCaseSensitive(user, "face");
    cJSON *finger_item = cJSON_GetObjectItemCaseSensitive(user, "finger");
    cJSON *role_item   = cJSON_GetObjectItemCaseSensitive(user, "role");

    // Copy user ID
    user_id[i] = id_item->valueint;

    // Allocate memory and copy user name
    user_name[i] = (char *)heap_caps_malloc(MAX_NAME_LEN, MALLOC_CAP_SPIRAM);

    strncpy(user_name[i], name_item->valuestring, MAX_NAME_LEN - 1);
    user_name[i][MAX_NAME_LEN - 1] = '\0'; // Ensure null-termination

    face[i]   = face_item->valueint;
    finger[i] = finger_item->valueint;

    // Allocate memory and copy role
    role[i] = (char *)heap_caps_malloc(6, MALLOC_CAP_SPIRAM);
    strncpy(role[i], role_item->valuestring, 5);
    role[i][5] = '\0'; // Ensure null-termination
  }

  cJSON_Delete(root);
}

void
DECODE_Status (char *json_str, int *status)
{
  cJSON *root = cJSON_Parse(json_str);

  if (!root)
  {
    ESP_LOGE(TAG, "Failed to parse JSON");
    return;
  }

  cJSON *response_item = cJSON_GetObjectItemCaseSensitive(root, "response");

  if (cJSON_IsString(response_item) && (response_item->valuestring != NULL))
  {
    if (strcmp(response_item->valuestring, "success") == 0)
    {
      *status = 1;
    }
    else if (strcmp(response_item->valuestring, "fail") == 0)
    {
      *status = 0;
    }
    else
    {
      ESP_LOGW(TAG, "Unknown response value: %s", response_item->valuestring);
      *status = -1; // Unknown response
    }
  }
  else
  {
    ESP_LOGE(TAG, "response field is missing or not a string");
    *status = -1;
  }

  cJSON_Delete(root);
}

void
DECODE_User_ID (char *json_str, int *user_id)
{
  cJSON *root = cJSON_Parse(json_str);

  if (!root)
  {
    ESP_LOGE(TAG, "Failed to parse JSON");
    return;
  }

  // Get "id"
  cJSON *user_id_item = cJSON_GetObjectItemCaseSensitive(root, "id");
  if (!cJSON_IsNumber(user_id_item))
  {
    ESP_LOGE(TAG, "id is missing or not a number");
    cJSON_Delete(root);
    return;
  }

  *user_id = user_id_item->valueint;

  cJSON_Delete(root);
}
