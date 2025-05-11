#ifndef APP_ETHERNET_H_
#define APP_ETHERNET_H_

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

  void APP_ETHERNET_CreateTask(void);
  void APP_ETHERNET_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_ETHERNET_H_ */