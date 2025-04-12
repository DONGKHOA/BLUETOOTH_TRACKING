#ifndef DATA_APP_DATA_ESP32_H_
#define DATA_APP_DATA_ESP32_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *    PUBLIC DEFINES
   ***************************************************************************/

  /*** CAN peripheral ********************************************************/

#define CAN_MODE          TWAI_MODE_NORMAL
#define CAN_TXD_PIN       GPIO_NUM_21
#define CAN_RXD_PIN       GPIO_NUM_14
#define CAN_TXD_QUEUE_LEN 8
#define CAN_RXD_QUEUE_LEN 8
#define CAN_INTR_FLAG     ESP_INTR_FLAG_LEVEL3 // lowest priority
#define CAN_BITRATE       CAN_500KBITS

  /*** SPI2 peripheral *******************************************************/

#define SPI2_MISO_PIN GPIO_NUM_NC
#define SPI2_MOSI_PIN GPIO_NUM_47
#define SPI2_SCLK_PIN GPIO_NUM_48
#define SPI2_CS_PIN   GPIO_NUM_46

#define SPI2_CLOCK_SPEED_HZ          SPI_CLOCK_40MHz
#define SPI2_SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)
#define SPI2_DMA_CHANNEL             SPI_DMA_CH_AUTO
#define SPI2_SPI_MODE                0
#define SPI2_QUEUE_SIZE              50

  /*** SPI3 peripheral *******************************************************/

#define SPI3_MISO_PIN GPIO_NUM_40
#define SPI3_MOSI_PIN GPIO_NUM_38
#define SPI3_SCLK_PIN GPIO_NUM_45
#define SPI3_CS_PIN   GPIO_NUM_41

#define SPI3_CLOCK_SPEED_HZ          SPI_CLOCK_100KHz
#define SPI3_SPI_BUS_MAX_TRANSFER_SZ 0
#define SPI3_DMA_CHANNEL             SPI_DMA_CH_AUTO
#define SPI3_SPI_MODE                0
#define SPI3_QUEUE_SIZE              1

  /*** ILI9341 device ********************************************************/

#define ILI9341_DC_PIN  GPIO_NUM_39
#define ILI9341_RST_PIN GPIO_NUM_5

  /*** XPT2046 device *********************************************************/

#define XPT2046_IRQ_PIN GPIO_NUM_42

#define ATTENDANCE_BIT         BIT0
#define ENROLL_FACE_ID_BIT     BIT1
#define ENROLL_FINGERPRINT_BIT BIT2
#define DELETE_FACE_ID_BIT     BIT3
#define DELETE_FINGERPRINT_BIT BIT4
#define AUTHENTICATION_BIT     BIT5
#define DATA_LIST_BIT          BIT6

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct
  {
    struct coord_box_face
    {
      uint16_t x1;
      uint16_t x2;
      uint16_t y1;
      uint16_t y2;
    } s_coord_box_face;
    struct left_eye
    {
      uint16_t x;
      uint16_t y;
    } s_left_eye;
    struct right_eye
    {
      uint16_t x;
      uint16_t y;
    } s_right_eye;
    struct left_mouth
    {
      uint16_t x;
      uint16_t y;
    } s_left_mouth;
    struct right_mouth
    {
      uint16_t x;
      uint16_t y;
    } s_right_mouth;
    struct nose
    {
      uint16_t x;
      uint16_t y;
    } s_nose;
    int ID;
  } data_result_recognition_t;

  typedef struct
  {
    uint8_t *u8_buff;
    size_t   width;
    size_t   height;
  } camera_capture_t;

  /**
   * @brief Data structure holding data of system
   *
   * @note s_camera_capture_queue: Camera control command queue
   * (app_handle_camera -> app_gui)
   *
   * @note s_camera_recognition_queue: Face recognition result queue
   * (app_face_recognition -> app_gui)
   *
   * @note s_send_data_queue: Data transmission queue (app_gui ->
   * app_data_transmit)
   *
   */
  typedef struct _DATA_System_t
  {
    QueueHandle_t      s_camera_capture_queue;
    QueueHandle_t      s_camera_recognition_queue;
    QueueHandle_t      s_result_recognition_queue;
    QueueHandle_t      s_send_data_queue;
    QueueHandle_t      s_receive_data_event_queue;
    EventGroupHandle_t s_display_event;
  } DATA_System_t;

  typedef struct
  {
    uint8_t u8_minute;
    uint8_t u8_hour;
    uint8_t u8_day;
    uint8_t u8_month;
    uint8_t u8_year;
  } time_data_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
