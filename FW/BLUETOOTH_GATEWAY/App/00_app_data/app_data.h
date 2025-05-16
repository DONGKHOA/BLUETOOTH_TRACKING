#ifndef DATA_APP_DATA_ESP32_H_
#define DATA_APP_DATA_ESP32_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

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

#define APP_CONFIGURATION_ENABLE  BIT2
#define APP_CONFIGURATION_DISABLE BIT3

#define MQTTSERVER_NVS "MQTTSERVER_NVS"
#define MQTTTOPIC_NVS  "MQTTTOPIC_NVS"

  /*** GPIO peripheral *******************************************************/

#define USER_BUTTON_1_PIN GPIO_NUM_36
#define USER_BUTTON_2_PIN GPIO_NUM_34
#define ROTARY_A_PIN      GPIO_NUM_39
#define ROTARY_B_PIN      GPIO_NUM_35
#define LED_STATUS_PIN    GPIO_NUM_5
#define RE_RS485_PIN      GPIO_NUM_25

  /*** I2C peripheral ********************************************************/

#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_27
#define I2C_SCL        GPIO_NUM_14
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

  /*** RMII peripheral *******************************************************/

#define RMII_MDC_PIN    GPIO_NUM_19
#define RMII_MDIO_PIN   GPIO_NUM_21
#define RMII_TXD0_PIN   GPIO_NUM_4
#define RMII_TXD1_PIN   GPIO_NUM_15
#define RMII_TX_EN_PIN  GPIO_NUM_18
#define RMII_RXD0_PIN   GPIO_NUM_23
#define RMII_RXD1_PIN   GPIO_NUM_13
#define RMII_CRS_DV_PIN GPIO_NUM_2

  /*** UART1 peripheral *******************************************************/

#define UART_RS485_TXD       GPIO_NUM_32
#define UART_RS485_RXD       GPIO_NUM_26
#define UART_RS485_NUM       UART_NUM_1
#define UART_RS485_BAUD_RATE 115200

  /*** state system **********************************************************/

#define STATUS_WIFI_CONNECTED    BIT0
#define STATUS_WIFI_DISCONNECTED BIT1
#define STATUS_MQTT_CONNECTED    BIT2
#define STATUS_MQTT_DISCONNECTED BIT3
#define STATUS_BLE_CONFIGURATION BIT4

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  /**
   * @brief Data structure holding data of system
   *
   *
   * @note s_rssi_ibeacon_queue: RSSI iBeacon data queue (app_ble_ibeacon ->
   * app_ble_tracking)
   *
   * @note s_location_tag_queue: Location tag data queue (app_ble_tracking ->
   * app_mqtt_client)
   *
   */
  typedef struct _DATA_System_t
  {
    QueueHandle_t      s_rssi_ibeacon_queue;
    QueueHandle_t      s_location_tag_queue;
    EventGroupHandle_t s_configuration_event;
  } DATA_System_t;

  /**
   * @brief Structure defining the data sync between the app_ble_ibeacon and
   * app_ble_tracking.
   */
  typedef struct
  {
    uint8_t u8_beacon_addr[6]; // 6 Bytes Address
    int8_t  i8_filtered_rssi;
  } ibeacon_infor_tag_t;

  /**
   * @brief Structure defining the data sync between the app_ble_tracking and
   * app_data_transmit.
   */
  typedef struct
  {
    uint8_t  u8_beacon_addr[6]; // 6 Bytes Address
    uint32_t u32_gateway_ID;
    char     c_gateway_version[6];
    int8_t   i8_filtered_rssi;
  } tracking_infor_tag_t;

  /****************************************************************************
   *   PUBLIC DATA
   ***************************************************************************/

  extern DATA_System_t s_data_system;

#ifdef __cplusplus
}
#endif

#endif /* DATA_APP_DATA_ESP32_H_ */
