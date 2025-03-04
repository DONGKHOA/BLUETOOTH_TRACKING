#ifndef DATA_APP_DATA_ESP32_H_
#define DATA_APP_DATA_ESP32_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct _DATA_READ_DataTypedef
  {
    uint8_t       u8_ssid[32];
    uint8_t       u8_pass[32];
    QueueHandle_t s_data_mqtt_queue; // float
  } DATA_READ_DataTypedef;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_READ_DataTypedef s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
