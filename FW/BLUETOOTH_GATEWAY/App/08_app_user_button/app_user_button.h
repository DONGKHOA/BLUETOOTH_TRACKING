#ifndef APP_USER_BUTTON_H_
#define APP_USER_BUTTON_H_

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *    PUBLIC DEFINES
   ***************************************************************************/

#define BUTTON_Short_Pressing_BIT BIT0
#define BUTTON_Long_Pressing_BIT  BIT1

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_USER_BUTTON_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_USER_BUTTON_H_ */