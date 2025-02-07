/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdspi.h"
#include "diskio.h"
/*****************************************************************************
 *   PRIVATE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *   PRIVATE VARIABLES
 *****************************************************************************/

static spi_device_handle_t s_spi            = NULL;  // SPI handle
static BSP_SD_SPI_CardInfo s_cardInfo       = { 0 }; // Save info SDCard
static bool                s_spi_bus_inited = false;

/*****************************************************************************
 *   PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/

static inline void BSP_sdSpiSelect(gpio_num_t e_cs_io);
static inline void BSP_sdSpiDeSelect(gpio_num_t e_cs_io);
static void        BSP_sdSpiWriteByte(uint8_t data);
static void        BSP_sdSpiWriteBuffer(const uint8_t *buffer, size_t len);
static uint8_t     BSP_sdSpiReadByte(void);
static uint8_t     BSP_sdSpiReadyWait(void);
static void        BSP_sdSpiSendDummyClocks(gpio_num_t e_cs_io);
static bool        BSP_sdSpiReadDataBlock(uint8_t *buff, size_t len);
static uint8_t     BSP_sdSpiSendCmd(uint8_t cmd, uint32_t arg);

/*****************************************************************************
 *    PUBLIC FUNCTIONS
 *****************************************************************************/

uint8_t
BSP_sdSpiInit (spi_host_device_t e_spi_host,
               gpio_num_t        e_miso_io,
               gpio_num_t        e_mosi_io,
               gpio_num_t        e_sclk_io,
               int               i_max_transfer_sz,
               gpio_num_t        e_cs_io,
               int               i_dma_channel,
               int               i_clock_init_hz,
               int               i_clock_work_hz,
               uint8_t           u8_spi_mode)
{
  esp_err_t ret = ESP_OK; // Sửa lỗi NULL -> ESP_OK

  if (!s_spi_bus_inited)
  {
    // 1) Cấu hình GPIO CS_PIN = output
    gpio_config_t io_conf = { .pin_bit_mask = 1ULL << e_cs_io,
                              .mode         = GPIO_MODE_OUTPUT,
                              .pull_up_en   = 0,
                              .pull_down_en = 0,
                              .intr_type    = GPIO_INTR_DISABLE };
    gpio_config(&io_conf);
    BSP_sdSpiDeSelect(e_cs_io);

    // 2) Init SPI bus
    spi_bus_config_t buscfg = { .miso_io_num     = e_miso_io,
                                .mosi_io_num     = e_mosi_io,
                                .sclk_io_num     = e_sclk_io,
                                .quadwp_io_num   = -1,
                                .quadhd_io_num   = -1,
                                .max_transfer_sz = i_max_transfer_sz };
    ret = spi_bus_initialize(e_spi_host, &buscfg, i_dma_channel);
    if (ret != ESP_OK)
    {
      printf("spi_bus_initialize failed=%d\n", ret);
      return MSD_ERROR;
    }
  }

  // 3) Gán device
  spi_device_interface_config_t devcfg = {
    .clock_speed_hz = i_clock_init_hz,
    .mode           = u8_spi_mode,
    .spics_io_num   = -1,
    .queue_size     = 1,
  };
  ret = spi_bus_add_device(e_spi_host, &devcfg, &s_spi);

  if (ret != ESP_OK)
  {
    printf("spi_bus_add_device failed=%d\n", ret);
    return MSD_ERROR;
  }
  s_spi_bus_inited = true;

  uint8_t  response;
  uint8_t  n, csd[16];
  uint32_t csize;

  BSP_sdSpiSendDummyClocks(e_cs_io);

  BSP_sdSpiSelect(e_cs_io);

  // CMD0
  if (BSP_sdSpiSendCmd(CMD0, 0) != 0x01)
  {
    BSP_sdSpiDeSelect(e_cs_io);
    return MSD_ERROR;
  }

  // CMD8 => Check SD v2
  response   = BSP_sdSpiSendCmd(CMD8, 0x1AA);
  bool sd_v2 = false;
  if (response == 0x01)
  {
    uint8_t r7[4];
    for (int i = 0; i < 4; i++)
    {
      r7[i] = BSP_sdSpiReadByte();
    }
    if (r7[2] == 0x01 && r7[3] == 0xAA)
    {
      sd_v2 = true;
    }
  }

  // Check CMD55 and ACMD41
  TickType_t start = xTaskGetTickCount();
  do
  {
    response = BSP_sdSpiSendCmd(CMD55, 0);
    response = BSP_sdSpiSendCmd(ACMD41, (sd_v2 ? (1UL << 30) : 0));
    if (response == 0)
    {
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } while (xTaskGetTickCount() - start < pdMS_TO_TICKS(2000));

  if (response != 0)
  {
    BSP_sdSpiDeSelect(e_cs_io);
    printf("ACMD41 fail, r1=0x%02X\n", response);
    return MSD_ERROR;
  }

  // CMD58 => Read OCR + check CCS
  response = BSP_sdSpiSendCmd(CMD58, 0);
  if (response != 0)
  {
    printf("CMD58 failed, R1=%02X\n", response);
    return MSD_ERROR;
  }

  uint8_t ocr[4];
  for (int i = 0; i < 4; i++)
  {
    ocr[i] = BSP_sdSpiReadByte();
  }
  if (ocr[0] & 0x40)
  {
    s_cardInfo.CardType = (CT_SDC2 | CT_BLOCK);
    printf("SDHC/SDXC detected\n");
  }
  else
  {
    s_cardInfo.CardType = CT_SDC2;
    printf("SDSC (v2) detected\n");
  }

  BSP_sdSpiDeSelect(e_cs_io);
  BSP_sdSpiReadByte(); // Idle State Transitions (Release DO)

  s_cardInfo.LogBlockNbr  = 0;
  s_cardInfo.LogBlockSize = 512;

  BSP_sdSpiSelect(e_cs_io);
  if ((BSP_sdSpiSendCmd(CMD9, 0) == 0) && BSP_sdSpiReadDataBlock(csd, 16) == 1)
  {
    if ((csd[0] >> 6) == 1)
    {
      csize                  = csd[9] + ((uint32_t)csd[8] << 8) + 1;
      s_cardInfo.LogBlockNbr = csize << 10;
    }
    else
    {
      n = (csd[5] & 15) + ((csd[10] & 0x80) >> 7) + ((csd[9] & 3) << 1) + 2;
      csize
          = (csd[8] >> 6) + ((uint32_t)csd[7] << 2) + ((csd[6] & 3) << 10) + 1;
      s_cardInfo.LogBlockNbr = csize << (n - 9);
    }
    s_cardInfo.LogBlockSize = 512;
    // printf("SD total sectors = %lu, sector_size=%lu\n",
    //        s_cardInfo.LogBlockNbr,
    //        s_cardInfo.LogBlockSize);
  }
  else
  {
    printf("Failed!\n");
    return MSD_ERROR;
  }
  BSP_sdSpiDeSelect(e_cs_io);

  // Change Frequency Clock
  devcfg.clock_speed_hz = i_clock_work_hz;
  ret                   = spi_bus_add_device(e_spi_host, &devcfg, &s_spi);
  if (ret != ESP_OK)
  {
    printf("re-add device fail for high freq\n");
    return MSD_ERROR;
  }
  else
  {
    printf("re-add device successfully for high freq\n");
  }

  return MSD_OK;
}

uint8_t
BSP_sdSpiReadBlocks (uint8_t   *p_data,
                     uint32_t   readAddr,
                     uint32_t   numOfBlocks,
                     uint32_t   timeout,
                     gpio_num_t e_cs_io)
{
  // SDSC => địa chỉ byte
  if (!(s_cardInfo.CardType & CT_BLOCK))
  {
    readAddr *= 512;
  }

  BSP_sdSpiSelect(e_cs_io);

  uint8_t response;
  if (numOfBlocks == 1)
  {
    // CMD17
    response = BSP_sdSpiSendCmd(CMD17, readAddr);
    if (response == 0)
    {
      // Read 512 byte
      if (!BSP_sdSpiReadDataBlock(p_data, 512))
      {
        BSP_sdSpiDeSelect(e_cs_io);
        return MSD_ERROR;
      }
      numOfBlocks = 0;
    }
  }
  else
  {
    // CMD18
    response = BSP_sdSpiSendCmd(CMD18, readAddr);
    if (response == 0)
    {
      while (numOfBlocks)
      {
        if (!BSP_sdSpiReadDataBlock(p_data, 512))
        {
          break;
        }
        p_data += 512;
        numOfBlocks--;
      }
      // STOP_TRANSMISSION (CMD12)
      BSP_sdSpiSendCmd(CMD12, 0);
    }
  }

  BSP_sdSpiDeSelect(e_cs_io);
  BSP_sdSpiReadByte(); // Idle State Transitions (Release DO)

  if (numOfBlocks == 0)
  {
    return MSD_OK;
  }
  return MSD_ERROR;
}

uint8_t
BSP_sdSpiWriteBlocks (uint8_t   *p_data,
                      uint32_t   writeAddr,
                      uint32_t   numOfBlocks,
                      uint32_t   timeout,
                      gpio_num_t e_cs_io)
{
  uint8_t  token;
  uint8_t  response;
  uint16_t i;
  uint32_t startTime = xTaskGetTickCount();

  if (!(s_cardInfo.CardType & CT_BLOCK))
  {
    writeAddr *= 512;
  }

  // printf("WriteAddr: %lu, numOfBlocks: %lu\n", writeAddr, numOfBlocks);

  if (numOfBlocks == 1)
  {
    BSP_sdSpiSelect(e_cs_io);
    if (BSP_sdSpiSendCmd(CMD24, writeAddr) != 0)
    {
      printf("CMD24 failed for WriteAddr: %lu\n", writeAddr);
      BSP_sdSpiDeSelect(e_cs_io);
      return MSD_ERROR;
    }
  }
  else
  {
    BSP_sdSpiSelect(e_cs_io);
    if (BSP_sdSpiSendCmd(CMD25, writeAddr) != 0)
    {
      printf("CMD25 failed for WriteAddr: %lu\n", writeAddr);
      BSP_sdSpiDeSelect(e_cs_io);
      return MSD_ERROR;
    }
  }

  token = (numOfBlocks == 1) ? 0xFE : 0xFC;
  // printf("Sending START_TOKEN: 0x%02X\n", token);
  BSP_sdSpiWriteByte(token);

  for (uint32_t block = 0; block < numOfBlocks; block++)
  {
    for (i = 0; i < 512; i++)
    {
      BSP_sdSpiWriteByte(p_data[block * 512 + i]);
    }

    BSP_sdSpiWriteByte(0xFF);
    BSP_sdSpiWriteByte(0xFF);

    response = BSP_sdSpiReadByte();
    // printf("Write Response for WriteAddr %lu: 0x%02X\n", writeAddr,
    // response);

    if ((response & 0x1F) != 0x05)
    {
      // printf("SD card rejected data block at WriteAddr %lu\n", writeAddr);
      BSP_sdSpiDeSelect(e_cs_io);
      return MSD_ERROR;
    }

    while (BSP_sdSpiReadByte() == 0x00)
    {
      if ((xTaskGetTickCount() - startTime) > timeout)
      {
        // printf("Timeout waiting for SD write completion\n");
        BSP_sdSpiDeSelect(e_cs_io);
        return MSD_ERROR;
      }
      vTaskDelay(1);
    }
  }

  if (numOfBlocks > 1)
  {
    BSP_sdSpiWriteByte(0xFD);
  }

  while (BSP_sdSpiReadByte() == 0x00)
  {
    if ((xTaskGetTickCount() - startTime) > timeout)
    {
      printf("Timeout waiting for SD card\n");
      BSP_sdSpiDeSelect(e_cs_io);
      return MSD_ERROR;
    }
    vTaskDelay(1);
  }

  BSP_sdSpiDeSelect(e_cs_io);
  return MSD_OK;
}

void
BSP_sdSpiGetCardInfo (BSP_SD_SPI_CardInfo *CardInfo)
{
  CardInfo->CardType     = s_cardInfo.CardType;
  CardInfo->LogBlockNbr  = s_cardInfo.LogBlockNbr;
  CardInfo->LogBlockSize = s_cardInfo.LogBlockSize;
}

/*****************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/
static inline void
BSP_sdSpiSelect (gpio_num_t e_cs_io)
{
  gpio_set_level(e_cs_io, 0);
}

static inline void
BSP_sdSpiDeSelect (gpio_num_t e_cs_io)
{
  gpio_set_level(e_cs_io, 1);
}

static void
BSP_sdSpiWriteByte (uint8_t data)
{
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = 8;
  t.tx_buffer = &data;
  t.rx_buffer = NULL;
  spi_device_polling_transmit(s_spi, &t);
}

static void
BSP_sdSpiWriteBuffer (const uint8_t *buffer, size_t len)
{
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = len * 8;
  t.tx_buffer = buffer;
  spi_device_polling_transmit(s_spi, &t);
}

/* Read 1 byte from SD Card and send 0xFF */
static uint8_t
BSP_sdSpiReadByte (void)
{
  uint8_t           txData = 0xFF, rxData = 0xFF;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = 8;
  t.tx_buffer = &txData;
  t.rx_buffer = &rxData;
  spi_device_polling_transmit(s_spi, &t);
  return rxData;
}

static void
BSP_sdSpiSendDummyClocks (gpio_num_t e_cs_io)
{
  BSP_sdSpiDeSelect(e_cs_io);
  uint8_t ff[10];
  memset(ff, 0xFF, 10);
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = 10 * 8;
  t.tx_buffer = ff;
  spi_device_polling_transmit(s_spi, &t);
}

static uint8_t
BSP_sdSpiReadyWait (void)
{
  const TickType_t startTick = xTaskGetTickCount();
  uint8_t          response;
  do
  {
    response = BSP_sdSpiReadByte();
    if (response == 0xFF)
    {
      break;
    }
    vTaskDelay(1); // 1ms
  } while ((xTaskGetTickCount() - startTick) < pdMS_TO_TICKS(500));

  return response; // 0xFF = ready
}

static bool
BSP_sdSpiReadDataBlock (uint8_t *buff, size_t len)
{
  const TickType_t startTick = xTaskGetTickCount();
  uint8_t          token;
  do
  {
    token = BSP_sdSpiReadByte();
    if (token == 0xFE)
    {
      break;
    }
    vTaskDelay(1);
  } while ((xTaskGetTickCount() - startTick) < pdMS_TO_TICKS(200));

  if (token != 0xFE)
  {
    printf("BSP_sdSpiReadDataBlock: invalid token=0x%02X", token);
    return false;
  }

  for (size_t i = 0; i < len; i++)
  {
    buff[i] = BSP_sdSpiReadByte();
  }

  // Bỏ qua 2 byte CRC
  BSP_sdSpiReadByte();
  BSP_sdSpiReadByte();
  return true;
}

static uint8_t
BSP_sdSpiSendCmd (uint8_t cmd, uint32_t arg)
{
  // Wait SDCard ready
  if (BSP_sdSpiReadyWait() != 0xFF)
  {
    // Not ready
    printf("SD Card not ready!");
  }

  // Send 6 bytes: [CMD|0x40, arg(4), CRC]
  uint8_t buf[6];
  buf[0] = (cmd & 0x3F) | 0x40; // Command
  buf[1] = (arg >> 24) & 0xFF;  // Argument[31..24]
  buf[2] = (arg >> 16) & 0xFF;  // Argument[23..16]
  buf[3] = (arg >> 8) & 0xFF;   // Argument[15..8]
  buf[4] = (arg) & 0xFF;        // Argument[7..0]

  // Prepare CRC
  uint8_t crc = 1;
  if (cmd == CMD0)
  {
    crc = 0x95; // CRC for CMD0(0)
  }
  else if (cmd == CMD8)
  {
    crc = 0x87; // CRC for CMD0(0)
  }
  buf[5] = crc;

  BSP_sdSpiWriteBuffer(buf, 6);

  // Skip a stuff byte when STOP_TRANSMISSION
  if (cmd == CMD12)
  {
    BSP_sdSpiReadByte();
  }

  // Receive response
  uint8_t response = 0xFF;
  for (int i = 0; i < 10; i++)
  {
    response = BSP_sdSpiReadByte();
    if ((response & 0x80) == 0)
    {
      break; // bit7 = 0 => OK
    }
  }
  return response;
}