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
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void DECODE_User_Data(char  *json_str,
                        int   *user_id,
                        char **user_name,
                        int    user_len);
  void DECODE_Command(char *json_string, char *command);
#ifdef __cplusplus
}
#endif

#endif /* HANDLE_DATA_H_ */