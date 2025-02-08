/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */

#include "sdspi.h"
#include "sdcard.h"

/* Private variables ---------------------------------------------------------*/
static volatile DSTATUS Stat        = STA_NOINIT;
static volatile UINT    WriteStatus = 0, ReadStatus = 0;

/* Private define ------------------------------------------------------------*/

/*
 * the following Timeout is useful to give the control back to the applications
 * in case of errors in either BSP_SD_ReadCpltCallback() or
 * BSP_SD_WriteCpltCallback() the value by default is as defined in the BSP
 * platform driver otherwise 30 secs
 */
#define SD_TIMEOUT 30 * 1000000 //Unit: us -> s

#define SD_DEFAULT_BLOCK_SIZE 512

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS
disk_status(BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
  if (pdrv)
  {
    return STA_NOINIT;
  }
  return Stat;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS
disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
  Stat = STA_NOINIT;
  if (BSP_sdSpiInit(HSPI_HOST,
                    MISO_PIN,
                    MOSI_PIN,
                    SCLK_PIN,
                    MAX_TRANSFER_SZ,
                    CS_PIN,
                    DMA_CHANNEL,
                    SD_INIT_FREQ,
                    SD_WORK_FREQ,
                    SPI_MODE)
      == MSD_OK)
  {
    Stat &= ~STA_NOINIT;
  }
  return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT
disk_read(BYTE  pdrv,   /* Physical drive nmuber to identify the drive */
          BYTE *buff,   /* Data buffer to store read data */
          LBA_t sector, /* Start sector in LBA */
          UINT  count   /* Number of sectors to read */
)
{
  DRESULT res = RES_ERROR;

  if (BSP_sdSpiReadBlocks(
          (uint8_t *)buff, (uint32_t)(sector), count, SD_TIMEOUT, CS_PIN)
      == MSD_OK)
  {
    res = RES_OK;
  }

  return res;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

/* USER CODE BEGIN beforeWriteSection */
/* can be used to modify previous code / undefine following code / add new code
 */
/* USER CODE END beforeWriteSection */
/**
 * @brief  Writes Sector(s)
 * @param  lun : not used
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */

DRESULT
disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
  DRESULT res = RES_ERROR;

  if (BSP_sdSpiWriteBlocks(
          (uint8_t *)buff, (uint32_t)sector, count, SD_TIMEOUT, CS_PIN)
      == MSD_OK)
  {
    res = RES_OK;
  }

  return res;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT
disk_ioctl(BYTE  pdrv, /* Physical drive nmuber (0..) */
           BYTE  cmd,  /* Control code */
           void *buff  /* Buffer to send/receive control data */
)
{
  DRESULT             res = RES_ERROR;
  BSP_SD_SPI_CardInfo CardInfo;

  if (Stat & STA_NOINIT)
  {
    return RES_NOTRDY;
  }

  switch (cmd)
  {
    /* Make sure that no pending write process */
    case CTRL_SYNC:
      res = RES_OK;
      break;

    /* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT:
      BSP_sdSpiGetCardInfo(&CardInfo);
      *(DWORD *)buff = CardInfo.LogBlockNbr;
      res            = RES_OK;
      break;

    /* Get R/W sector size (WORD) */
    case GET_SECTOR_SIZE:
      BSP_sdSpiGetCardInfo(&CardInfo);
      *(WORD *)buff = CardInfo.LogBlockSize;
      res           = RES_OK;
      break;

    /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE:
      BSP_sdSpiGetCardInfo(&CardInfo);
      *(DWORD *)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
      res            = RES_OK;
      break;

    default:
      res = RES_PARERR;
  }

  return res;
}

/* USER CODE BEGIN callbackSection */
/* can be used to modify / following code or add new code */
/* USER CODE END callbackSection */
/**
 * @brief Tx Transfer completed callbacks
 * @param hsd: SD handle
 * @retval None
 */
void
BSP_sdSpiWriteCpltCallback (void)
{

  WriteStatus = 1;
}

/**
 * @brief Rx Transfer completed callbacks
 * @param hsd: SD handle
 * @retval None
 */
void
BSP_sdSpiReadCpltCallback (void)
{
  ReadStatus = 1;
}

/**
 * @brief  Gets Time from RTC
 * @param  None
 * @retval Time in DWORD
 */
DWORD
get_fattime(void)
{
  return 0;
}

#if FF_MULTI_PARTITION
PARTITION VolToPart[] = { { 0, 1 } };
#endif
