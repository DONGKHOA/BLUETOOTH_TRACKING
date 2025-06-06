/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "nvs_rw.h"

/******************************************************************************
 *      DEFINES
 *****************************************************************************/

#define TAG "NVS"

/******************************************************************************
 *   GLOBAL FUNCTIONS
 *****************************************************************************/

/**
 * The function `NVS_Init` initializes the NVS (Non-Volatile Storage) and
 * handles cases where the NVS partition needs to be erased and reinitialized.
 *
 * @return The function `NVS_Init()` is returning an `esp_err_t` type, which is
 * typically used in ESP32 development for error handling. In this case, the
 * function is returning `ESP_OK` if the initialization of the NVS (Non-Volatile
 * Storage) is successful.
 */
esp_err_t
NVS_Init (void)
{
  esp_err_t retVal = nvs_flash_init();
  if (retVal == ESP_ERR_NVS_NO_FREE_PAGES
      || retVal == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    retVal = nvs_flash_init();
  }
  return retVal;
}

/**
 * The function `NVS_WriteString` writes a string value to a non-volatile
 * storage (NVS) with error handling.
 *
 * @param name The `name` parameter in the `NVS_WriteString` function is used to
 * specify the namespace under which the key-value pair will be stored in the
 * Non-Volatile Storage (NVS) system. It helps in organizing and accessing data
 * within the NVS.
 * @param key The `key` parameter in the `NVS_WriteString` function is a unique
 * identifier for the data you want to store in the Non-Volatile Storage (NVS).
 * It is used to retrieve the stored data later when needed.
 * @param stringVal The `stringVal` parameter in the `NVS_WriteString` function
 * is the string value that you want to write to the Non-Volatile Storage (NVS)
 * under the specified key. This function opens an NVS handle, sets the string
 * value associated with the given key, commits the changes
 *
 * @return The function `NVS_WriteString` is returning an `esp_err_t` type,
 * which is the error code indicating the success or failure of the operation.
 */
esp_err_t
NVS_WriteString (const char *name, const char *key, const char *stringVal)
{
  nvs_handle_t nvsHandle;
  esp_err_t    retVal;

  retVal = nvs_open(name, NVS_READWRITE, &nvsHandle);
  if (retVal != ESP_OK)
  {
    ESP_LOGE(TAG,
             "Error (%s) opening NVS handle for Write",
             esp_err_to_name(retVal));
    goto exit_write;
  }
  else
  {
    printf("opening NVS Write handle Done \r\n");
    retVal = nvs_set_str(nvsHandle, key, stringVal);
    if (retVal != ESP_OK)
    {
      ESP_LOGE(TAG,
               "Error (%s) Can not write/set value: %s",
               esp_err_to_name(retVal),
               stringVal);
      goto exit_write;
    }

    retVal = nvs_commit(nvsHandle);
    if (retVal != ESP_OK)
    {
      ESP_LOGE(
          TAG, "Error (%s) Can not commit - write", esp_err_to_name(retVal));
      goto exit_write;
    }
    else
    {
      ESP_LOGI(TAG, "Write Commit Done!");
    }
  }

exit_write:

  nvs_close(nvsHandle);
  return retVal;
}

/**
 * The function `NVS_ReadString` reads a string value from non-volatile storage
 * using the ESP-IDF NVS API.
 *
 * @param name The `name` parameter in the `NVS_ReadString` function is used to
 * specify the namespace within the Non-Volatile Storage (NVS) where the data is
 * stored. It is a string that identifies the specific namespace to access the
 * stored data.
 * @param key The `key` parameter in the `NVS_ReadString` function is used to
 * specify the key associated with the data you want to read from the
 * Non-Volatile Storage (NVS). This key is used to uniquely identify the data
 * stored in the NVS. When calling `nvs_get_str
 * @param savedData The `savedData` parameter in the `NVS_ReadString` function
 * is a pointer to a character array where the retrieved string data will be
 * stored. The function will read a string value associated with the provided
 * `key` from the Non-Volatile Storage (NVS) and store it in the
 *
 * @return The function `NVS_ReadString` returns an `esp_err_t` value, which is
 * the error code indicating the success or failure of the operation.
 */
esp_err_t
NVS_ReadString (const char *name, const char *key, char *savedData, size_t len)
{
  nvs_handle_t nvsHandle;
  esp_err_t    retVal;

  ESP_LOGW(TAG, "Show Value-> name: %s, key: %s, len: %zu", name, key, len);

  retVal = nvs_open(name, NVS_READWRITE, &nvsHandle);
  if (retVal != ESP_OK)
  {
    ESP_LOGE(
        TAG, "Error (%s) opening NVS handle for Read", esp_err_to_name(retVal));
    goto exit_read;
  }

  size_t buffer_size = len; // Use a local size_t variable for buffer size
  retVal             = nvs_get_str(nvsHandle, key, savedData, &buffer_size);
  if (retVal == ESP_OK)
  {
    ESP_LOGW(TAG,
             "*****(%s) Successfully read value: %s",
             esp_err_to_name(retVal),
             savedData);
  }
  else
  {
    ESP_LOGE(TAG,
             "Error (%s) reading value for key: %s",
             esp_err_to_name(retVal),
             key);
    goto exit_read;
  }

  retVal = nvs_commit(nvsHandle);
  if (retVal != ESP_OK)
  {
    ESP_LOGE(
        TAG, "Error (%s) committing NVS after read", esp_err_to_name(retVal));
    goto exit_read;
  }
  ESP_LOGI(TAG, "Read Commit Done!");

exit_read:
  nvs_close(nvsHandle);
  return retVal;
}