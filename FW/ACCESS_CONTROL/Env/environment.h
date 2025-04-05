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
#define DATA_SYNC_ENROLL_FACE                 (uint8_t)0x01
#define DATA_SYNC_ENROLL_FINGERPRINT          (uint8_t)0x02
#define DATA_SYNC_RESPONSE_ENROLL_FACE        (uint8_t)0x03
#define DATA_SYNC_RESPONSE_ENROLL_FINGERPRINT (uint8_t)0x04
#define DATA_SYNC_REQUEST_USER_DATA           (uint8_t)0x05
#define DATA_SYNC_NUMBER_OF_USER_DATA         (uint8_t)0x06
#define DATA_SYNC_DETAIL_OF_USER_DATA         (uint8_t)0x07
#define DATA_SYNC_REQUEST_AUTHENTICATION      (uint8_t)0x08
#define DATA_SYNC_RESPONSE_AUTHENTICATION     (uint8_t)0x09
#define DATA_SYNC_TIME                        (uint8_t)0x0A
#define DATA_SYNC_STATE_CONNECTION            (uint8_t)0x0B
#define DATA_SYNC_REQUEST_ATTENDANCE          (uint8_t)0x0C
#define DATA_SYNC_RESPONSE_ATTENDANCE         (uint8_t)0x0D
#define DATA_SYNC_REQUEST_DELETE_USER_DATA    (uint8_t)0x0E

// Data Stop Frame
#define DATA_STOP_FRAME (uint8_t)0xFF

// Data Response
#define DATA_SYNC_SUCCESS (uint8_t)0x00
#define DATA_SYNC_FAIL    (uint8_t)0x01

// Data Response
#define DATA_SYNC_STATE_CONNECT     (uint8_t)0x00
#define DATA_SYNC_STATE_NOT_CONNECT (uint8_t)0x01

#define DATA_SYNC_DUMMY (uint8_t)0x00

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
  } __attribute__((packed)) DATA_SYNC_t;

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_H_ */
