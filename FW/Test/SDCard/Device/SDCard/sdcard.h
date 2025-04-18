#ifndef SDCARD_H_
#define SDCARD_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "spi.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

#define MSD_OK                   0x00
#define MSD_ERROR                0x01
#define MSD_ERROR_SD_NOT_PRESENT 0x02
#define SD_TRANSFER_OK           0x00
#define SD_TRANSFER_BUSY         0x01
#define SD_PRESENT               0x01
#define SD_NOT_PRESENT           0x00

  /*****************************************************************************
   *      PUBLIC TYPEDEFS
   *****************************************************************************/
  typedef struct
  {
    uint32_t CardType;
    uint32_t LogBlockNbr;
    uint32_t LogBlockSize;
  } SDCard_Info_t;

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  /**
   * @brief Initializes the SD card.
   *
   * This function initializes the SD card by setting up the SPI interface and
   * check type of SD card.
   *
   * @param e_spi_host The SPI host device to be used for communication with the
   * SD card.
   * @param e_cs_io The GPIO pin number used for chip select (CS) to communicate
   * with the SD card.
   *
   * @return Returns 0 on success (MSD_OK), or an error code on failure.
   */
  uint8_t DEV_SDCard_Init(spi_device_handle_t *spi_handle,
                          spi_host_device_t   e_spi_host,
                          gpio_num_t          e_cs_io);

  /**
   * @brief Reads single block or multiple block of data from the SD card.
   *
   * This function reads a specified number of blocks from the SD card starting
   * from a given address and stores the data in the provided buffer.
   *
   * @param p_data Pointer to the buffer where the read data will be stored.
   * @param u32_readAddr The starting address on the SD card from where the data
   *                     will be read.
   * @param u32_numOfBlocks The number of blocks to read from the SD card.
   * @param e_cs_io The GPIO pin number used for chip select (CS) to communicate
   *                with the SD card.
   *
   * @return Returns 0 on success (MSD_OK), or an error code on failure.
   */
  uint8_t DEV_SDCard_ReadBlock(spi_device_handle_t spi_handle,
                               uint8_t            *p_data,
                               uint32_t            u32_readAddr,
                               uint32_t            u32_numOfBlocks,
                               gpio_num_t          e_cs_io);

  /**
   * @brief Writes single block or multiple block of data to the SD card.
   *
   * This function writes a specified number of blocks of data to the SD card at
   * a given address.
   *
   * @param p_data Pointer to the data to be written.
   * @param u32_writeAddr Address on the SD card where the data will be written.
   * @param u32_numOfBlocks Number of blocks to write.
   * @param u32_timeout Timeout duration for the write operation.
   * @param e_cs_io GPIO pin used for chip select.
   * @return  SReturns 0 on success (MSD_OK), or an error code on failure.
   */
  uint8_t DEV_SDCard_WriteBlock(spi_device_handle_t spi_handle,
                                uint8_t            *p_data,
                                uint32_t            u32_writeAddr,
                                uint32_t            u32_numOfBlocks,
                                uint32_t            u32_timeout,
                                gpio_num_t          e_cs_io);

  /**
   * @brief Retrieves information about the SD card.
   *
   * This function populates the provided SDCard_Info_t structure with
   * information about the SD card.
   *
   * @param p_CardInfo Pointer to an SDCard_Info_t structure that will be filled
   * with the card information.
   */
  void DEV_SDCard_Info(SDCard_Info_t *p_CardInfo);

#ifdef __cplusplus
}
#endif

#endif /* SDCARD_H_ */