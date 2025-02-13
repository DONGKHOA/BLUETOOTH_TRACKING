#ifndef APP_BLE_IBEACON_H_
#define APP_BLE_IBEACON_H_

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
    uint8_t  flags[3];
    uint8_t  length;
    uint8_t  type;
    uint16_t company_id;
    uint16_t beacon_type;
  } __attribute__((packed)) esp_ble_ibeacon_head_t;

  typedef struct
  {
    esp_ble_ibeacon_head_t ibeacon_head;
  } __attribute__((packed)) esp_ble_ibeacon_t;
       
  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_BLE_IBEACON_CreateTask(void);
  void APP_BLE_IBEACON_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_IBEACON_H_ */