#ifndef CRC16_H
#define CRC16_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/
  /**
   * @brief Computes the CRC-16 checksum for the given data.
   *
   * This function calculates the CRC-16 checksum for a block of data using a
   * specified polynomial. The CRC-16 checksum is commonly used for
   * error-checking in data transmission and storage.
   *
   * @param data Pointer to the data buffer for which the CRC-16 checksum is to
   * be calculated.
   * @param length Length of the data buffer in bytes.
   * @return The computed CRC-16 checksum.
   */
  uint16_t MID_EncodeCRC16(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* CRC16_H*/