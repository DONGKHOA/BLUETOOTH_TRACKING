/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ff.h"

#include "sdspi.h"

/*****************************************************************************
 *   PRIVATE DEFINES
 *****************************************************************************/

#define TRUE  1
#define FALSE 0

/*****************************************************************************
 *   PRIVATE VARIABLES
 *****************************************************************************/

static volatile BSP_SD_SPI_CardInfo cardInfo;
// spi_device_handle_t spi_handle = NULL;
static volatile uint16_t Timer1, Timer2; /* 1ms Timer Counter */
static uint8_t           PowerFlag = 0;  /* Power flag */

/*****************************************************************************
 *   PRIVATE FUNCTION PROTOTYPE
 *****************************************************************************/
static bool BSP_sdSpiIsReady(void);

static void    BSP_sdSpiSelectDevice(gpio_num_t e_gpio_pin);
static void    BSP_sdSpiDeselectDevice(gpio_num_t e_gpio_pin);
static uint8_t BSP_sdSpiWriteByte(uint8_t u8_tx_data, uint32_t u32_timeout);
static void    BSP_sdSpiWriteToDevice(uint8_t *u8_tx_data, size_t i64_length);
static void    BSP_sdSpiReadFromDevice(uint8_t *u8_rx_data, size_t i64_length);
static uint8_t BSP_sdSpiReadByte(uint32_t u32_timeout);
static void    BSP_sdSpiReadBytePtr(uint8_t *p_buffer, uint32_t u32_timeout);

static void    BSP_sdSpiPowerOn(void);
static void    BSP_sdSpiPowerOff(void);
static uint8_t BSP_sdSpiCheckPower(void);
static uint8_t BSP_sdSpiReadyWait(void);

static bool BSP_sdSpiReadDataBlock(uint8_t *p_buffer,
                                   uint32_t u32_len,
                                   uint32_t u32_timeout);
static bool BSP_sdSpiWriteDataBlock(uint8_t *p_buffer,
                                    BYTE     token,
                                    uint32_t u32_timeout);
static BYTE BSP_sdSpiSendCmd(BYTE cmd, uint32_t u32_arg, uint32_t u32_timeout);

static void SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc);

static uint8_t DEV_SDCard_SendCommand(uint8_t  u8_command,
                                      uint32_t u32_arg,
                                      uint8_t  u8_crc);
static uint8_t DEV_SDCard_SendCommand_NEW(uint8_t  cmd,
                                          uint32_t arg,
                                          uint8_t  crc);
/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

uint8_t
BSP_sdSpiInit (void)
{
  uint8_t response;
  uint8_t dummy_clocks = 0xFF;

  uint8_t type = NULL;
  uint8_t n, ocr[4];

  printf("📂 Đang khởi tạo SD Card...\n");

  BSP_spiDriverInit(HSPI_HOST,
                    MISO_PIN,
                    MOSI_PIN,
                    SCLK_PIN,
                    MAX_TRANSFER_SZ,
                    CS_PIN,
                    DMA_CHANNEL,
                    CLOCK_SPEED_HZ,
                    SPI_MODE);

  BSP_spiSelectDevice(CS_PIN);

  printf("📂 Gửi xung clock dummy...\n");
  for (int i = 0; i < 10; i++)
  { // 10 bytes = 80 clock cycles
    BSP_spiWriteByte(dummy_clocks);
  }

  BSP_spiDeselectDevice(CS_PIN);

  PowerFlag = 1;

  printf("📂 Gửi CMD0 (GO_IDLE_STATE)...\n");
  if (DEV_SDCard_SendCommand(CMD0, 0, 0x95) == 0x01)
  {
    /* Set the timer for 1 second */
    Timer1 = 100;

    /* SD Checking Interface Behavior Conditions */
    if (DEV_SDCard_SendCommand(CMD8, 0x000001AA, 0x87) == 0x01)
    {
      /* SDC Ver2+ */
      for (n = 0; n < 4; n++)
      {
        ocr[n] = BSP_spiReadByte();
      }
      if (ocr[2] == 0x01 && ocr[3] == 0xAA)
      {
        /* 2.7-3.6V */
        do
        {
          if (DEV_SDCard_SendCommand(CMD55, 0, 0x00) <= 1
              && DEV_SDCard_SendCommand(CMD41, 1UL << 30, 0x00) == 0)
          {
            break; /* ACMD41 with HCS bit */
          }
        } while (Timer1);

        if (Timer1 && DEV_SDCard_SendCommand(CMD58, 0, 0x00) == 0)
        {
          /* Check CCS bit */
          for (n = 0; n < 4; n++)
          {
            ocr[n] = BSP_spiReadByte();
            printf("ocr[%d] = %d\n", n, ocr[n]);
          }
          type = (ocr[0] & 0x40) ? 6 : 2;
        }
      }
    }
    else
    {
      /* SDC Ver1 or MMC */
      type = (DEV_SDCard_SendCommand(CMD55, 0, 0x00) <= 1
              && DEV_SDCard_SendCommand(CMD41, 0, 0x00) <= 1)
                 ? 2
                 : 1; /* SDC : MMC */
      do
      {
        if (type == 2)
        {
          if (DEV_SDCard_SendCommand(CMD55, 0, 0x00) <= 1
              && DEV_SDCard_SendCommand(CMD41, 0, 0x00) == 0)
          {
            break; /* ACMD41 */
          }
        }
        else
        {
          if (DEV_SDCard_SendCommand(CMD1, 0, 0x00) == 0)
          {
            break; /* CMD1 */
          }
        }
      } while (Timer1);

      if (!Timer1 || DEV_SDCard_SendCommand(CMD16, 512, 0x00) != 0)
      {
        /* Select Block Length */
        type = 0;
      }
    }
  }

  cardInfo.CardType = type;

  BSP_spiDeselectDevice(CS_PIN);

  // BSP_sdSpiReadByte(1000);
  // BSP_spiReadByte();

  printf("Debug: type=%d\n", type);
  if (type)
  {

    // Get information
    uint8_t  n, csd[16];
    uint32_t csize;
    printf("Debug: Vào if(type)\n");
    uint8_t r1     = DEV_SDCard_SendCommand(CMD9, 0, 0x00);
    bool    readOK = BSP_sdSpiReadDataBlock(csd, 16, 1000);
    printf("Debug: CMD9 R1=0x%02X, readOK=%d\n", r1, readOK);

    if ((r1 == 0) && readOK)
    {
      printf("Debug: csd[0]>>6 = %u\n", (csd[0] >> 6));
      if ((csd[0] >> 6) == 1)
      {
        printf("Debug: Vào nhánh SDC V2\n");
        printf("csd[8]=0x%02X, csd[9]=0x%02X\n", csd[8], csd[9]);

        printf("Bắt đầu tính csize...\n");
        csize = csd[9] + ((WORD)csd[8] << 8) + 1;
        printf("csize = %lu\n", csize);

        printf("Gán cardInfo.LogBlockNbr...\n");
        cardInfo.LogBlockNbr = csize << 10;
        printf("Đã gán cardInfo.LogBlockNbr xong\n");

        cardInfo.LogBlockSize = 512;
        printf("Gán cardInfo.LogBlockSize xong\n");

        // Hết nhánh V2
        printf("Ra khỏi nhánh V2\n");
      }
      else
      {
        printf("Debug: Vào nhánh SDC V1/MMC\n");
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + (csd[7] << 2) + ((csd[6] & 3) << 10) + 1;
        cardInfo.LogBlockNbr = csize << (n - 9);
      }
    }
    printf("Initialization Success\n");
    return MSD_OK;
  }
  //   /* Initialization failed */
  PowerFlag = 0;
  return MSD_ERROR;
  // }
}

uint8_t
BSP_sdSpiITConfig (void)
{
  return 0;
}

uint8_t
BSP_sdSpiReadBlocks (uint8_t *p_data,
                     uint32_t u32_readAddr,
                     uint32_t u32_numOfBlocks,
                     uint32_t u32_timeout)
{
  // 1) Kéo CS = 0 (bắt đầu giao dịch)
  BSP_sdSpiSelectDevice(CS_PIN);

  // 2) Nếu chỉ đọc 1 block => CMD17
  if (u32_numOfBlocks == 1)
  {
    // Gửi CMD17(address)
    uint8_t r1
        = DEV_SDCard_SendCommand_NEW(CMD17, u32_readAddr, 0x01 /* hoặc 0x00 */);
    if (r1 == 0) // R1=0 => OK
    {
      // Chờ token 0xFE, đọc 512 byte
      if (BSP_sdSpiReadDataBlock(p_data, 512, u32_timeout))
      {
        u32_numOfBlocks = 0; // Đánh dấu đã đọc thành công
      }
    }
  }
  else
  {
    // 3) Nhiều block => CMD18
    uint8_t r1 = DEV_SDCard_SendCommand_NEW(CMD18, u32_readAddr, 0x01);
    if (r1 == 0)
    {
      // Liên tục đọc mỗi block 512 byte
      do
      {
        if (!BSP_sdSpiReadDataBlock(p_data, 512, u32_timeout))
        {
          // Nếu đọc block này lỗi => thoát
          break;
        }
        // Dịch con trỏ sang block kế tiếp
        p_data += 512;
      } while (--u32_numOfBlocks);

      // Gửi CMD12 để dừng
      DEV_SDCard_SendCommand_NEW(CMD12, 0, 0x01);
    }
  }

  // 4) Thả CS
  BSP_sdSpiDeselectDevice(CS_PIN);

  // 5) Đọc 1 byte 0xFF “nhàn rỗi” (thường có trong spec)
  BSP_sdSpiReadByte(u32_timeout);

  // Trả về kết quả
  return (u32_numOfBlocks == 0) ? MSD_OK : MSD_ERROR;
}

// uint8_t
// BSP_sdSpiReadBlocks (uint8_t *p_data,
//                      uint32_t u32_readAddr,
//                      uint32_t u32_numOfBlocks,
//                      uint32_t u32_timeout)
// {

//   BSP_sdSpiSelectDevice(CS_PIN);

//   if (u32_numOfBlocks == 1)
//   {
//     /* 싱글 블록 읽기 */
//     if ((DEV_SDCard_SendCommand(CMD17, u32_readAddr, 0x00) == 0)
//         && BSP_sdSpiReadDataBlock(p_data, 512, u32_timeout))
//     {
//       u32_numOfBlocks = 0;
//     }
//   }
//   else
//   {
//     /* 다중 블록 읽기 */
//     if (DEV_SDCard_SendCommand(CMD18, u32_readAddr, 0x00) == 0)
//     {
//       do
//       {
//         if (!BSP_sdSpiReadDataBlock(p_data, 512, u32_timeout))
//         {
//           break;
//         }

//         p_data += 512;
//       } while (--u32_numOfBlocks);

//       /* STOP_TRANSMISSION, 모든 블럭을 다 읽은 후, 전송 중지 요청 */
//       DEV_SDCard_SendCommand(CMD12, 0, 0x00);
//     }
//   }

//   BSP_sdSpiDeselectDevice(CS_PIN);
//   BSP_sdSpiReadByte(u32_timeout); /* Idle 상태(Release DO) */

//   return u32_numOfBlocks ? MSD_ERROR : MSD_OK;
// }

uint8_t
BSP_sdSpiWriteBlocks (uint32_t *p_data,
                      uint32_t  u32_writeAddr,
                      uint32_t  u32_numOfBlocks,
                      uint32_t  u32_timeout)
{
  if (!(cardInfo.CardType & CT_SD2))
  {
    u32_writeAddr *= 512;
  }

  BSP_sdSpiSelectDevice(CS_PIN);

  if (u32_numOfBlocks == 1)
  {
    /* 싱글 블록 쓰기 */
    if ((BSP_sdSpiSendCmd(CMD24, u32_writeAddr, u32_timeout) == 0)
        && BSP_sdSpiWriteDataBlock((uint8_t *)p_data, 0xFE, u32_timeout))
    {
      u32_numOfBlocks = 0;
    }
  }
  else
  {
    /* WRITE_MULTIPLE_BLOCK */
    if (cardInfo.CardType & CT_SD1)
    {
      BSP_sdSpiSendCmd(CMD55, 0, u32_timeout);
      BSP_sdSpiSendCmd(CMD23, u32_numOfBlocks, u32_timeout); /* ACMD23 */
    }
    if (BSP_sdSpiSendCmd(CMD25, u32_writeAddr, u32_timeout) == 0)
    {
      do
      {
        if (!BSP_sdSpiWriteDataBlock((uint8_t *)p_data, 0xFC, u32_timeout))
        {
          break;
        }
        p_data += 512;
      } while (--u32_numOfBlocks);

      /* STOP_TRAN token */
      if (!BSP_sdSpiWriteDataBlock(0, 0xFD, u32_timeout))
      {
        u32_numOfBlocks = 1;
      }
    }
  }

  BSP_sdSpiDeselectDevice(CS_PIN);
  BSP_sdSpiReadByte(1000);

  return u32_numOfBlocks ? MSD_ERROR : MSD_OK;
}

/**
 * @brief Retrieves information about the SD card and populates the provided
 *        `BSP_SD_SPI_CardInfo` structure.
 *
 * This function fills the `CardInfo` structure with details about the SD
 * card, including the card type, the number of logical blocks, and the size
 * of each logical block. The information is fetched from a global or internal
 * `cardInfo` object that contains the SD card's current configuration.
 *
 * @param CardInfo Pointer to a `BSP_SD_SPI_CardInfo` structure to be filled
 *                 with the card information.
 */
void
BSP_sdSpiGetCardInfo (BSP_SD_SPI_CardInfo *CardInfo)
{
  CardInfo->CardType     = cardInfo.CardType;
  CardInfo->LogBlockNbr  = cardInfo.LogBlockNbr;
  CardInfo->LogBlockSize = cardInfo.LogBlockSize;
}

/*****************************************************************************
 *   PRIVATE FUNCTIONS
 *****************************************************************************/
static uint8_t
DEV_SDCard_SendCommand_NEW (uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t response;
  uint8_t tx_data[6]
      = { (cmd & 0x3F) | 0x40, (arg >> 24) & 0xFF, (arg >> 16) & 0xFF,
          (arg >> 8) & 0xFF,   (arg) & 0xFF,       crc };

  // Cũ: tự gọi select/deselect
  // BSP_spiSelectDevice(CS_PIN);
  // ...
  // BSP_spiDeselectDevice(CS_PIN);

  // Thay vào đó, bạn bỏ hẳn 2 dòng select/deselect ở đây
  // => "DEV_SDCard_SendCommand" sẽ chỉ gửi 6 byte lệnh + đọc R1

  // Gửi 6 byte lệnh
  BSP_sdSpiWriteToDevice(tx_data, 6);

  // Đọc R1 tối đa 8 byte
  uint8_t resp = 0xFF;
  for (int i = 0; i < 8; i++)
  {
    resp = BSP_sdSpiReadByte(1000);
    if (resp != 0xFF)
    {
      break;
    }
  }

  return resp;
}

static uint8_t
DEV_SDCard_SendCommand (uint8_t u8_command, uint32_t u32_arg, uint8_t u8_crc)
{
  uint8_t response;
  uint8_t tx_data[6]
      = { (u8_command & 0x3F) | 0x40, // 🔹 Đảm bảo CMD đúng format (0x40 | CMD)
          (u32_arg >> 24) & 0xFF,
          (u32_arg >> 16) & 0xFF,
          (u32_arg >> 8) & 0xFF,
          u32_arg & 0xFF,
          u8_crc };

  BSP_spiSelectDevice(CS_PIN);
  vTaskDelay(pdMS_TO_TICKS(1)); // 🔹 Delay để ổn định CS

  BSP_sdSpiWriteToDevice(tx_data, 6);

  // 4️⃣ Đọc phản hồi từ SD (tối đa 8 byte chờ response)
  for (uint16_t i = 0; i < 8; i++)
  {
    response = BSP_sdSpiReadByte(1000);
    if (response != 0xFF)
    {
      break;
    }
  }

  BSP_spiDeselectDevice(CS_PIN);
  vTaskDelay(pdMS_TO_TICKS(1)); // 🔹 Delay để ổn định CS

  return response;
}

static bool
BSP_sdSpiIsReady (void)
{
  spi_transaction_t trans;
  memset(&trans, 0, sizeof(trans));

  // Kiểm tra xem hàng đợi SPI có chỗ trống không
  esp_err_t ret = spi_device_polling_transmit(spi_handle, &trans);

  return (ret == ESP_OK); // Trả về true nếu SPI sẵn sàng
}

static void
BSP_sdSpiSelectDevice (gpio_num_t e_gpio_pin)
{
  gpio_set_level(e_gpio_pin, 0);
}

static void
BSP_sdSpiDeselectDevice (gpio_num_t e_gpio_pin)
{
  gpio_set_level(e_gpio_pin, 1);
}

static void
BSP_sdSpiWriteToDevice (uint8_t *u8_tx_data, size_t i64_length)
{
  spi_transaction_t master_trans
      = { .flags = 0, .tx_buffer = u8_tx_data, .length = i64_length * 8 };
  spi_device_transmit(spi_handle, &master_trans);
}

static uint8_t
BSP_sdSpiWriteByte (uint8_t u8_tx_data, uint32_t u32_timeout)
{
  printf("Attempting to send byte: 0x%02X", u8_tx_data);

  uint32_t elapsed_time = 0;
  int64_t  start_time   = esp_timer_get_time();

  while (elapsed_time < u32_timeout)
  {
    if (BSP_sdSpiIsReady())
    {
      printf("SPI is ready, writing data...");
      BSP_sdSpiWriteToDevice(&u8_tx_data, 1);
      return 0; // Trả về kết quả thực tế của `BSP_sdSpiWriteToDevice()`
    }

    elapsed_time = (esp_timer_get_time() - start_time) / 1000;
  }

  printf("SPI Write Timeout!");
  return 1; // Timeout, send failed
}

static void
BSP_sdSpiReadFromDevice (uint8_t *u8_rx_data, size_t i64_length)
{
  spi_transaction_t master_rec = {
    .tx_buffer = NULL,
    .rx_buffer = u8_rx_data,
    .length    = i64_length * 8,
  };
  spi_device_transmit(spi_handle, &master_rec);
}

static uint8_t
BSP_sdSpiReadByte (uint32_t u32_timeout)
{
  uint8_t u8_rx_data = 0xFF; // Default value
  printf("Attempting to read SPI byte...\r\n");

  uint32_t elapsed_time = 0;
  int64_t  start_time   = esp_timer_get_time();

  while (elapsed_time < u32_timeout)
  {
    BSP_sdSpiReadFromDevice(&u8_rx_data, 1);

    printf("Received: 0x%02X\r\n", u8_rx_data);

    if (u8_rx_data != 0xFF) // Kiểm tra xem có dữ liệu hợp lệ không
    {
      printf("Valid SPI data received: 0x%02X\r\n", u8_rx_data);
      return u8_rx_data;
    }

    elapsed_time = (esp_timer_get_time() - start_time) / 1000;
  }

  printf("SPI Read Timeout!\r\n");
  return u8_rx_data;
}

static void
BSP_sdSpiReadBytePtr (uint8_t *p_buffer, uint32_t u32_timeout)
{
  *p_buffer = BSP_sdSpiReadByte(u32_timeout);
}

/* power on */
static void
BSP_sdSpiPowerOn (void)
{
  uint8_t response;
  uint8_t dummy_clock = 0xFF;
  for (int i = 0; i < 10; i++)
  {
    BSP_spiWriteToDevice(&dummy_clock, 1);
  }
}

/* power off */
static void
BSP_sdSpiPowerOff (void)
{
  PowerFlag = 0;
}

/* check power flag */
static uint8_t
BSP_sdSpiCheckPower (void)
{
  return PowerFlag;
}

/* SD Ready */
/* wait SD ready */
static uint8_t
BSP_sdSpiReadyWait (void)
{
  uint8_t res;

  /* timeout 500ms */
  Timer2 = 500;

  /* if SD goes ready, receives 0xFF */
  do
  {
    res = BSP_sdSpiReadByte(Timer2);
  } while ((res != 0xFF) && Timer2);

  return res;
}

/* receive data block */
static bool
BSP_sdSpiReadDataBlock (uint8_t *p_buffer,
                        uint32_t u32_len,
                        uint32_t u32_timeout)
{
  // 1) Chờ token 0xFE
  // uint8_t token;
  // Timer1 = 200;
  // do
  // {
  //   token = BSP_sdSpiReadByte(u32_timeout);
  // } while ((token == 0xFF) && Timer1);
  // if (token != 0xFE)
  // {
  //   return false;
  // }

  // // 2) Đọc đúng `len` byte (16 byte CSD)
  // for (size_t i = 0; i < u32_len; i++)
  // {
  //   p_buffer[i] = BSP_sdSpiReadByte(u32_timeout);
  //   // printf("p_buffer[%d] = 0x%02X\r\n", i, p_buffer[i]);
  // }

  // // 3) Bỏ qua 2 byte CRC
  // BSP_sdSpiReadByte(u32_timeout);
  // BSP_sdSpiReadByte(u32_timeout);

  // return true;

  uint8_t token;

  // Chờ token 0xFE
  Timer1 = 200;
  do
  {
    token = BSP_sdSpiReadByte(u32_timeout);
  } while ((token == 0xFF) && Timer1);

  if (token != 0xFE)
  {
    return false;
  }

  // Đọc "len" byte dữ liệu
  for (size_t i = 0; i < u32_len; i++)
  {
    p_buffer[i] = BSP_sdSpiReadByte(u32_timeout);
  }

  // Bỏ qua 2 byte CRC
  BSP_sdSpiReadByte(u32_timeout);
  BSP_sdSpiReadByte(u32_timeout);

  return true;
}

/* transmit data block */
static bool
BSP_sdSpiWriteDataBlock (uint8_t *p_buffer, BYTE token, uint32_t u32_timeout)
{
  uint8_t resp = 0xFF;
  uint8_t i    = 0;

  /* wait SD ready */
  if (BSP_sdSpiReadyWait() != 0xFF)
  {
    return FALSE;
  }

  /* transmit token */
  BSP_sdSpiWriteByte(token, u32_timeout);

  /* if it's not STOP token, transmit data */
  if (token != 0xFD)
  {
    BSP_sdSpiWriteToDevice((uint8_t *)p_buffer, 512);

    /* discard CRC */
    BSP_sdSpiReadByte(u32_timeout);
    BSP_sdSpiReadByte(u32_timeout);

    /* receive response */
    while (i <= 64)
    {
      resp = BSP_sdSpiReadByte(u32_timeout);

      /* transmit 0x05 accepted */
      if ((resp & 0x1F) == 0x05)
      {
        break;
      }
      i++;
    }

    /* recv buffer clear */
    while (BSP_sdSpiReadByte(u32_timeout) == 0)
      ;
  }

  /* transmit 0x05 accepted */
  if ((resp & 0x1F) == 0x05)
  {
    return TRUE;
  }

  return FALSE;
}

/* transmit command */
static BYTE
BSP_sdSpiSendCmd (BYTE cmd, uint32_t u32_arg, uint32_t u32_timeout)
{
  uint8_t crc, res;

  /* wait SD ready */
  if (BSP_sdSpiReadyWait() != 0xFF)
  {
    return 0xFF;
  }

  /* transmit command */
  BSP_sdSpiWriteByte(cmd, u32_timeout); /* Command */
  BSP_sdSpiWriteByte((uint8_t)(u32_arg >> 24),
                     u32_timeout); /* Argument[31..24] */
  BSP_sdSpiWriteByte((uint8_t)(u32_arg >> 16),
                     u32_timeout); /* Argument[23..16] */
  BSP_sdSpiWriteByte((uint8_t)(u32_arg >> 8),
                     u32_timeout);                   /* Argument[15..8] */
  BSP_sdSpiWriteByte((uint8_t)u32_arg, u32_timeout); /* Argument[7..0] */

  /* prepare CRC */
  if (cmd == CMD0)
  {
    crc = 0x95; /* CRC for CMD0(0) */
  }
  else if (cmd == CMD8)
  {
    crc = 0x87; /* CRC for CMD8(0x1AA) */
  }
  else
  {
    crc = 1;
  }

  /* transmit CRC */
  BSP_sdSpiWriteByte(crc, u32_timeout);

  /* Skip a stuff byte when STOP_TRANSMISSION */
  if (cmd == CMD12)
  {
    BSP_sdSpiReadByte(u32_timeout);
  }

  /* receive response */
  uint8_t n = 10;
  do
  {
    res = BSP_sdSpiReadByte(u32_timeout);
  } while ((res & 0x80) && --n);

  return res;
}

// Sửa ngày 4/2/2025

// #define SD_CMD0  0x40  // Reset SD Card
// #define SD_CMD8  0x48  // Check SDHC/SDXC
// #define SD_CMD17 0x51  // Read single block
// #define SD_CMD24 0x58  // Write single block

static void
SD_SendCommand (uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t tx_data[6];
  tx_data[0] = cmd;
  tx_data[1] = (arg >> 24) & 0xFF;
  tx_data[2] = (arg >> 16) & 0xFF;
  tx_data[3] = (arg >> 8) & 0xFF;
  tx_data[4] = arg & 0xFF;
  tx_data[5] = crc;

  BSP_spiSelectDevice(CS_PIN);
  BSP_spiWriteToDevice(tx_data, 6);
  BSP_spiDeselectDevice(CS_PIN);
}
