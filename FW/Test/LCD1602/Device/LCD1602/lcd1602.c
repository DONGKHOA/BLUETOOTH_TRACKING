/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include "i2c.h"
#include "driver/gpio.h"

#include "lcd1602.h"

#include "freertos/FreeRTOS.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

// commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_DISPLAYCONTROL 0x08
#define LCD_ENTRYMODESET   0x06
#define LCD_FUNCTIONSET    0x28

// display
#define LCD_CURSOR_LINE 0x80

// flags for display on/off control
#define LCD_DISPLAY_ON  0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON   0x02
#define LCD_CURSOR_OFF  0x00
#define LCD_BLINK_ON    0x01
#define LCD_BLINK_OFF   0x00

// flags for backlight control
#define LCD_BACKLIGHT   0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100 // Enable bit
#define Rw 0b00000010 // Read/Write bit
#define Rs 0b00000001 // Register select bit

#define LCD1602_I2C_TIMEOUT 1000 / portTICK_PERIOD_MS
#define LCD1602_I2C_ADDR    0x27

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void LCD1602_SendCommand(i2c_port_t e_i2c_port, uint8_t command);
static void LCD1602_SendData(i2c_port_t e_i2c_port, uint8_t data);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
LCD1602_Init (i2c_port_t e_i2c_port)
{
  vTaskDelay(50 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port, 0x30);
  vTaskDelay(5 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port, 0x30);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port, 0x30);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port, 0x20);
  vTaskDelay(10 / portTICK_PERIOD_MS);

  LCD1602_SendCommand(e_i2c_port, 0x28);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port, 0x08);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port, 0x01);
  vTaskDelay(2 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port, 0x06);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  LCD1602_SendCommand(e_i2c_port,
                      LCD_DISPLAYCONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF
                          | LCD_BLINK_OFF);
}

void
LCD1602_PrintString (i2c_port_t e_i2c_port, const char *str)
{
  while (*str)
  {
    LCD1602_SendData(e_i2c_port, *str++);
  }
}

void
LCD1602_SetCursor (i2c_port_t e_i2c_port, uint8_t row, uint8_t col)
{
  uint8_t rowOffset[] = { 0x00, 0x40, 0x14, 0x54 };
  LCD1602_SendCommand(e_i2c_port, LCD_CURSOR_LINE | (col + rowOffset[row]));
}

void
LCD1602_Set_Clear (i2c_port_t e_i2c_port)
{
  LCD1602_SendCommand(e_i2c_port, LCD_CLEARDISPLAY);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  LCD1602_SetCursor(e_i2c_port, 0, 0);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

/**
 * The function `LCD1602_SendCommand` sends a command to an LCD display using
 * I2C communication by packing the command into 4 bytes with control bits and
 * then transmitting them in a single I2C transaction.
 *
 * @param e_i2c_port The `e_i2c_port` parameter is the I2C port that the LCD1602
 * display is connected to. It specifies the specific I2C port that will be used
 * for communication with the LCD1602 display.
 * @param command The `command` parameter in the `LCD1602_SendCommand` function
 * is the command to be sent to the LCD display. It is an 8-bit unsigned integer
 * that represents the specific command to be executed by the LCD controller.
 * This command can include instructions such as setting the cursor position,
 * clearing
 */
static void
LCD1602_SendCommand (i2c_port_t e_i2c_port, uint8_t command)
{
  uint8_t data_buf[4];
  // Pack high nibble with control bits
  data_buf[0] = (command & 0xF0) | 0x0C;
  data_buf[1] = (command & 0xF0) | 0x08;
  // Pack low nibble with control bits
  data_buf[2] = ((command << 4) & 0xF0) | 0x0C;
  data_buf[3] = ((command << 4) & 0xF0) | 0x08;

  // Send all 4 bytes in a single I2C transaction
  i2c_master_write_to_device(e_i2c_port,
                             LCD1602_I2C_ADDR,
                             data_buf,
                             sizeof(data_buf),
                             LCD1602_I2C_TIMEOUT);
}

/**
 * The function `LCD1602_SendData` sends data to an LCD display using I2C
 * communication by packing the data into control bits and sending it in a
 * single transaction.
 *
 * @param e_i2c_port The `e_i2c_port` parameter is of type `i2c_port_t` and
 * represents the I2C port that the data will be sent through.
 * @param data The `data` parameter in the `LCD1602_SendData` function
 * represents the information or command that you want to send to an LCD display
 * connected via I2C. In this function, the data is split into two parts - the
 * high nibble and the low nibble - and control bits
 */
static void
LCD1602_SendData (i2c_port_t e_i2c_port, uint8_t data)
{
  uint8_t data_buf[4];
  // Pack high nibble with control bits
  data_buf[0] = (data & 0xF0) | 0x0D;
  data_buf[1] = (data & 0xF0) | 0x09;
  // Pack low nibble with control bits
  data_buf[2] = ((data << 4) & 0xF0) | 0x0D;
  data_buf[3] = ((data << 4) & 0xF0) | 0x09;

  // Send all 4 bytes in a single I2C transaction
  i2c_master_write_to_device(e_i2c_port,
                             LCD1602_I2C_ADDR,
                             data_buf,
                             sizeof(data_buf),
                             LCD1602_I2C_TIMEOUT);
}