/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "esp_timer.h"

#include "sdcard.h"
#include "ff.h"
#include "diskio.h"

/*****************************************************************************
 *   PRIVATE DATA
 *****************************************************************************/

SDCard_Info_t s_cardInfo;
bool          is_initialized = false;

/*****************************************************************************
 *   PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/
static void DEV_SDCard_SendDummyClocks(spi_device_handle_t spi_handle,
                                       gpio_num_t          e_cs_io);

static uint8_t DEV_SDCard_ReadyWait(spi_device_handle_t spi_handle);

static uint8_t DEV_SDCard_ReadDataBlock(spi_device_handle_t spi_handle,
                                        uint8_t            *buff,
                                        uint32_t            len);

static uint8_t DEV_SDCard_SendCmd(spi_device_handle_t spi_handle,
                                  uint8_t             cmd,
                                  uint32_t            arg);

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

uint8_t
DEV_SDCard_Init (spi_device_handle_t *spi_handle,
                 spi_host_device_t    e_spi_host,
                 gpio_num_t           e_cs_io)
{
  uint8_t  response;
  uint8_t  n, csd[16];
  uint32_t csize;

  if (is_initialized == false)
  {
    // Send dummy clocks to initialize SDCard
    DEV_SDCard_SendDummyClocks(*spi_handle, e_cs_io);

    // Send CMD0 => Reset SDCard
    if (DEV_SDCard_SendCmd(*spi_handle, CMD0, 0) != 0x01)
    {
      // Response incorrect -> Stop
      BSP_spiDeselectDevice(e_cs_io);
      return MSD_ERROR;
    }

    // Send CMD8 => Check SD v2
    bool sd_v2 = false;
    if (DEV_SDCard_SendCmd(*spi_handle, CMD8, 0x1AA) == 0x01)
    {
      // Receive response from SDCard
      uint8_t r7[4];
      for (int i = 0; i < 4; i++)
      {
        r7[i] = BSP_spiReadByte(*spi_handle);
      }

      // Check SD v2 (Check voltage range and check pattern)
      if (r7[2] == 0x01 && r7[3] == 0xAA)
      {
        sd_v2 = true;
      }
    }

    // Check CMD55 and ACMD41
    TickType_t start = esp_timer_get_time();
    do
    {
      response = DEV_SDCard_SendCmd(
          *spi_handle,
          CMD55,
          0); // Sending CMD55 to tell SDCard next command card application
              // command, not the standard command
      response
          = DEV_SDCard_SendCmd(*spi_handle, ACMD41, (sd_v2 ? 0x40000000 : 0));
      if (response == 0)
      {
        break;
      }
    } while (esp_timer_get_time() - start < 2000000ULL); // Timeout 2s

    // CMD58 => Read OCR and check CCS => Determine CardType (SDSC or SDHC/SDXC)
    response = DEV_SDCard_SendCmd(*spi_handle, CMD58, 0);
    if (response != 0)
    {
      printf("CMD58 failed, R1=%02X\n", response);
      return MSD_ERROR;
    }

    // Check CardType and store to struct s_cardInfo
    uint8_t ocr[4];
    for (int i = 0; i < 4; i++)
    {
      ocr[i] = BSP_spiReadByte(*spi_handle);
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

    BSP_spiDeselectDevice(e_cs_io);
    BSP_spiReadByte(*spi_handle); // Idle State Transitions (Release DO)

    // Init the first value for Block Number and Block Size
    s_cardInfo.LogBlockNbr  = 0;
    s_cardInfo.LogBlockSize = 512;

    BSP_spiSelectDevice(e_cs_io);

    // CMD9: Check number of blocks and block size via ReadDataBlock
    // store in csd array
    if ((DEV_SDCard_SendCmd(*spi_handle, CMD9, 0) == 0)
        && DEV_SDCard_ReadDataBlock(*spi_handle, csd, 16) == 1)
    {
      if ((csd[0] >> 6) == 1)
      {
        csize                  = csd[9] + ((uint32_t)csd[8] << 8) + 1;
        s_cardInfo.LogBlockNbr = csize << 10;
      }
      else
      {
        n = (csd[5] & 15) + ((csd[10] & 0x80) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint32_t)csd[7] << 2) + ((csd[6] & 3) << 10)
                + 1;
        s_cardInfo.LogBlockNbr = csize << (n - 9);
      }
      s_cardInfo.LogBlockSize = 512;
    }
    else
    {
      printf("Failed!\n");
      return MSD_ERROR;
    }

    BSP_spiDeselectDevice(e_cs_io);

    // Set high frequency for SD Card
    BSP_spiSChangeClock(*spi_handle, e_spi_host, SPI_CLOCK_10MHz);
    is_initialized = true;
    return MSD_OK;
  }
  return MSD_OK;
}

uint8_t
DEV_SDCard_ReadBlock (spi_device_handle_t spi_handle,
                      uint8_t            *p_data,
                      uint32_t            u32_readAddr,
                      uint32_t            u32_numOfBlocks,
                      gpio_num_t          e_cs_io)
{
  uint8_t response;

  // SDSC => byte address
  if (!(s_cardInfo.CardType & CT_BLOCK))
  {
    u32_readAddr *= 512; // sector -> byte
  }

  BSP_spiSelectDevice(e_cs_io);

  if (u32_numOfBlocks == 1)
  {
    // CMD17 => Single Block Read
    response = DEV_SDCard_SendCmd(spi_handle, CMD17, u32_readAddr);
    if (response == 0)
    {
      // Read 512 byte
      if (!DEV_SDCard_ReadDataBlock(spi_handle, p_data, 512))
      {
        BSP_spiDeselectDevice(e_cs_io);
        return MSD_ERROR;
      }
      u32_numOfBlocks = 0;
    }
  }
  else
  {
    // CMD18 => Multiple Block Read
    response = DEV_SDCard_SendCmd(spi_handle, CMD18, u32_readAddr);
    if (response == 0)
    {
      while (u32_numOfBlocks)
      {
        if (!DEV_SDCard_ReadDataBlock(spi_handle, p_data, 512))
        {
          break;
        }
        p_data += 512;
        u32_numOfBlocks--;
      }
      // Stop transmission (CMD12)
      DEV_SDCard_SendCmd(spi_handle, CMD12, 0);
    }
  }

  BSP_spiDeselectDevice(e_cs_io);
  BSP_spiReadByte(spi_handle);

  if (u32_numOfBlocks == 0)
  {
    return MSD_OK;
  }
  return MSD_ERROR;
}

uint8_t
DEV_SDCard_WriteBlock (spi_device_handle_t spi_handle,
                       uint8_t            *p_data,
                       uint32_t            u32_writeAddr,
                       uint32_t            u32_numOfBlocks,
                       uint32_t            u32_timeout,
                       gpio_num_t          e_cs_io)
{
  uint8_t  token;
  uint8_t  response;
  uint16_t i;
  uint32_t startTime = esp_timer_get_time();

  // SDSC => byte address
  if (!(s_cardInfo.CardType & CT_BLOCK))
  {
    u32_writeAddr *= 512; // sector -> byte
  }

  if (u32_numOfBlocks == 1)
  {
    BSP_spiSelectDevice(e_cs_io);
    if (DEV_SDCard_SendCmd(spi_handle, CMD24, u32_writeAddr)
        != 0) // Single Block Write
    {
      printf("CMD24 failed for WriteAddr: %lu\n", u32_writeAddr);
      BSP_spiDeselectDevice(e_cs_io);
      return MSD_ERROR;
    }
  }
  else
  {
    BSP_spiSelectDevice(e_cs_io);
    if (DEV_SDCard_SendCmd(spi_handle, CMD25, u32_writeAddr)
        != 0) // Multiple Block Write
    {
      printf("CMD25 failed for WriteAddr: %lu\n", u32_writeAddr);
      BSP_spiDeselectDevice(e_cs_io);
      return MSD_ERROR;
    }
  }

  // Start Token = 0xFE -> Write 1 block
  token = (u32_numOfBlocks == 1) ? 0xFE : 0xFC;

  BSP_spiWriteByte(spi_handle, token);

  for (uint32_t block = 0; block < u32_numOfBlocks; block++)
  {
    for (i = 0; i < 512; i++)
    {
      BSP_spiWriteByte(spi_handle, p_data[block * 512 + i]);
    }

    // CRC Bytes
    BSP_spiWriteByte(spi_handle, 0xFF);
    BSP_spiWriteByte(spi_handle, 0xFF);

    // Data response token:
    //  0x05: Data accepted
    //  0x0B: Data rejected due to CRC error
    //  0x0D: Data rejected due to write error
    response = BSP_spiReadByte(spi_handle);
    if ((response & 0x1F) != 0x05)
    { // & 0x1F: filter 5 bit lower that contain data response
      BSP_spiDeselectDevice(e_cs_io); // Write failed
      return MSD_ERROR;
    }

    // Check SDCard Busy
    while (BSP_spiReadByte(spi_handle) == 0x00)
    {
      if ((esp_timer_get_time() - startTime) > u32_timeout)
      {
        printf("Timeout waiting for SD write completion\n");
        BSP_spiDeselectDevice(e_cs_io);
        return MSD_ERROR;
      }
    }
  }

  if (u32_numOfBlocks > 1)
  {
    BSP_spiWriteByte(spi_handle,
                     0xFD); // STOP_TRANSMISSION Bytes for write multi-block
  }

  // Check SDCard Busy
  while (BSP_spiReadByte(spi_handle) == 0x00)
  {
    if ((esp_timer_get_time() - startTime) > u32_timeout)
    {
      printf("Timeout waiting for SD card\n");
      BSP_spiDeselectDevice(e_cs_io);
      return MSD_ERROR;
    }
  }

  BSP_spiDeselectDevice(e_cs_io);
  return MSD_OK;
}

void
DEV_SDCard_Info (SDCard_Info_t *p_CardInfo)
{
  p_CardInfo->CardType     = s_cardInfo.CardType;
  p_CardInfo->LogBlockNbr  = s_cardInfo.LogBlockNbr;
  p_CardInfo->LogBlockSize = s_cardInfo.LogBlockSize;
}

/*****************************************************************************
 *   PRIVATE FUNCTIONS
 *****************************************************************************/
static void
DEV_SDCard_SendDummyClocks (spi_device_handle_t spi_handle, gpio_num_t e_cs_io)
{
  // Set logic cs io to 1
  BSP_spiDeselectDevice(e_cs_io);

  // Prepare data before send
  uint8_t ff[10];
  memset(ff, 0xFF, 10);

  // Send dummy data
  BSP_spiWriteMultipleByte(spi_handle, ff, sizeof(ff));

  // Set logic cs io to 0
  BSP_spiSelectDevice(e_cs_io);
}

static uint8_t
DEV_SDCard_ReadyWait (spi_device_handle_t spi_handle)
{
  const TickType_t startTick = esp_timer_get_time();
  uint8_t          response;
  do
  {
    response = BSP_spiReadByte(spi_handle);
    if (response == 0xFF)
    {
      break;
    }
  } while ((esp_timer_get_time() - startTick) < 500000ULL); // 500ms timeout

  return response; // 0xFF = ready
}

static uint8_t
DEV_SDCard_ReadDataBlock (spi_device_handle_t spi_handle,
                          uint8_t            *buff,
                          uint32_t            len)
{
  const TickType_t startTick = esp_timer_get_time();
  uint8_t          token;
  do
  {
    token = BSP_spiReadByte(spi_handle);
    if (token == 0xFE) // 0xFE = Start Block Token
    {
      break;
    }
  } while ((esp_timer_get_time() - startTick) < 500000ULL); // 500ms timeout

  if (token != 0xFE)
  {
    printf("BSP_sdSpiReadDataBlock: invalid token=0x%02X", token);
    return 0;
  }

  for (uint32_t i = 0; i < len; i++)
  {
    buff[i] = BSP_spiReadByte(spi_handle);
  }

  // Pass 2 bytes CRC
  BSP_spiReadByte(spi_handle);
  BSP_spiReadByte(spi_handle);

  return 1;
}

static uint8_t
DEV_SDCard_SendCmd (spi_device_handle_t spi_handle, uint8_t cmd, uint32_t arg)
{
  // Wait SDCard ready
  if (DEV_SDCard_ReadyWait(spi_handle) != 0xFF)
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
    crc = 0x95; // CRC for CMD0
  }
  else if (cmd == CMD8)
  {
    crc = 0x87; // CRC for CMD8
  }

  buf[5] = crc;

  BSP_spiWriteMultipleByte(spi_handle, buf, 6);

  // Skip a stuff byte when STOP_TRANSMISSION
  if (cmd == CMD12)
  {
    BSP_spiReadByte(spi_handle);
  }

  // Receive response, check the r1 register
  // bit 7 in r1 register is 0 => OK
  uint8_t response = 0xFF;
  for (int i = 0; i < 10; i++)
  {
    response = BSP_spiReadByte(spi_handle);
    if ((response & 0x80) == 0)
    {
      break; // bit7 = 0 => OK
    }
  }
  return response;
}