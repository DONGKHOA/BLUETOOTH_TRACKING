#ifndef DATA_APP_DATA_H_
#define DATA_APP_DATA_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"

#include "environment.h"

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
#define ROOM_NVS       "ROOM_NVS"

#define TIME_SOURCE_SNTP_READY BIT0
#define TIME_SOURCE_RTC_READY  BIT1
#define TIME_SOURCE_RTC_OFF    BIT2

#define EVENT_ENROLL_FINGERPRINT     BIT0
#define EVENT_ATTENDANCE_FINGERPRINT BIT1
#define EVENT_DELETE_FINGERPRINT     BIT2

#define MAX_USER_DATA 10240 // Maximum user data size

#define LOCAL_DATABASE_SUCCESS (uint8_t)0x00
#define LOCAL_DATABASE_FAIL    (uint8_t)0x01

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

  /*** GPIO peripheral *******************************************************/

#define BUTTON_USER_PIN GPIO_NUM_45
#define LED_STATUS_PIN  GPIO_NUM_1
#define DAC_RDY         GPIO_NUM_38
#define DHT22_PIN       GPIO_NUM_9

  /*** SPI2 peripheral *******************************************************/

#define SPI3_MISO_PIN       GPIO_NUM_11
#define SPI3_MOSI_PIN       GPIO_NUM_13
#define SPI3_SCLK_PIN       GPIO_NUM_12
#define SPI3_CS_SD_CARD_PIN GPIO_NUM_48
#define SPI3_CS_DAC_PIN     GPIO_NUM_14

#define SPI3_CLOCK_SPEED_HZ          SPI_CLOCK_400KHz
#define SPI3_SPI_BUS_MAX_TRANSFER_SZ 4096
#define SPI3_DMA_CHANNEL             SPI_DMA_CH_AUTO
#define SPI3_SPI_MODE                0
#define SPI3_QUEUE_SIZE              1

  /*** UART2 peripheral *******************************************************/

#define UART_FINGERPRINT_TXD       GPIO_NUM_15
#define UART_FINGERPRINT_RXD       GPIO_NUM_16
#define UART_FINGERPRINT_NUM       UART_NUM_2
#define UART_FINGERPRINT_BAUD_RATE 115200

  /*** UART1 peripheral *******************************************************/

#define UART_RS485_TXD       GPIO_NUM_17
#define UART_RS485_RXD       GPIO_NUM_18
#define UART_RS485_NUM       UART_NUM_1
#define UART_RS485_BAUD_RATE 115200

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef enum
  {
    LOCAL_DATABASE_FIRST = DATA_SYNC_LAST + 1,
    LOCAL_DATABASE_REQUEST_DELETE_USER_DATA,
    LOCAL_DATABASE_RESPONSE_DELETE_USER_DATA,
    LOCAL_DATABASE_DATA,
    LOCAL_DATABASE_DATA_ATTENDANCE,
    LOCAL_DATABASE_SET_ROLE,
    LOCAL_DATABASE_FINGER_DELETE,
    LOCAL_DATABASE_FACEID_DELETE,
    LOCAL_DATABASE_LAST
  } __attribute__((packed)) local_database_status_t;

  typedef enum
  {
    FINGER_ENROLL = 0x00,
    FINGER_ATTENDANCE,
    FINGER_DELETE
  } __attribute__((packed)) finger_cmd_t;

  typedef enum
  {
    STATE_WIFI_CONNECTED = 0x00,
    STATE_WIFI_DISCONNECTED,
    STATE_BLUETOOTH_CONFIG
  } __attribute__((packed)) state_system_t;

  typedef enum
  {
    SDCARD_ENROLL_FACE = 0x00,
    SDCARD_ENROLL_FINGERPRINT,
    SDCARD_ATTENDANCE,
    SDCARD_SYNC_DATA_SERVER,
    SDCARD_ATTENDANCE_DATA,
    SDCARD_ADD_USER_DATA,
    SDCARD_DELETE_USER_DATA,
    SDCARD_SET_ROLE,
    SDCARD_DELETE_FINGER_USER,
    SDCARD_DELETE_FACEID_USER,  
    SDCARD_UNKNOWN_CMD,
  } __attribute__((packed)) sdcard_cmd_t;

  typedef struct app_data
  {
    uint32_t u32_time;
    uint16_t u16_user_id;
    char     user_name[32];
    char     role[6];
  } sdcard_data_t; // Data structure for sdcard, local database and mqtt

  typedef struct attendance_data
  {
    uint32_t u32_index_packet;
    uint32_t u32_time[16];
    uint16_t u16_user_id;
    uint8_t  u8_number_checkin;
  } attendance_data_t;

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
    QueueHandle_t      s_data_sdcard_queue;
    QueueHandle_t      s_data_attendance_queue;
    SemaphoreHandle_t  s_spi_mutex;
    SemaphoreHandle_t  s_i2c_mutex;
    SemaphoreHandle_t  s_time_mutex;
    EventGroupHandle_t s_flag_time_event;
    EventGroupHandle_t s_configuration_event;
    EventGroupHandle_t s_fingerprint_event;
    state_system_t     s_state_system;
  } DATA_System_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

  extern bool b_valid_sdcard;

  extern char       **user_name;
  extern int         *user_id;
  extern int         *face;
  extern int         *finger;
  extern char       **role;
  extern uint16_t     user_len;
  extern portMUX_TYPE spi_mux;
  extern portMUX_TYPE i2c_mux;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_H_ */
