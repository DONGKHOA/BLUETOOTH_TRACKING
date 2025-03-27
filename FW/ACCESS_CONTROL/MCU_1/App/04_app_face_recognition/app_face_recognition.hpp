#ifndef APP_FACE_RECOGNITION_H_
#define APP_FACE_RECOGNITION_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "sdkconfig.h"

#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "face_recognition_112_v1_s8.hpp"
#include "face_recognition_112_v1_s16.hpp"

#include "app_data.h"

/****************************************************************************
 *   PUBLIC TYPEDEFS
 ***************************************************************************/

class Face
{
  private:
  QueueHandle_t          *p_camera_recognition_queue;
  QueueHandle_t *p_result_recognition_queue;
  HumanFaceDetectMSR01    detector;
  HumanFaceDetectMNP01    detector2;
  FaceRecognition112V1S8 *recognizer;

  static void APP_FACE_RECOGNITION_Task(void *pvParameters);

  public:
  Face();
  ~Face();

  void CreateTask();
};

#endif /* APP_FACE_RECOGNITION_H_ */