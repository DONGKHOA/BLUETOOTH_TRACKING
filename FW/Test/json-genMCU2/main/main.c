#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "cJSON.h"

#define TAG "JSON_GENERATE"

char     json_str[10240];
int      user_id[2]   = { 1, 2 };
int      face[2]      = { 1, 0 };
int      finger[2]    = { 1, 0 };
char    *role[2]      = { "admin", "user" };
char    *user_name[2] = { "John Doe", "Jane Smith" };
uint16_t user_len     = 2;

void
ENCODE_User_Data (char     *json_str,
                  int      *user_id,
                  int      *face,
                  int      *finger,
                  char    **role,
                  char    **user_name,
                  uint16_t *user_len)
{
  cJSON *root = cJSON_CreateObject();

  cJSON_AddStringToObject(root, "command", "USER_DATA");
  cJSON_AddNumberToObject(root, "user_id", *user_id);

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

  json_str = cJSON_PrintUnformatted(root);
  if (json_str == NULL)
  {
    cJSON_Delete(root);
    return;
  }

  cJSON_Delete(root);
}

void
app_main ()
{
  ENCODE_User_Data(json_str, user_id, face, finger, role, user_name, &user_len);
  printf("%s\n", json_str);
}