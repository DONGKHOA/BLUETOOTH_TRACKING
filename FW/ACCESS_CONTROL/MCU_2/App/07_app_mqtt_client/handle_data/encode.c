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

void
ENCODE_User_Data (char     *json_str,
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

  cJSON_AddStringToObject(root, "command", "USER_DATA");

  json_str = cJSON_PrintUnformatted(root);
  if (json_str == NULL)
  {
    cJSON_Delete(root);
    return;
  }

  cJSON_Delete(root);
}