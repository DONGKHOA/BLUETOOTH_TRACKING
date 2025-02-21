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

#define BUF_SIZE 256

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/
static modbusRTU_request_t request;
static QueueHandle_t       uart_event_queue;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/
void IRAM_ATTR ModbusRTU_UART_EventHandler(void *arg);
void           ModbusRTU_EventTask(void *arg);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  uart_event_queue = xQueueCreate(10, sizeof(uart_event_t));

  BSP_uartConfigIO(UART_PORT_NUM_2, UART_TXD, UART_RXD);
  BSP_uartConfigParity(UART_PARITY_EVEN);
  BSP_uartConfigDataLen(UART_DATA_8_BITS);
  BSP_uartConfigStopBits(UART_STOP_BITS_1);
  BSP_uartConfigBaudrate(UART_BAUDRATE_9600);
  BSP_uartConfigHWFlowCTRL(UART_HW_FLOWCTRL_DISABLE);

  BSP_uartDriverInit(UART_PORT_NUM_2);

  BSP_uartIntrInit(UART_RXD, GPIO_INTR_POSEDGE, ModbusRTU_UART_EventHandler);

  DEV_RS485_ReceiveMode(UART_RE, UART_DE);
  // MID_ModbusRTU_InfoMessage(
  //     &request, 0x01, MODBUSRTU_FUNC_READ_HOLDING_REGS, 0x0010, 1);

  xTaskCreate(ModbusRTU_EventTask, "ModbusRTU_EventTask", 2048, NULL, 10, NULL);

  while (1)
  {
    MID_ModbusRTU_SendRequest(UART_PORT_NUM_2, UART_RE, UART_DE, &request);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/
void IRAM_ATTR
ModbusRTU_UART_EventHandler (void *arg)
{
  uart_event_t event;
  event.type = UART_DATA;
  printf("EventHandler\n");
  xQueueSendFromISR(uart_event_queue, &event, NULL);
}

void
ModbusRTU_EventTask (void *arg)
{
  // uart_event_t event;
  // uint8_t      data[RX_BUF_SIZE];

  // while (1)
  // {
  //   if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY))
  //   {
  //     printf("ISR\n");
  //     if (event.type == UART_DATA)
  //     {
  //     }
  //   }
  // }

  uart_event_t event;
  uint8_t      data[RX_BUF_SIZE];

  while (1)
  {
    if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY))
    {
      if (event.type == UART_DATA)
      {
        int len = uart_read_bytes(
            UART_PORT_NUM_2, data, sizeof(data), pdMS_TO_TICKS(10));
        if (len > 0)
        {
          printf("Received: ");
          for (int i = 0; i < len; i++)
          {
            printf("%02X ", data[i]);
          }
          printf("\n");
        }
      }
    }
  }
}
