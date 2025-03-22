#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

// Data Start Frame
#define DATA_SYNC_DETECT_FACE        (uint8_t)0x00
#define DATA_SYNC_DETECT_BUTTON_USER (uint8_t)0x01
#define DATA_SYNC_DETECT_FINGERPRINT (uint8_t)0x02

// Data Stop Frame
#define DATA_STOP_FRAME (uint8_t)0xFF

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/
  /**
   * @brief Data structure sync data between MCU1 and MCU2
   */

  typedef struct DATA_SYNC
  {
    uint8_t u8_data_packet[256];
    uint8_t u8_data_start;
    uint8_t u8_data_length;
    uint8_t u8_data_stop;
  }__attribute__((packed)) DATA_SYNC_t;

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_H_ */
