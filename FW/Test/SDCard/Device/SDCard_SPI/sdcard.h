#ifndef SDCARD_H_
#define SDCARD_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"
#include "sdspi.h"

/*****************************************************************************
 *      PUBLIC DEFINES
 *****************************************************************************/

#define HSPI_HOST       SPI2_HOST
#define MISO_PIN        11
#define MOSI_PIN        13
#define SCLK_PIN        12
#define CS_PIN          48
#define MAX_TRANSFER_SZ 4096
#define SPI_MODE        0
#define DMA_CHANNEL     3

#define SD_INIT_FREQ 400000
#define SD_WORK_FREQ 10000000

/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/

/**
 * @brief Initializes the SD card.
 *
 * This function sets up the necessary configurations and initializes the SD
 * card for use. It prepares the SD card interface for read and write
 * operations.
 */
void DEV_SDCard_Init(void);

/**
 * @brief Writes data to a file on the SD card.
 *
 * This function writes the specified data to a file on the SD card. If the file
 * does not exist, it will be created. If the file already exists, its contents
 * will be overwritten.
 *
 * @param p_filename Pointer to a null-terminated string that specifies the name
 *                   of the file to write to.
 * @param p_data     Pointer to a null-terminated string that contains the data
 *                   to be written to the file.
 */
void DEV_SDCard_WriteFile(const char *p_filename, const char *p_data);

/**
 * @brief Reads the contents of a file from the SD card.
 *
 * This function opens the file specified by the given path and reads its
 * contents. The path should be a null-terminated string representing the
 * location of the file on the SD card.
 *
 * @param p_path A pointer to a null-terminated string that specifies the path
 * of the file to be read.
 */
void DEV_SDCard_Read_File(const char *p_path);

#endif /* SDCARD_H_ */