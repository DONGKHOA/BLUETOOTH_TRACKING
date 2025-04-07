#ifndef DATA_APP_DATA_H_
#define DATA_APP_DATA_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

#define APP_CONFIGURATION_ENABLE  BIT2
#define APP_CONFIGURATION_DISABLE BIT3

#define MQTTSERVER_NVS "MQTTSERVER_NVS"
#define MQTTTOPIC_NVS  "MQTTTOPIC_NVS"

  /*** CAN peripheral ********************************************************/

#define CAN_MODE          TWAI_MODE_NORMAL
#define CAN_TXD_PIN       GPIO_NUM_42
#define CAN_RXD_PIN       GPIO_NUM_41
#define CAN_TXD_QUEUE_LEN 8
#define CAN_RXD_QUEUE_LEN 8
#define CAN_INTR_FLAG     ESP_INTR_FLAG_LEVEL3 // lowest priority
#define CAN_BITRATE       5

  /*** I2C peripheral ********************************************************/

#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_39
#define I2C_SCL        GPIO_NUM_40
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

#define TIME_SOURCE_SNTP_READY BIT0
#define TIME_SOURCE_RTC_READY  BIT1

  /*** GPIO peripheral *******************************************************/

#define BUTTON_USER_PIN GPIO_NUM_45
#define LED_STATUS_PIN  GPIO_NUM_1

#define LOCAL_DATABASE_NUMBER_OF_USER_DATA         0xFE
#define LOCAL_DATABASE_DETAIL_OF_USER_DATA         0xFD
#define LOCAL_DATABASE_RESPONSE_ENROLL_FACE        0xFC
#define LOCAL_DATABASE_RESPONSE_ENROLL_FINGERPRINT 0xFB
#define LOCAL_DATABASE_RESPONSE_DELETE_USER_DATA   0xFA

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  /**
   * @brief Data structure holding data of system
   *
   *
   * @note s_data_mqtt_queue: Data receive queue (app_data_receive ->
   * app_mqtt_client)
   *
   */

  typedef struct _DATA_System_t
  {
    QueueHandle_t      s_data_mqtt_queue;
    QueueHandle_t      s_send_data_queue;
    QueueHandle_t      s_data_local_database_queue;
    EventGroupHandle_t s_flag_time_event;
    EventGroupHandle_t s_configuration_event;
  } DATA_System_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_H_ */
