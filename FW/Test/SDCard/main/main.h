#ifndef MAIN_H_
#define MAIN_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "spi.h"
#ifdef __cplusplus
extern "C"
{
#endif

  /******************************************************************************
   *    PUBLIC VARIABLES
   *****************************************************************************/

   extern spi_device_handle_t spi_sdcard_handle;

  /******************************************************************************
   *    PUBLIC DEFINES
   *****************************************************************************/

#define HSPI_HOST       SPI2_HOST
#define MISO_PIN        11
#define MOSI_PIN        13
#define SCLK_PIN        12
#define CS_PIN          48
#define MAX_TRANSFER_SZ 4096
#define SPI_MODE        0

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_ */