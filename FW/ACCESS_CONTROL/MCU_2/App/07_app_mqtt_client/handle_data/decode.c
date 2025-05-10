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

#define TAG "DECODE"

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
    if (strcmp(cmd_item->valuestring, "SYN") == 0)
    {
      return SYNC_CMD;
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
    if (strcmp(cmd_item->valuestring, "ADD_USER") == 0)
    {
      return ADD_USER_DATA_CMD;
    }
  }

  cJSON_Delete(root);

  return UNKNOWN_CMD;
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

void
DECODE_Sync_Data (char *json_str, char *id_ac)
{
  cJSON *root = cJSON_Parse(json_str);

  if (!root)
  {
    ESP_LOGE(TAG, "Failed to parse JSON");
    return;
  }

  // Get "id_ac"
  cJSON *id_ac_item = cJSON_GetObjectItemCaseSensitive(root, "id");
  if (!cJSON_IsString(id_ac_item) || (id_ac_item->valuestring == NULL))
  {
    ESP_LOGE(TAG, "id_ac is missing or not a string");
    cJSON_Delete(root);
    return;
  }

  strncpy(id_ac, id_ac_item->valuestring, strlen(id_ac_item->valuestring));

  cJSON_Delete(root);
}

void
DECODE_Add_User_Data (char *json_str, int *user_id, char *user_name)
{
  cJSON *root = cJSON_Parse(json_str);

  cJSON *user_id_item   = cJSON_GetObjectItemCaseSensitive(root, "id");
  cJSON *user_name_item = cJSON_GetObjectItemCaseSensitive(root, "name");

  *user_id = user_id_item->valueint;
  strncpy(user_name, user_name_item->valuestring, MAX_NAME_LEN - 1);
  user_name[MAX_NAME_LEN - 1] = '\0'; // Ensure null-termination

  cJSON_Delete(root);
}

void
DECODE_Set_Role_Data (char *json_str, int *user_id, char *role)
{
  cJSON *root = cJSON_Parse(json_str);

  cJSON *user_id_item   = cJSON_GetObjectItemCaseSensitive(root, "id");
  cJSON *role_item = cJSON_GetObjectItemCaseSensitive(root, "role");

  *user_id = user_id_item->valueint;
  strncpy(role, role_item->valuestring, 5);
  role[5] = '\0'; // Ensure null-termination

  cJSON_Delete(root);
}