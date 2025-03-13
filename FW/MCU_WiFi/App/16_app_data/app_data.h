#ifndef DATA_APP_DATA_ESP32_H_
#define DATA_APP_DATA_ESP32_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef enum
  {
    TAG_INDOOR,
    TAG_OUTDOOR,
    TAG_UNKNOWN
  } location_tag_t;

  /**
   * @brief Data structure holding data of system
   *
   *
   * @note s_data_mqtt_queue: Data receive queue (app_data_receive ->
   * app_mqtt_client)
   *
   */

  typedef struct _DATA_System_t
  {
    QueueHandle_t           s_data_mqtt_queue;
  } DATA_System_t;

  /**
   * @brief Structure defining the data sync between the app_ble_tracking and
   * app_data_transmit.
   */
  typedef struct
  {
    char          p_name[16];
    location_tag_t e_location_tag;
    int8_t         i8_filtered_rssi;
  } location_infor_tag_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
