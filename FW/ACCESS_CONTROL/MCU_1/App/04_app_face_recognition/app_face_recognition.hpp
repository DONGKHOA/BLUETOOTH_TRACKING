#ifndef APP_FACE_RECOGNITION_H_
#define APP_FACE_RECOGNITION_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <list>
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "face_recognition_112_v1_s8.hpp"
#include "face_recognition_112_v1_s16.hpp"

/****************************************************************************
 *   PUBLIC TYPEDEFS
 ***************************************************************************/

class Face
{
  private:
  QueueHandle_t          *p_camera_recognition_queue;
  QueueHandle_t          *p_result_recognition_queue;
  EventGroupHandle_t     *p_display_event;
  HumanFaceDetectMSR01    detector;
  HumanFaceDetectMNP01    detector2;
  FaceRecognition112V1S8 *recognizer;
  face_info_t             recognize_result;

  static void APP_FACE_RECOGNITION_Task(void *pvParameters);

  public:
  Face();
  ~Face();

  void CreateTask();
};

#endif /* APP_FACE_RECOGNITION_H_ */