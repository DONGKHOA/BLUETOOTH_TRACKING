/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_log.h"

#include "as608.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_15)

#define UART_NUM  UART_NUM_2
#define BAUD_RATE 115200

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static uint16_t DEV_AS608_Checksum(uint8_t *p_tx_cmd_data,
                                   uint8_t *p_AS608_data);

static uint16_t DEV_AS608_Checksum_Search(uint8_t *p_tx_cmd_data,
                                          uint8_t *p_AS608_data);

static uint16_t DEV_AS608_Checksum_Store(uint8_t *p_tx_cmd_data,
                                         uint8_t *p_AS608_data);

static void DEV_AS608_Response_Parser(uint8_t  u8_instruction_code,
                                      uint8_t *p_received_package);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
DEV_AS608_Init (void)
{
  BSP_uartDriverInit(UART_NUM,
                     TXD_PIN,
                     RXD_PIN,
                     BAUD_RATE,
                     UART_DATA_8_BITS,
                     UART_PARITY_DISABLE,
                     UART_HW_FLOWCTRL_DISABLE,
                     UART_STOP_BITS_1);
}

uint8_t
DEV_AS608_Reponse (uint8_t *p_instruction_code)
{
  uint8_t   u8_received_confirmation_code = 0;
  uint8_t  *p_received_package            = (uint8_t *)malloc(RX_BUF_SIZE + 1);
  const int rxBytes                       = uart_read_bytes(
      UART_NUM, p_received_package, RX_BUF_SIZE, pdMS_TO_TICKS(500));

  if (rxBytes > 0)
  {
    p_received_package[rxBytes] = 0;
    printf("Read %d bytes:", rxBytes);
    for (int i = 0; i < rxBytes; i++)
    {
      printf("0x%02X ", p_received_package[i]);
    }
    printf("\r\n");

    // Pass the received response to the parser function
    DEV_AS608_Response_Parser(p_instruction_code, p_received_package);

    // Get the Confirmation Code from received from the response
    u8_received_confirmation_code = p_received_package[9];
  }

  free(p_received_package);

  return u8_received_confirmation_code;
}

uint8_t
DEV_AS608_VfyPwd (uint8_t *p_AS608_address, uint8_t *p_vfy_password)
{
  uint8_t u8_tx_cmd_data[16]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x13 };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_confirmation_code = 0;

  // Get the checksum result
  uint16_t u16_checksum_value
      = DEV_AS608_Checksum(u8_tx_cmd_data, p_vfy_password);

  u8_check_sum_data[0]
      = (u16_checksum_value >> 8) & (0xFF); // Split checksum in Higher bits
  u8_check_sum_data[1]
      = u16_checksum_value & (0xFF); // Split checksum in Lower bits

  // Loop to add module address and password
  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2]  = p_AS608_address[i];
    u8_tx_cmd_data[i + 10] = p_vfy_password[i];

    // Loop to add checksum to the packet
    if (i < 2)
    {
      u8_tx_cmd_data[i + 14] = u8_check_sum_data[i];
    }
  }

  uint8_t u8_instruction_code = 0;

  // Get the Instruction Code (0x13)
  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  vTaskDelay(500 / portTICK_PERIOD_MS);

  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

uint8_t
DEV_AS608_GenImg (uint8_t *p_AS608_address)
{
  uint8_t u8_tx_cmd_data[12]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01 };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_confirmation_code = 0;

  BSP_resetBuffer(UART_NUM);
  vTaskDelay(50 / portTICK_PERIOD_MS);

  uint16_t u16_checksum_value
      = DEV_AS608_Checksum(u8_tx_cmd_data, (uint8_t *)"#12");
  u8_check_sum_data[0] = (u16_checksum_value >> 8) & (0xFF);
  u8_check_sum_data[1] = u16_checksum_value & (0xFF);

  // Loop to add module address and checksum
  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2] = p_AS608_address[i];
    if (i < 2)
    {
      u8_tx_cmd_data[i + 10] = u8_check_sum_data[i];
    }
  }

  uint8_t u8_instruction_code;

  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  vTaskDelay(500 / portTICK_PERIOD_MS);

  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

uint8_t
DEV_AS608_Img2Tz (uint8_t *p_AS608_address, uint8_t *p_buffer_id)
{
  uint8_t u8_tx_cmd_data[13]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02 };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_confirmation_code = 0;

  BSP_resetBuffer(UART_NUM);
  vTaskDelay(50 / portTICK_PERIOD_MS);

  uint16_t u16_checksum_value = DEV_AS608_Checksum(u8_tx_cmd_data, p_buffer_id);
  u8_check_sum_data[0]        = (u16_checksum_value >> 8) & (0xFF);
  u8_check_sum_data[1]        = u16_checksum_value & (0xFF);

  u8_tx_cmd_data[10] = p_buffer_id[0];
  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2] = p_AS608_address[i];
    if (i < 2)
    {
      u8_tx_cmd_data[i + 11] = u8_check_sum_data[i];
    }
  }

  uint8_t u8_instruction_code;

  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  vTaskDelay(500 / portTICK_PERIOD_MS);

  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

uint8_t
DEV_AS608_Search (uint8_t *p_AS608_address,
                  uint8_t *p_buffer_id,
                  uint8_t *p_start_page,
                  uint8_t *p_page_number)
{
  uint8_t u8_tx_cmd_data[17]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04 };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_combined_data[5]  = { 0 };
  uint8_t u8_confirmation_code = 0;

  BSP_resetBuffer(UART_NUM);
  vTaskDelay(50 / portTICK_PERIOD_MS);

  memset(u8_combined_data, 0, sizeof(u8_combined_data));
  u8_combined_data[0] = p_buffer_id[0];
  for (int i = 0; i < 2; i++)
  {
    u8_combined_data[i + 1] = p_start_page[i];
    u8_combined_data[i + 3] = p_page_number[i];
  }

  uint16_t u16_checksum_value
      = DEV_AS608_Checksum_Search(u8_tx_cmd_data, u8_combined_data);

  u8_check_sum_data[0] = (u16_checksum_value >> 8) & (0xFF);
  u8_check_sum_data[1] = u16_checksum_value & (0xFF);

  u8_tx_cmd_data[10] = p_buffer_id[0];
  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2] = p_AS608_address[i];
    if (i < 2)
    {
      u8_tx_cmd_data[i + 11] = p_start_page[i];
      u8_tx_cmd_data[i + 13] = p_page_number[i];
    }
  }
  u8_tx_cmd_data[15] = u8_check_sum_data[0];
  u8_tx_cmd_data[16] = u8_check_sum_data[1];

  uint8_t u8_instruction_code;

  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  vTaskDelay(500 / portTICK_PERIOD_MS);

  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

uint8_t
DEV_AS608_RegModel (uint8_t *p_AS608_address)
{
  uint8_t u8_tx_cmd_data[12]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05 };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_confirmation_code = 0;

  BSP_resetBuffer(UART_NUM);
  vTaskDelay(50 / portTICK_PERIOD_MS);

  uint16_t u16_checksum_value
      = DEV_AS608_Checksum(u8_tx_cmd_data, (uint8_t *)"#12");
  u8_check_sum_data[0] = (u16_checksum_value >> 8) & (0xFF);
  u8_check_sum_data[1] = u16_checksum_value & (0xFF);

  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2] = p_AS608_address[i];
    if (i < 2)
    {
      u8_tx_cmd_data[i + 10] = u8_check_sum_data[i];
    }
  }

  uint8_t u8_instruction_code;

  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  vTaskDelay(500 / portTICK_PERIOD_MS);

  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

uint8_t
DEV_AS608_Store (uint8_t *p_AS608_address,
                 uint8_t *p_buffer_id,
                 uint8_t *p_page_id)
{
  uint8_t u8_tx_cmd_data[15]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06 };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_combined_data[3]  = { 0x00, 0x00, 0x00 };
  uint8_t u8_confirmation_code = 0;

  memset(u8_combined_data, 0, sizeof(u8_combined_data));
  memcpy(u8_combined_data, p_buffer_id, 1);
  memcpy(u8_combined_data + 1, p_page_id, 2);

  uint16_t u16_checksum_value
      = DEV_AS608_Checksum_Store(u8_tx_cmd_data, u8_combined_data);
  ESP_LOGI("Store | u16_checksum_value", "checksum: %d", u16_checksum_value);
  u8_check_sum_data[0] = (u16_checksum_value >> 8) & (0xFF);
  u8_check_sum_data[1] = u16_checksum_value & (0xFF);

  memcpy(u8_tx_cmd_data + 10, u8_combined_data, 3);

  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2] = p_AS608_address[i];
    if (i < 2)
    {
      u8_tx_cmd_data[i + 13] = u8_check_sum_data[i];
    }
  }
  ESP_LOG_BUFFER_HEXDUMP(
      "Store | u8_tx_cmd_data", u8_tx_cmd_data, 15, ESP_LOG_INFO);

  uint8_t u8_instruction_code;

  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  vTaskDelay(500 / portTICK_PERIOD_MS);

  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

uint8_t
DEV_AS608_TempleteNum (uint8_t  *p_AS608_address,
                       uint16_t *p_stored_fingerprints)
{
  uint8_t u8_tx_cmd_data[12]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x1D };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_confirmation_code = 0;

  uint16_t u16_checksum_value
      = DEV_AS608_Checksum(u8_tx_cmd_data, (uint8_t *)"#12");
  u8_check_sum_data[0] = (u16_checksum_value >> 8) & (0xFF);
  u8_check_sum_data[1] = u16_checksum_value & (0xFF);

  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2] = p_AS608_address[i];
    if (i < 2)
    {
      u8_tx_cmd_data[i + 10] = u8_check_sum_data[i];
    }
  }

  uint8_t u8_instruction_code;

  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  // Export the number of stored fingerprints
  uint8_t response[14] = { 0 };
  int     rxBytes      = uart_read_bytes(
      UART_NUM, response, sizeof(response), pdMS_TO_TICKS(100));

  if (rxBytes >= 12 && response[9] == 0x00)
  {
    *p_stored_fingerprints
        = (response[10] << 8) | response[11]; // Combinned 2 Bytes to ID
    printf("Stored Fingerprints: %d\n", *p_stored_fingerprints);
  }

  vTaskDelay(500 / portTICK_PERIOD_MS);

  // Get the Confirmation Code from received from the response
  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

uint8_t
DEV_AS608_DeletChar (uint8_t *p_AS608_address,
                     uint8_t *p_page_id,
                     uint8_t *p_number_of_templates)
{
  uint8_t u8_tx_cmd_data[16]
      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C };
  uint8_t u8_check_sum_data[2] = { 0x00, 0x00 };
  uint8_t u8_combined_data[4]  = { 0 };
  uint8_t u8_confirmation_code = 0;

  memset(u8_combined_data, 0, sizeof(u8_combined_data));
  for (int i = 0; i < 2; i++)
  {
    u8_combined_data[i]     = p_page_id[i];
    u8_combined_data[i + 2] = p_number_of_templates[i];
  }

  uint16_t u16_checksum_value
      = DEV_AS608_Checksum(u8_tx_cmd_data, u8_combined_data);
  u8_check_sum_data[0] = (u16_checksum_value >> 8) & (0xFF);
  u8_check_sum_data[1] = u16_checksum_value & (0xFF);

  for (int i = 0; i < 4; i++)
  {
    u8_tx_cmd_data[i + 2] = p_AS608_address[i];
    if (i < 2)
    {
      u8_tx_cmd_data[i + 10] = p_page_id[i];
      u8_tx_cmd_data[i + 12] = p_number_of_templates[i];
      u8_tx_cmd_data[i + 14] = u8_check_sum_data[i];
    }
  }

  uint8_t u8_instruction_code;

  u8_instruction_code = u8_tx_cmd_data[9];

  // Send entire packet over UART
  const int package_length = sizeof(u8_tx_cmd_data);
  BSP_uartSendData(UART_NUM, u8_tx_cmd_data, package_length);

  vTaskDelay(500 / portTICK_PERIOD_MS);

  u8_confirmation_code = DEV_AS608_Reponse(u8_instruction_code);

  return u8_confirmation_code;
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

uint16_t
DEV_AS608_Checksum (uint8_t *p_tx_cmd_data, uint8_t *p_AS608_data)
{
  uint16_t u16_result = 0;
  if (p_AS608_data[0] == '#') // Check whether to perform checksum for packet
                              //  without extra data inputs
  {
    char c_length[4];
    sprintf(c_length, "%c%c", p_AS608_data[1], p_AS608_data[2]);

    for (int i = 0; i < atoi(c_length) - 8; i++)
    {
      u16_result = u16_result + p_tx_cmd_data[i + 6];
    }
  }
  else // Check whether to perform checksum for packet with extra data inputs
  {
    for (int i = 0; i < 4; i++)
    {
      u16_result = u16_result + p_tx_cmd_data[i + 6];

      if (i < sizeof(p_AS608_data))
      {
        u16_result = u16_result + p_AS608_data[i];
      }
    }
  }

  if (u16_result <= 256)
  {
    u16_result = u16_result % 256;
  }

  return u16_result;
}

uint16_t
DEV_AS608_Checksum_Search (uint8_t *p_tx_cmd_data, uint8_t *p_AS608_data)
{
  uint16_t u16_result = 0;

  for (int i = 0; i < 4; i++)
  {
    u16_result = u16_result + p_tx_cmd_data[i + 6];
  }

  for (int i = 0; i < 5; i++)
  {
    u16_result = u16_result + p_AS608_data[i];
  }

  if (u16_result <= 256)
  {
    u16_result = u16_result % 256;
  }

  return u16_result;
}

uint16_t
DEV_AS608_Checksum_Store (uint8_t *p_tx_cmd_data, uint8_t *p_AS608_data)
{
  uint16_t u16_result = 0;

  for (int i = 0; i < 4; i++)
  {
    u16_result = u16_result + p_tx_cmd_data[i + 6];
    if (i < 3)
    {
      u16_result = u16_result + p_AS608_data[i];
    }
  }

  if (u16_result <= 256)
  {
    u16_result = u16_result % 256;
  }

  return u16_result;
}

void
DEV_AS608_Response_Parser (uint8_t  u8_instruction_code,
                           uint8_t *p_received_package)
{
  // Get Confirmation Code from received response packet
  uint8_t u8_confirmation_code = p_received_package[9];

  if (u8_instruction_code == 0x13)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("VfyPwd", "(0x00H) CORRECT PASSWORD");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("VfyPwd", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x13)
    {
      ESP_LOGE("VfyPwd", "(0x13H) WRONG PASSWORD");
    }
  }

  if (u8_instruction_code == 0x12)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("SetPwd", "(0x00H) NEW PASSWORD COMPLETE");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("SetPwd", "(0x01H) ERROR RECEIVING PACKAGE");
    }
  }

  if (u8_instruction_code == 0x15)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("SetAdder", "(0x00H) ADDRESS SETTING COMPLETE");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("SetAdder", "(0x01H) ERROR RECEIVING PACKAGE");
    }
  }

  if (u8_instruction_code == 0x17)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("PortControl", "(0x00H) PORT OPERATION COMPLETE");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("PortControl", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x1D)
    {
      ESP_LOGE("PortControl", "(0x01DH) FAIL TO OPERATE PORT");
    }
  }

  if (u8_instruction_code == 0x0F)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("ReadSysPara", "(0x00H) SYSTEM READ COMPLETE");

      ESP_LOGW("SYSTEM PARAMETER",
               "Library Size - %d",
               (p_received_package[14] << 8) | p_received_package[15]);
      ESP_LOGW(
          "SYSTEM PARAMETER", "Security Level - %x", p_received_package[17]);
      ESP_LOGW("SYSTEM PARAMETER",
               "32bit Address - %x:%x:%x:%x",
               p_received_package[18],
               p_received_package[19],
               p_received_package[20],
               p_received_package[21]);
      ESP_LOGW("SYSTEM PARAMETER", "Size Code - %x", p_received_package[23]);
      ESP_LOGW("SYSTEM PARAMETER", "N - %x", p_received_package[25]);
      ESP_LOGW("SYSTEM PARAMETER",
               "BAUD - %d",
               ((p_received_package[24] << 8) | p_received_package[25]) * 9600);
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("ReadSysPara", "(0x01H) ERROR RECEIVING PACKAGE");
    }
  }

  if (u8_instruction_code == 0x1D)
  {
    if (u8_confirmation_code == 0x00)
    {
      uint16_t u16_template_number = 0;
      ESP_LOGI("TempleteNum", "(0x00H) READ COMPLETE");

      u16_template_number = p_received_package[10] + p_received_package[11];
      ESP_LOGW("SYSTEM PARAMETER", "Template Number - %d", u16_template_number);
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("TempleteNum", "(0x01H) ERROR RECEIVING PACKAGE");
    }
  }

  if (u8_instruction_code == 0x32 || u8_instruction_code == 0x34)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("GR_Auto", "(0x00H) READ COMPLETE");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("GR_Auto/GR_Identify", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x06)
    {
      ESP_LOGE("GR_Auto/GR_Identify",
               "(0x06H) FINGERPRINT IMAGE GENERATION FAIL");
    }
    else if (u8_confirmation_code == 0x07)
    {
      ESP_LOGE("GR_Auto/GR_Identify",
               "(0x07H) FINGERPRINT IMAGE GENERATION FAIL");
    }
    else if (u8_confirmation_code == 0x09)
    {
      ESP_LOGE("GR_Auto/GR_Identify", "(0x09H) NO MATCHING FINGERPRINT");
    }
  }

  if (u8_instruction_code == 0x01)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("GenImg", "(0x00H) FINGER COLLECTION SUCCESS");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("GenImg", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x02)
    {
      ESP_LOGE("GenImg", "(0x02H) NO FINGER DETECED");
    }
    else if (u8_confirmation_code == 0x03)
    {
      ESP_LOGE("GenImg", "(0x03H) FAIL TO COLLECT FINGER");
    }
  }

  if (u8_instruction_code == 0x0A)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("UpImage", "(0x00H) READY TO TRANSFER PACKET");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("UpImage", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x0F)
    {
      ESP_LOGE("UpImage", "(0x0FH) FAILED TO TRANSFER PACKET");
    }
  }

  if (u8_instruction_code == 0x0B)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("DownImage", "(0x00H) READY TO TRANSFER PACKET");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("DownImage", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x0E)
    {
      ESP_LOGE("DownImage", "(0x0EH) FAILED TO TRANSFER PACKET");
    }
  }

  if (u8_instruction_code == 0x02)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("Img2Tz", "(0x00H) GENERATE CHARACTER FILE COMPLETE");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("Img2Tz", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x06)
    {
      ESP_LOGE("Img2Tz", "(0x06H) FAILED TO GENERATE CHARACTER FILE");
    }
    else if (u8_confirmation_code == 0x07)
    {
      ESP_LOGE("Img2Tz", "(0x07H) FAILED TO GENERATE CHARACTER FILE");
    }
    else if (u8_confirmation_code == 0x15)
    {
      ESP_LOGE("Img2Tz", "(0x15H) FAILED TO GENERATE IMAGE");
    }
  }

  if (u8_instruction_code == 0x05)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("RegModel", "(0x00H) OPERATION SUCCESS");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("RegModel", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x0A)
    {
      ESP_LOGE("RegModel", "(0x0AH) FAILED TO COMBINE CHARACTER FILES");
    }
  }

  if (u8_instruction_code == 0x08)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("UpChar", "(0x00H) READY TO TRANSFER");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("UpChar", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x0D)
    {
      ESP_LOGE("UpChar", "(0x0DH) ERROR UPLOADING TEMPLATE");
    }
  }

  if (u8_instruction_code == 0x09)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("DownChar", "(0x00H) READY TO TRANSFER");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("DownChar", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x0E)
    {
      ESP_LOGE("DownChar", "(0x0EH) FAILED TO RECEIVE PACKAGES");
    }
  }

  if (u8_instruction_code == 0x06)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("Store", "(0x00H) STORAGE SUCCESS");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("Store", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x0B)
    {
      ESP_LOGE("Store", "(0x0BH) ADDRESSED PAGE ID IS BEYOND LIMIT");
    }
    else if (u8_confirmation_code == 0x18)
    {
      ESP_LOGE("Store", "(0x18H) ERROR WRITING FLASH");
    }
  }

  if (u8_instruction_code == 0x07)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("LoadChar", "(0x00H) LOAD SUCCESS");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("LoadChar", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x0C)
    {
      ESP_LOGE("LoadChar", "(0x0CH) ERROR READING TEMPLATE FROM LIBRARY");
    }
    else if (u8_confirmation_code == 0x0B)
    {
      ESP_LOGE("LoadChar", "(0x0BH) ADDRESSED PAGE ID IS BEYOND LIMIT");
    }
  }

  if (u8_instruction_code == 0x0C)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("DeletChar", "(0x00H) DELETE SUCCESS");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("DeletChar", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x10)
    {
      ESP_LOGE("DeletChar", "(0x10H) FAIL TO DELETE TEMPLATE");
    }
  }

  if (u8_instruction_code == 0x0D)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("Empty", "(0x00H) EMPTY SUCCESS");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("Empty", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x11)
    {
      ESP_LOGE("Empty", "(0x11H) FAIL TO CLEAR LIBRARY");
    }
  }

  if (u8_instruction_code == 0x03)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("Match", "(0x00H) TWO TEMPLATE BUFFERS MATCH");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("Match", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x08)
    {
      ESP_LOGE("Match", "(0x08H) TWO TEMPLATES BUFFER UNMATHED");
    }
  }

  if (u8_instruction_code == 0x04)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("Search", "(0x00H) FOUND MATCHING FINGER");
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("Search", "(0x01H) ERROR RECEIVING PACKAGE");
    }
    else if (u8_confirmation_code == 0x09)
    {
      ESP_LOGE("Search", "(0x09H) NO MATCHING FINGER IN LIBRARY");
    }
  }

  if (u8_instruction_code == 0x14)
  {
    if (u8_confirmation_code == 0x00)
    {
      ESP_LOGI("GetRandomCode", "(0x00H) GENERATION SUCCESSFUL");

      ESP_LOGW("GetRandomCode",
               "RANDOMLY GENERATED NUMBER - %x:%x:%x:%x",
               p_received_package[10],
               p_received_package[11],
               p_received_package[12],
               p_received_package[13]);
    }
    else if (u8_confirmation_code == 0x01)
    {
      ESP_LOGE("GetRandomCode", "(0x01H) ERROR RECEIVING PACKAGE");
    }
  }
}