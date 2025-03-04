/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "modbusRTU.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define UART_TXD GPIO_NUM_4
#define UART_RXD GPIO_NUM_15
#define UART_RE  GPIO_NUM_12
#define UART_DE  GPIO_NUM_13

#define DATA_BIT    UART_DATA_8_BITS
#define PARITY_BIT  UART_PARITY_EVEN
#define HW_FLOWCTRL UART_HW_FLOWCTRL_DISABLE
#define STOP_BITS   UART_STOP_BITS_1

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static modbusRTU_request_t s_modbus_request;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

void ModbusRTU_Receive_Task(void *arg);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  // Config UART
  BSP_uartConfigIO(UART_PORT_NUM_2, UART_TXD, UART_RXD);
  BSP_uartConfigParity(UART_PARITY_EVEN);
  BSP_uartConfigDataLen(UART_DATA_8_BITS);
  BSP_uartConfigStopBits(UART_STOP_BITS_1);
  BSP_uartConfigBaudrate(UART_BAUDRATE_9600);
  BSP_uartConfigHWFlowCTRL(UART_HW_FLOWCTRL_DISABLE);
  BSP_uartDriverInit(UART_PORT_NUM_2);

  // Init RE and DE Pin
  BSP_gpioSetDirection(UART_RE, GPIO_MODE_OUTPUT, GPIO_PULL_DOWN);
  BSP_gpioSetDirection(UART_DE, GPIO_MODE_OUTPUT, GPIO_PULL_DOWN);
  DEV_RS485_ReceiveMode(UART_RE, UART_DE);

  // Config the message want to transmit
  MID_ModbusRTU_InfoMessage(
      &s_modbus_request, 0x01, MODBUSRTU_FUNC_READ_HOLDING_REGS, 0x0010, 1);

  // Create Task to receive Data from uart
  xTaskCreate(
      ModbusRTU_Receive_Task, "ModbusRTU_Receive_Task", 2048, NULL, 10, NULL);

  while (1)
  {
    // Send data
    // MID_ModbusRTU_SendRequest(UART_PORT_NUM_2, UART_RE, UART_DE, &s_modbus_request);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

void
ModbusRTU_Receive_Task (void *arg)
{
  while (1)
  {
    MID_ModbusRTU_ReceiveResponse(UART_PORT_NUM_2, &s_modbus_request);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}