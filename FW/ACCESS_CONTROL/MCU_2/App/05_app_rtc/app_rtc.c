/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_rtc.h"
#include "app_data.h"
#include "ds3231.h"

#include "esp_log.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_RTC"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  ds3231_data_t       s_ds3231_data;
  EventGroupHandle_t *p_flag_time_event;
} handle_rtc_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static handle_rtc_t s_handle_rtc;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_RTC_Task(void *arg);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_RTC_CreateTask (void)
{
  xTaskCreate(APP_RTC_Task, "RTC task", 1024 * 4, NULL, 7, NULL);
}

void
APP_RTC_Init (void)
{
  s_handle_rtc.p_flag_time_event = &s_data_system.s_flag_time_event;

  xEventGroupSetBits(*s_handle_rtc.p_flag_time_event, TIME_SOURCE_RTC_READY);

  DEV_DS3231_Init(&s_handle_rtc.s_ds3231_data, I2C_NUM_0);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_RTC_Task (void *arg)
{
  while (1)
  {
    taskENTER_CRITICAL(&i2c_mux);

    xSemaphoreTake(s_data_system.s_i2c_mutex, 1000 / portTICK_PERIOD_MS);

    DEV_DS3231_Register_Read(&s_handle_rtc.s_ds3231_data, I2C_NUM);

    xSemaphoreGive(s_data_system.s_i2c_mutex);

    taskEXIT_CRITICAL(&i2c_mux);
  }
}