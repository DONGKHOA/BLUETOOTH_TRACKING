#ifndef JSON_GENERATE_H_
#define JSON_GENERATE_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct
  {
    char   mac[13]; // Device MAC address (string)
    int8_t rssi;    // RSSI value (integer)
  } dev_info_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  /**
   * @brief Create a JSON string with device information using cJSON.
   *
   * This function builds a JSON object with the following structure:
   * {
   *    "dev_id": <device ID string>,
   *    "dev_version": <firmware version string>,
   *    "dev_list": [
   *        {
   *            "mac": <MAC address string>,
   *            "rssi": <RSSI integer>,
   *            "ad": <advertising data string>,
   *            "ts": <timestamp integer>
   *        },
   *        ...  // more entries
   *    ],
   *   "dev_list_len": <number of entries>
   * }
   *
   * @param dev_id            Device identifier string.
   * @param dev_version       Device firmware/version string.
   * @param dev_list          Array of dev_info_t structs containing device list
   * entries.
   * @param dev_list_len      Number of elements in dev_list array.
   * @return char* Pointer to the generated JSON string (must be freed by the
   * caller), or NULL on failure.
   */

  char *create_device_json(char       *dev_id,
                           char       *dev_version,
                           dev_info_t *dev_list,
                           uint8_t     dev_list_len);

#ifdef __cplusplus
}
#endif

#endif /* JSON_GENERATE_H_ */