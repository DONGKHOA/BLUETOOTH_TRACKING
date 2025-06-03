/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "app_display.h"
#include "app_data.h"

#include "i2c.h"
#include "driver/gpio.h"

#include "lcd1602.h"

/******************************************************************************
 *    PUBLIC DATA
 *****************************************************************************/

extern uint32_t u32_number_tag;

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_DISPLAY"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/
typedef struct ble_ibeacon_data
{
  char               c_content[32];
  SemaphoreHandle_t *p_mutex_num_tag;
} display_data_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_DISPLAY_Task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static display_data_t s_display_data;

/******************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
APP_DISPLAY_CreateTask (void)
{
  xTaskCreate(APP_DISPLAY_Task, "display task", 1024 * 4, NULL, 15, NULL);
}

void
APP_DISPLAY_Init (void)
{
  s_display_data.p_mutex_num_tag = &s_data_system.s_mutex_num_tag;

  LCD1602_Init(I2C_NUM);

  LCD1602_SetCursor(I2C_NUM, 0, 0);
  LCD1602_PrintString(I2C_NUM, "Starting...");
  vTaskDelay(2000 / portTICK_PERIOD_MS);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_DISPLAY_Task (void *arg)
{
  uint32_t u32_number_tag_temp = 0;

  while (1)
  {
    xSemaphoreTake(*s_display_data.p_mutex_num_tag, portMAX_DELAY);
    u32_number_tag_temp = u32_number_tag;
    xSemaphoreGive(*s_display_data.p_mutex_num_tag);

    sprintf(
        s_display_data.c_content, "Number of tag: %ld", u32_number_tag_temp);

    LCD1602_SetCursor(I2C_NUM, 0, 0);
    LCD1602_PrintString(I2C_NUM, s_display_data.c_content);

    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}