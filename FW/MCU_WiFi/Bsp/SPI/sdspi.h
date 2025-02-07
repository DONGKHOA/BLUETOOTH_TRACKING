#ifndef SDSPI_H_
#define SDSPI_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdkconfig.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

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
  } BSP_SD_SPI_CardInfo;

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/
  /**
   * @brief Initialize the SPI interface for the SD card.
   *
   * This function sets up the SPI interface with the specified parameters to
   * communicate with an SD card.
   *
   * @param e_spi_host        The SPI host device to use.
   * @param e_miso_io         The GPIO number for the MISO (Master In Slave Out)
   * line.
   * @param e_mosi_io         The GPIO number for the MOSI (Master Out Slave In)
   * line.
   * @param e_sclk_io         The GPIO number for the SCLK (Serial Clock) line.
   * @param i_max_transfer_sz The maximum transfer size in bytes.
   * @param e_cs_io           The GPIO number for the CS (Chip Select) line.
   * @param i_dma_channel     The DMA channel to use.
   * @param i_clock_init_hz   The initial clock frequency in Hz.
   * @param i_clock_work_hz   The working clock frequency in Hz.
   * @param u8_spi_mode       The SPI mode (0-3).
   *
   * @return uint8_t Returns 0 on success, or an error code on failure.
   */
  uint8_t BSP_sdSpiInit(spi_host_device_t e_spi_host,
                        gpio_num_t        e_miso_io,
                        gpio_num_t        e_mosi_io,
                        gpio_num_t        e_sclk_io,
                        int               i_max_transfer_sz,
                        gpio_num_t        e_cs_io,
                        int               i_dma_channel,
                        int               i_clock_init_hz,
                        int               i_clock_work_hz,
                        uint8_t           u8_spi_mode);

  /**
   * @brief Reads blocks of data from the SD card via SPI.
   *
   * This function reads a specified number of blocks from the SD card starting
   * from a given address and stores the data in the provided buffer.
   *
   * @param p_data Pointer to the buffer where the read data will be stored.
   * @param readAddr Address from where the read operation will start.
   * @param numOfBlocks Number of blocks to read.
   * @param timeout Timeout duration for the read operation.
   * @param e_cs_io GPIO pin number for the chip select (CS) line.
   *
   * @return Status of the read operation.
   *         - 0: Success
   *         - Non-zero: Error code
   */
  uint8_t BSP_sdSpiReadBlocks(uint8_t   *p_data,
                              uint32_t   readAddr,
                              uint32_t   numOfBlocks,
                              uint32_t   timeout,
                              gpio_num_t e_cs_io);

  /**
   * @brief  Writes blocks of data to the SD card via SPI.
   *
   * This function writes a specified number of blocks to the SD card starting
   * from a given address. The operation is performed using SPI communication.
   *
   * @param  p_data      Pointer to the data buffer to be written.
   * @param  writeAddr   Address on the SD card where the write operation
   * starts.
   * @param  numOfBlocks Number of blocks to write.
   * @param  timeout     Timeout duration for the write operation.
   * @param  e_cs_io     GPIO pin number used for chip select (CS).
   *
   * @retval uint8_t     Returns 0 on success, non-zero error code on failure.
   */
  uint8_t BSP_sdSpiWriteBlocks(uint8_t   *p_data,
                               uint32_t   writeAddr,
                               uint32_t   numOfBlocks,
                               uint32_t   timeout,
                               gpio_num_t e_cs_io);

  /**
   * @brief  Retrieves the SD card information.
   * @param  CardInfo: Pointer to a BSP_SD_SPI_CardInfo structure that will hold
   * the card information.
   * @retval None
   */
  void BSP_sdSpiGetCardInfo(BSP_SD_SPI_CardInfo *CardInfo);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SD_SPI_H_ */
