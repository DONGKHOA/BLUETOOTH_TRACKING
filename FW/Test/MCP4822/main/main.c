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

#include "mcp4822.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/
#define HSPI_HOST SPI3_HOST
#define MISO_PIN  -1
#define MOSI_PIN  GPIO_NUM_19
#define SCLK_PIN  GPIO_NUM_20
#define CS_PIN    GPIO_NUM_21
#define LDAC_PIN  GPIO_NUM_47

#define CLOCK_SPEED_HZ  40000000
#define MAX_TRANSFER_SZ 4096
#define DMA_CHANNEL     3
#define SPI_MODE        0
#define QUEUE_SIZE      50
/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

spi_device_handle_t spi_mcp4822_handle;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void TestMain_MCP4822_Task(void *pvParameter);
static void TestMain_MCP4822_Init(void);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  TestMain_MCP4822_Init();
  xTaskCreate(TestMain_MCP4822_Task, "MCP4822_Task", 4096, NULL, 8, NULL);
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
TestMain_MCP4822_Init (void)
{
  BSP_spiConfigDefault();
  BSP_spiConfigMode(SPI_MODE);
  BSP_spiConfigIO(MISO_PIN, MOSI_PIN, SCLK_PIN);
  BSP_spiConfigCS(CS_PIN);
  BSP_spiMaxTransferSize(MAX_TRANSFER_SZ);
  BSP_spiClockSpeed(CLOCK_SPEED_HZ);
  BSP_spiTransactionQueueSize(QUEUE_SIZE);

  BSP_spiDriverInit(&spi_mcp4822_handle, HSPI_HOST);
  DEV_MPC4822_EnableOutput(LDAC_PIN);
}

static void
TestMain_MCP4822_Task (void *pvParameter)
{
  DEV_MCP4822_SetValue(
      spi_mcp4822_handle, MCP4822_DAC_A, MCP4822_OUTPUT_GAIN_x2, CS_PIN, 2000);
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
