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

/*** CAN peripheral **********************************************************/

#define CAN_MODE          TWAI_MODE_NORMAL
#define CAN_TXD_PIN       GPIO_NUM_21
#define CAN_RXD_PIN       GPIO_NUM_14
#define CAN_TXD_QUEUE_LEN 8
#define CAN_RXD_QUEUE_LEN 8
#define CAN_INTR_FLAG     ESP_INTR_FLAG_LEVEL3 // lowest priority
#define CAN_BITRATE       CAN_500KBITS

/*** SPI2 peripheral *********************************************************/

#define SPI2_MISO_PIN GPIO_NUM_NC
#define SPI2_MOSI_PIN GPIO_NUM_47
#define SPI2_SCLK_PIN GPIO_NUM_48
#define SPI2_CS_PIN   GPIO_NUM_46

#define SPI2_CLOCK_SPEED_HZ          SPI_CLOCK_40MHz
#define SPI2_SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)
#define SPI2_DMA_CHANNEL             SPI_DMA_CH_AUTO
#define SPI2_SPI_MODE                0
#define SPI2_QUEUE_SIZE              50

/*** SPI3 peripheral *********************************************************/

#define SPI3_MISO_PIN GPIO_NUM_40
#define SPI3_MOSI_PIN GPIO_NUM_38
#define SPI3_SCLK_PIN GPIO_NUM_45
#define SPI3_CS_PIN   GPIO_NUM_41

#define SPI3_CLOCK_SPEED_HZ          SPI_CLOCK_100KHz
#define SPI3_SPI_BUS_MAX_TRANSFER_SZ 0
#define SPI3_DMA_CHANNEL             SPI_DMA_CH_AUTO
#define SPI3_SPI_MODE                0
#define SPI3_QUEUE_SIZE              1

/*** ILI9341 device **********************************************************/

#define ILI9341_DC_PIN  GPIO_NUM_39
#define ILI9341_RST_PIN GPIO_NUM_5

/*** XPT2046 device **********************************************************/

#define XPT2046_IRQ_PIN GPIO_NUM_42

/******************************************************************************
 *   PUBLIC DATA
 *****************************************************************************/

ili9341_handle_t s_ili9341_0;
xpt2046_handle_t s_xpt2046_0;

Face *p_face;

/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

static spi_device_handle_t spi_ili9341_handle;
static spi_device_handle_t spi_xpt2046_handle;

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static inline void APP_MAIN_InitGPIO(void);
static inline void APP_MAIN_InitCan(void);
static inline void APP_MAIN_InitSpi(void);
static inline void APP_MAIN_InitDataSystem(void);

/******************************************************************************
 *       MAIN FUNCTION
 *****************************************************************************/

extern "C" void
app_main (void)
{
  // BSP Initialization

  APP_MAIN_InitGPIO();
  APP_MAIN_InitSpi();
  APP_MAIN_InitCan();

  // Main Initialization data system

  APP_MAIN_InitDataSystem();

  // App Initialization

  APP_DISPLAY_Init();
  // APP_HANDLE_CAMERA_Init();
  // p_face = new Face();
  APP_DATA_TRANSMIT_Init();
  APP_DATA_RECEIVE_Init();

  // App Create Task

  APP_DISPLAY_CreateTask();
  // APP_HANDLE_CAMERA_CreateTask();
  // p_face->CreateTask();
  APP_DATA_TRANSMIT_CreateTask();
  APP_DATA_RECEIVE_CreateTask();
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
APP_MAIN_InitCan (void)
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
APP_MAIN_InitSpi (void)
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
  s_data_system.s_display_data_queue   = xQueueCreate(1, sizeof(DATA_SYNC_t));
  s_data_system.s_camera_capture_queue = xQueueCreate(2, sizeof(camera_fb_t *));
  s_data_system.s_camera_recognition_queue
      = xQueueCreate(2, sizeof(camera_capture_t));
  s_data_system.s_result_recognition_queue
      = xQueueCreate(2, sizeof(data_result_recognition_t));

  s_data_system.s_display_event = xEventGroupCreate();
}
