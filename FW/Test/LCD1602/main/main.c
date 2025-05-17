/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "i2c.h"
#include "lcd1602.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_27
#define I2C_SCL        GPIO_NUM_14
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static inline void APP_MAIN_InitI2C(void);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  APP_MAIN_InitI2C();

  LCD1602_Init(I2C_NUM);

  LCD1602_SetCursor(I2C_NUM, 0, 0);
  LCD1602_PrintString(I2C_NUM, "Hello, World!");
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static inline void
APP_MAIN_InitI2C (void)
{
  BSP_i2cConfigMode(I2C_NUM, I2C_MODE);
  BSP_i2cConfigSDA(I2C_NUM, I2C_SDA, I2C_SDA_PULLUP);
  BSP_i2cConfigSCL(I2C_NUM, I2C_SCL, I2C_SCL_PULLUP);
  BSP_i2cConfigClockSpeed(I2C_NUM, I2C_CLK_SPEED);
  BSP_i2cDriverInit(I2C_NUM);
}
