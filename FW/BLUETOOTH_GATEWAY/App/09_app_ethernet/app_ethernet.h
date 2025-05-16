#ifndef APP_ETHERNET_H_
#define APP_ETHERNET_H_

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_ETHERNET_CreateTask(void);
  void APP_ETHERNET_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_ETHERNET_H_ */