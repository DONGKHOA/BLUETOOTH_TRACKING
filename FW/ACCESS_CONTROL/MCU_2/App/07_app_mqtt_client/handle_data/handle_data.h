#ifndef HANDLE_DATA_H_
#define HANDLE_DATA_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *    PUBLIC DEFINES
   ***************************************************************************/

#define MAX_NAME_LEN 32

  /****************************************************************************
   *   PUBLIC TYPEDEF
   ***************************************************************************/

  typedef enum
  {
    USER_DATA_CMD,
    ATTENDANCE_DATA_CMD,
    ADD_USER_DATA_CMD,
    ENROLL_FACE_CMD,
    ENROLL_FINGER_CMD,
    ATTENDANCE_CMD,
    DELETE_USER_DATA_CMD,
    SET_ROLE,
    DELETE_FINGER_USER,
    DELETE_FACEID_USER,
    SYNCHRONIZE_CMD,
    UNKNOWN_CMD,
  } CommandType;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  CommandType DECODE_Command(char *json_string);
  void        DECODE_Status(char *json_str, int *status);
  void        DECODE_User_ID(char *json_str, int *user_id);
  void DECODE_Add_User_Data(char *json_str, int *user_id, char *user_name);
  void DECODE_Set_Role_Data(char *json_str, int *user_id, char *role);

  char *ENCODE_Sync_Data(int      *user_id,
                         int      *face,
                         int      *finger,
                         char    **role,
                         char    **user_name,
                         uint16_t *user_len);

  char *ENCODE_Attendance_Data(int      user_id,
                               char    *data_time,
                               uint8_t  number_checkin,
                               uint32_t index_packet);

#ifdef __cplusplus
}
#endif

#endif /* HANDLE_DATA_H_ */