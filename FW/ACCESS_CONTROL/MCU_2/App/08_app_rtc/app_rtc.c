/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_rtc.h"
#include "app_data.h"
#include "ds3231.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

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

static void APP_RTC_task(void *arg);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_RTC_CreateTask (void)
{
  xTaskCreate(APP_RTC_task, "RTC task", 1024 * 4, NULL, 7, NULL);
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
APP_RTC_task (void *arg)
{
  static bool synced = false;
  while (1)
  {
    EventBits_t bits = xEventGroupGetBits(*s_handle_rtc.p_flag_time_event);
    if ((bits & TIME_SOURCE_RTC_READY) != 0)
    {
      synced = false; // Reset SNTP sync flag

      // SNTP not ready yet → read and send RTC time
      DEV_DS3231_Register_Read(&s_handle_rtc.s_ds3231_data, I2C_NUM_0);

      // Send data to app_transmit
    }
    else
    {
      if (!synced)
      {
        time_t now;
        time(&now); // SNTP time already set to system time
        DEV_DS3231_Register_Write(now, I2C_NUM_0); // Sync RTC
        ESP_LOGI(TAG, "[RTC] Synced with SNTP");
        synced = true;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}