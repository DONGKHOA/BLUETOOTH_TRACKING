#ifndef APP_HANDLE_CAMERA_H_
#define APP_HANDLE_CAMERA_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

#define CAM_PIN_PWDN  -1 // power down is not used
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK  10
#define CAM_PIN_SIOD  3
#define CAM_PIN_SIOC  20

#define CAM_PIN_D7    9  // Y9
#define CAM_PIN_D6    11 // Y8
#define CAM_PIN_D5    12 // Y7
#define CAM_PIN_D4    17 // Y6
#define CAM_PIN_D3    15 // Y5
#define CAM_PIN_D2    6  // Y4
#define CAM_PIN_D1    7  // Y3
#define CAM_PIN_D0    16 // Y2
#define CAM_PIN_VSYNC 8
#define CAM_PIN_HREF  18
#define CAM_PIN_PCLK  13

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void APP_HANDLE_CAMERA_CreateTask(void);
  void APP_HANDLE_CAMERA_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_HANDLE_CAMERA_H_ */