#ifndef AS608_H
#define AS608_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "uart.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTIONS
   ***************************************************************************/

  /**
   * @brief Function to get responses from AS608 fingerprint module
   *
   * @param p_instruction_code Instruction code for each command
   * @return Returns confirmation code received from the response
   */
  uint8_t DEV_AS608_Reponse(uart_port_num_t e_uart_port,
                            uint8_t        p_instruction_code);

  /**
   * @brief Function to detect finger and store image in imagebuffer
   *
   * @param p_AS608_address Current module address
   * @return Returns received confirmation code from module
   */
  uint8_t DEV_AS608_GenImg(uart_port_num_t e_uart_port,
                           uint8_t        *p_AS608_address);

  /**
   * @brief Function to generate character file from image in image buffer and
   * store in charbuffer1/charbuffer2
   *
   * @param p_AS608_address Current module address
   * @param p_buffer_id Buffer id ( character file buffer number )
   * @return Returns received confirmation code from module
   */
  uint8_t DEV_AS608_Img2Tz(uart_port_num_t e_uart_port,
                           uint8_t        *p_AS608_address,
                           uint8_t        *p_buffer_id);

  /**
   * @brief Function to search whole library for template that matches
   * charbuffer1/charbuffer2
   *
   * @param p_AS608_address Current module address
   * @param p_buffer_id Buffer id ( character file buffer number )
   * @param p_start_page Start address for search operation
   * @param p_page_number Searching number
   * @return Returns received confirmation code from module
   */
  uint8_t DEV_AS608_Search(uart_port_num_t e_uart_port,
                           uint8_t        *p_AS608_address,
                           uint8_t        *p_buffer_id,
                           uint8_t        *p_start_page,
                           uint8_t        *p_page_number);

  /**
   * @brief Function to combine both character files and generate template,
   * store in charbuffer1 & charbuffer2
   * @param p_AS608_address Current module address
   * @return Returns received confirmation code from module
   */
  uint8_t DEV_AS608_RegModel(uart_port_num_t e_uart_port,
                             uint8_t        *p_AS608_address);

  /**
   * @brief Function to store template to specified buffer at desired flash
   * location
   *
   * @param p_AS608_address Current module address
   * @param p_buffer_id Buffer id (character file buffer number)
   * @param p_page_id Flash location of the template
   * @return Returns received confirmation code from module
   */
  uint8_t DEV_AS608_Store(uart_port_num_t e_uart_port,
                          uint8_t        *p_AS608_address,
                          uint8_t        *p_buffer_id,
                          uint8_t        *p_page_id);

  /**
   * @brief Function to read current valid template number
   *
   * @param p_AS608_address Current module address
   * @return Returns received confirmation code from module
   */
  uint8_t DEV_AS608_TempleteNum(uart_port_num_t e_uart_port,
                                uint8_t        *p_AS608_address,
                                uint16_t       *p_stored_fingerprints);

  /**
   * @brief Function to delete N segment of templates of flash starting from
   * desired location
   *
   * @param p_AS608_address Current module address
   * @param p_page_id Flash location of the template
   * @param p_number_of_templates N : Number of templates to be deleted
   * @return Returns received confirmation code from module
   */
  uint8_t DEV_AS608_DeleteChar(uart_port_num_t e_uart_port,
                              uint8_t        *p_AS608_address,
                              uint8_t        *p_page_id,
                              uint8_t        *p_number_of_templates);

  void DEV_AS608_TimeOut(void);

#ifdef __cplusplus
}
#endif

#endif /* AS608_H*/