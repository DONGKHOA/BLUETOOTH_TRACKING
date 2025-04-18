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

// Data Response
#define DATA_SYNC_SUCCESS (uint8_t)0x00
#define DATA_SYNC_FAIL    (uint8_t)0x01

// Data Response
#define DATA_SYNC_STATE_CONNECT     (uint8_t)0x00
#define DATA_SYNC_STATE_NOT_CONNECT (uint8_t)0x01

#define AUTHENTICATE_PASSWORD 123

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef enum
  {
    DATA_SYNC_DUMMY = 0x00,
    DATA_SYNC_ENROLL_FACE,
    DATA_SYNC_ENROLL_FINGERPRINT,
    DATA_SYNC_RESPONSE_ENROLL_FACE,
    DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT,
    DATA_SYNC_REQUEST_USER_DATA,
    DATA_SYNC_RESPONSE_USER_DATA,
    DATA_SYNC_RESPONSE_USERNAME_DATA,
    DATA_SYNC_REQUEST_AUTHENTICATION,
    DATA_SYNC_RESPONSE_AUTHENTICATION,
    DATA_SYNC_TIME,
    DATA_SYNC_STATE_CONNECTION_WIFI,
    DATA_SYNC_START_ATTENDANCE,
    DATA_SYNC_REQUEST_ATTENDANCE,
    DATA_SYNC_RESPONSE_ATTENDANCE,
    DATA_SYNC_RESPONSE_ENROLL_FOUND_FINGERPRINT,
    DATA_SYNC_DELETE_FACE_ID,
    DATA_SYNC_STOP_ATTENDANCE,
    DATA_SYNC_STATE_CONNECTION_BLE,
    DATA_SYNC_LAST,
    DATA_STOP_FRAME = 0xFF
  } __attribute__((packed)) Data_Frame;
  /**
   * @brief Data structure sync data between MCU1 and MCU2
   */

  typedef struct DATA_SYNC
  {
    uint8_t u8_data_packet[256];
    uint8_t u8_data_start;
    uint8_t u8_data_length;
    uint8_t u8_data_stop;
  } __attribute__((packed)) DATA_SYNC_t;

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_H_ */
