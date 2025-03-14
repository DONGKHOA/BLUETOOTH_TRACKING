#ifndef CAN_H_
#define CAN_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/twai.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************
   *      PUBLIC TYPEDEFS
   *****************************************************************************/

  typedef enum
  {
    CAN_25KBITS,  // 0
    CAN_50KBITS,  // 1
    CAN_100KBITS, // 2
    CAN_125KBITS, // 3
    CAN_250KBITS, // 4
    CAN_500KBITS, // 5
    CAN_800KBITS, // 6
    CAN_1MBITS    // 7
  } can_bit_rate_t;

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  /**
   * @brief Initialize the CAN driver.
   *
   * This function initializes the CAN driver and prepares it for operation.
   *
   * @return
   *      - ESP_OK: Success
   *      - ESP_FAIL: Failed to initialize the CAN driver
   */
  esp_err_t BSP_canDriverInit(void);

  /**
   * @brief Configure the CAN driver with default settings.
   *
   * This function sets the CAN driver to its default configuration.
   */
  void BSP_canConfigDefault(void);

  /**
   * @brief Configure the CAN driver mode.
   *
   * @param e_can_mode The CAN mode to be set.
   *
   * This function configures the CAN driver to operate in the specified mode.
   */
  void BSP_canConfigMode(twai_mode_t e_can_mode);

  /**
   * @brief Configure the CAN driver I/O pins.
   *
   * @param e_tx_pin The GPIO pin to be used for CAN TX.
   * @param e_rx_pin The GPIO pin to be used for CAN RX.
   *
   * This function configures the GPIO pins to be used for CAN transmission and
   * reception.
   */
  void BSP_canConfigIO(gpio_num_t e_tx_pin, gpio_num_t e_rx_pin);

  /**
   * @brief Configure the CAN driver queue lengths.
   *
   * @param u32_tx_queue_len Length of the TX queue.
   * @param u32_rx_queue_len Length of the RX queue.
   *
   * This function sets the lengths of the transmission and reception queues.
   */
  void BSP_canConfigQueue(uint32_t u32_tx_queue_len, uint32_t u32_rx_queue_len);

  /**
   * @brief Configure the CAN driver interrupt flags.
   *
   * @param i_intr_flag Interrupt flags to be set.
   *
   * This function configures the interrupt flags for the CAN driver.
   */
  void BSP_canConfigIntr(int i_intr_flag);

  /**
   * @brief Configure the CAN driver bit rate.
   *
   * @param e_bitrate The bit rate to be set.
   *
   * This function sets the bit rate for the CAN driver.
   */
  void BSP_canConfigBitRate(can_bit_rate_t e_bitrate);

  /**
   * @brief Start the CAN driver.
   *
   * This function starts the CAN driver, enabling CAN communication.
   *
   * @return
   *      - ESP_OK: Success
   *      - ESP_FAIL: Failed to start the CAN driver
   */
  static inline esp_err_t BSP_canStart (void)
  {
    return twai_start();
  }

  /**
   * @brief Stop the CAN driver.
   *
   * This function stops the CAN driver, disabling CAN communication.
   *
   * @return
   *      - ESP_OK: Success
   *      - ESP_FAIL: Failed to stop the CAN driver
   */
  static inline esp_err_t BSP_canStop (void)
  {
    return twai_stop();
  }

  /**
   * @brief Transmit a CAN message.
   *
   * @param p_message Pointer to the CAN message to be transmitted.
   * @param u32_timeout Timeout for the transmission.
   *
   * This function transmits a CAN message.
   *
   * @return
   *      - ESP_OK: Success
   *      - ESP_FAIL: Failed to transmit the CAN message
   */
  static inline esp_err_t BSP_canTransmit (twai_message_t *p_message,
                                           TickType_t      u32_timeout)
  {
    return twai_transmit(p_message, u32_timeout);
  }

  /**
   * @brief Receive a CAN message.
   *
   * @param p_message Pointer to the CAN message to be received.
   * @param u32_timeout Timeout for the reception.
   *
   * This function receives a CAN message.
   *
   * @return
   *      - ESP_OK: Success
   *      - ESP_FAIL: Failed to receive the CAN message
   */
  static inline esp_err_t BSP_canReceive (twai_message_t *p_message,
                                          TickType_t      u32_timeout)
  {
    return twai_receive(p_message, u32_timeout);
  }

  /**
   * @brief Get the status of the CAN driver.
   *
   * @param status_info Pointer to the structure to store the status
   * information.
   *
   * This function retrieves the current status of the CAN driver.
   *
   * @return
   *      - ESP_OK: Success
   *      - ESP_FAIL: Failed to get the CAN driver status
   */
  static inline esp_err_t BSP_canGetStatus (twai_status_info_t *status_info)
  {
    return twai_get_status_info(status_info);
  }

#ifdef __cplusplus
}
#endif

#endif