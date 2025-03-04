#ifndef MAIN_H_
#define MAIN_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

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