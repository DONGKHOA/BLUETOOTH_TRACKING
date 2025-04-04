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
   *   PUBLIC TYPEDEF
   ***************************************************************************/
  typedef enum
  {
    USER_DATA_CMD,
    AUTHENTICATE_CMD,
    ENROLL_FACE_CMD,
    ENROLL_FINGER_CMD,
    ATTENDANCE_CMD,
    DELETE_USER_DATA_CMD,
    UNKNOWN_CMD
  } CommandType;
  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  CommandType DECODE_Command(char *json_string);
  void        DECODE_User_Data(char     *json_str,
                               int      *user_id,
                               char      user_name[][32],
                               uint16_t *user_len);
  void        DECODE_Status(char *json_str, int *status);
  void        DECODE_User_ID(char *json_str, int *user_id);
#ifdef __cplusplus
}
#endif

#endif /* HANDLE_DATA_H_ */