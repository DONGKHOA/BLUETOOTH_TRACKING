/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

/*** standard ****************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdint.h>

/*** freeRTOS ****************************************************************/

/*** esp - idf ***************************************************************/

#include "driver/spi_master.h"
#include "esp_camera.h"

/*** bsp *********************************************************************/

#include "can.h"
#include "spi.h"
#include "gpio.h"
#include "nvs_rw.h"

/*** device ******************************************************************/

#include "sn65hvd230dr.h"
#include "ili9341.h"
#include "xpt2046.h"

/*** application *************************************************************/

#include "app_data.h"
#include "app_display.h"
#include "app_data_transmit.h"
#include "app_data_receive.h"
#include "app_handle_camera.h"
#include "app_face_recognition.hpp"

#include "environment.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_MAIN"

/******************************************************************************
 *   PUBLIC DATA
 *****************************************************************************/

ili9341_handle_t s_ili9341_0;
xpt2046_handle_t s_xpt2046_0;
Face            *p_face;

/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

static spi_device_handle_t spi_ili9341_handle;
static spi_device_handle_t spi_xpt2046_handle;

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static inline void APP_MAIN_InitGPIO(void);
static inline void APP_MAIN_InitCAN(void);
static inline void APP_MAIN_InitSPI(void);
static inline void APP_MAIN_InitDataSystem(void);

/******************************************************************************
 *       MAIN FUNCTION
 *****************************************************************************/

extern "C" void
app_main (void)
{
  // BSP Initialization

  APP_MAIN_InitGPIO();
  APP_MAIN_InitSPI();
  APP_MAIN_InitCAN();

  // Main Initialization data system

  APP_MAIN_InitDataSystem();

  // App Initialization

  APP_DISPLAY_Init();
  APP_DATA_RECEIVE_Init();
  // APP_HANDLE_CAMERA_Init();
  // p_face = new Face();
  APP_DATA_TRANSMIT_Init();

  // App Create Task

  APP_DATA_RECEIVE_CreateTask();
  APP_DISPLAY_CreateTask();
  // APP_HANDLE_CAMERA_CreateTask();
  // p_face->CreateTask();
  APP_DATA_TRANSMIT_CreateTask();
}

/******************************************************************************
 *       PRIVATE FUNCTION
 *****************************************************************************/

static inline void
APP_MAIN_InitGPIO (void)
{
  esp_rom_gpio_pad_select_gpio(ILI9341_DC_PIN);
  gpio_set_direction(ILI9341_DC_PIN, GPIO_MODE_OUTPUT);

  esp_rom_gpio_pad_select_gpio(ILI9341_RST_PIN);
  gpio_set_direction(ILI9341_RST_PIN, GPIO_MODE_OUTPUT);
}

static inline void
APP_MAIN_InitCAN (void)
{
  // Config parameter of CAN protocol
  BSP_canConfigDefault();
  BSP_canConfigMode(CAN_MODE);
  BSP_canConfigIO(CAN_TXD_PIN, CAN_RXD_PIN);
  BSP_canConfigQueue(CAN_TXD_QUEUE_LEN, CAN_RXD_QUEUE_LEN);
  BSP_canConfigIntr(CAN_INTR_FLAG);
  BSP_canConfigBitRate(CAN_BITRATE);

  // Install TWAI driver
  BSP_canDriverInit();

  // Start TWAI driver
  BSP_canStart();
}

static inline void
APP_MAIN_InitSPI (void)
{
  BSP_spiConfigDefault();
  BSP_spiConfigMode(SPI2_SPI_MODE);
  BSP_spiConfigIO(SPI2_MISO_PIN, SPI2_MOSI_PIN, SPI2_SCLK_PIN);
  BSP_spiConfigCS(SPI2_CS_PIN);
  BSP_spiMaxTransferSize(SPI2_SPI_BUS_MAX_TRANSFER_SZ);
  BSP_spiClockSpeed(SPI2_CLOCK_SPEED_HZ);
  BSP_spiTransactionQueueSize(SPI2_QUEUE_SIZE);
  BSP_spiDMADriverInit(&spi_ili9341_handle, SPI2_HOST, SPI2_DMA_CHANNEL);

  BSP_spiConfigDefault();
  BSP_spiConfigCommand();
  BSP_spiConfigMode(SPI3_SPI_MODE);
  BSP_spiConfigIO(SPI3_MISO_PIN, SPI3_MOSI_PIN, SPI3_SCLK_PIN);
  BSP_spiConfigCS(SPI3_CS_PIN);
  BSP_spiMaxTransferSize(SPI3_SPI_BUS_MAX_TRANSFER_SZ);
  BSP_spiClockSpeed(SPI3_CLOCK_SPEED_HZ);
  BSP_spiTransactionQueueSize(SPI3_QUEUE_SIZE);

  BSP_spiDMADriverInit(&spi_xpt2046_handle, SPI3_HOST, SPI3_DMA_CHANNEL);
}

static inline void
APP_MAIN_InitDataSystem (void)
{
  s_ili9341_0.p_spi_Handle = &spi_ili9341_handle;
  s_ili9341_0.e_dc_pin     = ILI9341_DC_PIN;
  s_ili9341_0.e_rst_pin    = ILI9341_RST_PIN;

  s_xpt2046_0.p_spi_Handle = &spi_xpt2046_handle;
  s_xpt2046_0.e_irq_pin    = XPT2046_IRQ_PIN;

  s_data_system.s_send_data_queue      = xQueueCreate(4, sizeof(DATA_SYNC_t));
  s_data_system.s_camera_capture_queue = xQueueCreate(2, sizeof(camera_fb_t *));
  s_data_system.s_camera_recognition_queue
      = xQueueCreate(2, sizeof(camera_capture_t));
  s_data_system.s_result_recognition_queue
      = xQueueCreate(2, sizeof(data_result_recognition_t));
  s_data_system.s_receive_data_event_queue
      = xQueueCreate(64, sizeof(DATA_SYNC_t));
  s_data_system.s_display_event = xEventGroupCreate();
}
