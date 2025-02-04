// /******************************************************************************
//  *      INCLUDES
//  *****************************************************************************/

// #include <stdio.h>
// #include <string.h>

// #include "sdkconfig.h"

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "sdcard.h"
// #include "esp_vfs_fat.h"
// #include "sdmmc_cmd.h"

// /*****************************************************************************
//  *   PRIVATE DEFINES
//  *****************************************************************************/

// #define HSPI_HOST       SPI2_HOST
// #define MISO_PIN        GPIO_NUM_19
// #define MOSI_PIN        GPIO_NUM_23
// #define SCLK_PIN        GPIO_NUM_18
// #define CS_PIN          GPIO_NUM_5
// #define CLOCK_SPEED_HZ  400000
// #define MAX_TRANSFER_SZ 4096
// #define DMA_CHANNEL     1
// #define SPI_MODE        0

// #define SPI_CLOCK_FAST 20000000

// /*****************************************************************************
//  *   PRIVATE PROTOTYPE FUNCTIONS
//  *****************************************************************************/

// #define MOUNT_POINT "/sdcard"

// /*****************************************************************************
//  *   PUBLIC FUNCTIONS
//  *****************************************************************************/
// uint8_t
// DEV_SDCard_Init (void)
// {
//   esp_err_t ret;

//   // Initialize SPI with low speed at startup
//   BSP_sdSpiDriverInit(HSPI_HOST,
//                       MISO_PIN,
//                       MOSI_PIN,
//                       SCLK_PIN,
//                       MAX_TRANSFER_SZ,
//                       CS_PIN,
//                       DMA_CHANNEL,
//                       CLOCK_SPEED_HZ,
//                       SPI_MODE);

//   // Mount SD Card using FATFS
//   ret = BSP_sdFatFsMount(HSPI_HOST, "/sdcard", CS_PIN, false, 5, 16 * 1024);

//   if (ret != ESP_OK)
//   {
//     return 0x01; // Mount failed
//   }

//   // Increase SPI speed to 20MHz
//   BSP_spiSChangeClock(HSPI_HOST, SPI_CLOCK_FAST);

//   return 0x00; // Success
// }

// // Const char* được sử dụng khi ký tự cần ghi là string
// // Const uint8_t* được sử dụng khi ký tự là nhị phân/hex
// void
// DEV_SDCard_Write_File (const char *p_filename, const char *p_data)
// {
//   char full_path[128];

//   snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, p_filename);

//   FILE *f = fopen(full_path, "w");
//   if (f == NULL)
//   {
//     printf("Không thể mở file để ghi\n");
//     return;
//   }
//   fprintf(f, "%s\n", p_data);
//   fclose(f);

//   printf("Ghi file thành công: %s\n", p_filename);
// }

// void
// DEV_SDCard_Read_File (const char *p_path)
// {
//   FILE *f = fopen(p_path, "r");
//   if (f == NULL)
//   {
//     printf("Failed to open file for reading\n");
//     return;
//   }
//   char line[128];
//   fgets(line, sizeof(line), f);
//   fclose(f);
//   printf("Read from file: %s\n", line);
// }

// void
// DEV_SDCard_Delete_File (const char *p_path)
// {
//   if (f_unlink(p_path) == 0)
//   {
//     printf("File deleted successfully\n");
//   }
//   else
//   {
//     printf("Failed to delete file\n");
//   }
// }

// /*****************************************************************************
//  *   PRIVATE FUNCTIONS
//  *****************************************************************************/