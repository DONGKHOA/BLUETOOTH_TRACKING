#ifndef SDSPI_H_
#define SDSPI_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"
// #include "driver/spi_master.h"
#include "spi.h"

/*****************************************************************************
 *      PUBLIC DEFINES
 *****************************************************************************/

#define HSPI_HOST SPI2_HOST
#define MISO_PIN  GPIO_NUM_19
#define MOSI_PIN  GPIO_NUM_23
#define SCLK_PIN  GPIO_NUM_18
#define CS_PIN    GPIO_NUM_5

#define CLOCK_SPEED_HZ  100000
#define MAX_TRANSFER_SZ 4096
#define DMA_CHANNEL     1
#define SPI_MODE        0
/*****************************************************************************
 *      PUBLIC TYPEDEFS
 *****************************************************************************/

typedef struct _BSP_SD_SPI_CardInfo
{
  uint32_t CardType;     /*!< Specifies the card Type */
  uint32_t LogBlockNbr;  /*!< Specifies the Card logical Capacity in blocks */
  uint32_t LogBlockSize; /*!< Specifies logical block size in bytes */
} BSP_SD_SPI_CardInfo;

/*****************************************************************************
 *      PUBLIC DEFINES
 *****************************************************************************/

/* MMC/SD command */
#define CMD0   (0)         /* GO_IDLE_STATE */
#define CMD1   (1)         /* SEND_OP_COND (MMC) */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8   (8)         /* SEND_IF_COND */
#define CMD9   (9)         /* SEND_CSD */
#define CMD10  (10)        /* SEND_CID */
#define CMD12  (12)        /* STOP_TRANSMISSION */
#define ACMD13 (0x80 + 13) /* SD_STATUS (SDC) */
#define CMD16  (16)        /* SET_BLOCKLEN */
#define CMD17  (17)        /* READ_SINGLE_BLOCK */
#define CMD18  (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)        /* SET_BLOCK_COUNT (MMC) */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)        /* WRITE_BLOCK */
#define CMD25  (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32  (32)        /* ERASE_ER_BLK_START */
#define CMD33  (33)        /* ERASE_ER_BLK_END */
#define CMD41  (0x40 + 41) /* SEND_OP_COND (ACMD) */
#define CMD38  (38)        /* ERASE */
#define CMD55  (55)        /* APP_CMD */
#define CMD58  (58)        /* READ_OCR */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC   0x01 /* MMC ver 3 */
#define CT_SD1   0x02 /* SD ver 1 */
#define CT_SD2   0x04 /* SD ver 2 */
#define CT_SDC   0x06 /* SD */
#define CT_BLOCK 0x08 /* Block addressing */

/**
 * @brief  SD status structure definition
 */
#define MSD_OK                   ((uint8_t)0x00)
#define MSD_ERROR                ((uint8_t)0x01)
#define MSD_ERROR_SD_NOT_PRESENT ((uint8_t)0x02)

/**
 * @brief  SD transfer state definition
 */
#define SD_TRANSFER_OK   ((uint8_t)0x00)
#define SD_TRANSFER_BUSY ((uint8_t)0x01)

#define SD_PRESENT     ((uint8_t)0x01)
#define SD_NOT_PRESENT ((uint8_t)0x00)
#define SD_DATATIMEOUT ((uint32_t)100000000)
/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/

uint8_t BSP_sdSpiInit(void);
uint8_t BSP_sdSpiITConfig(void);
uint8_t BSP_sdSpiReadBlocks(uint8_t *p_data,
                            uint32_t u32_readAddr,
                            uint32_t u32_numOfBlocks,
                            uint32_t u32_timeout);
uint8_t BSP_sdSpiWriteBlocks(uint32_t *p_data,
                             uint32_t  u32_writeAddr,
                             uint32_t  u32_numOfBlocks,
                             uint32_t  u32_timeout);

// uint8_t BSP_sdSpiReadBlocks_DMA(uint32_t *pData,
//                                 uint32_t  ReadAddr,
//                                 uint32_t  NumOfBlocks);
// uint8_t BSP_sdSpiWriteBlocks_DMA(uint32_t *pData,
//                                  uint32_t  WriteAddr,
//                                  uint32_t  NumOfBlocks);

uint8_t BSP_sdSpiErase(uint32_t StartAddr, uint32_t EndAddr);
uint8_t BSP_sdSpiGetCardState(void);
void    BSP_sdSpiGetCardInfo(BSP_SD_SPI_CardInfo *CardInfo);
uint8_t BSP_sdSpiIsDetected(void);

/* These functions can be modified in case the current settings (e.g. DMA
   stream) need to be changed for specific application needs */
void BSP_sdSpiAbortCallback(void);
void BSP_sdSpiWriteCpltCallback(void);
void BSP_sdSpiReadCpltCallback(void);

#endif /* SDSPI_H_ */