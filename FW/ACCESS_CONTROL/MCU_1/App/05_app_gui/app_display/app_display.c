/******************************************************************************
 *      INCLUDES
 ******************************************************************************/

 #include "freertos/FreeRTOS.h"
 #include "freertos/queue.h"
 #include "freertos/event_groups.h"

#include "ili9341.h"
#include "xpt2046.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "lvgl.h"
#include "ui.h"
#include "app_display.h"
#include "app_data.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "app_display"

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void lvgl_driver_init(void);
static void lv_tick_task(void *arg);
static void APP_DISPLAY_Task(void *arg);

/******************************************************************************
 *   PUBLIC DATA
 *****************************************************************************/

extern ili9341_handle_t s_ili9341_0;
extern xpt2046_handle_t s_xpt2046_0;
extern lv_obj_t        *ui_Time;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_DISPLAY_CreateTask (void)
{
  // Create task
  xTaskCreate(APP_DISPLAY_Task, "APP_DISPLAY_Task", 1024 * 40, NULL, 5, NULL);
}

void
APP_DISPLAY_Init (void)
{
  // Initialize LVGL
  lv_init();

  // Initialize display driver
  lvgl_driver_init();

  static lv_color_t         buf1[DISP_BUF_SIZE];
  static lv_disp_draw_buf_t draw_buf;
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, DISP_BUF_SIZE);

  // Configure display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = 320;               // Horizontal resolution
  disp_drv.ver_res  = 240;               // Vertical resolution
  disp_drv.flush_cb = DEV_ILI9341_Flush; // Callback to send data to the screen
  disp_drv.draw_buf = &draw_buf;         // Attach drawing buffer
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type    = LV_INDEV_TYPE_POINTER; // Touch device type: pointer
  indev_drv.read_cb = DEV_XPT2046_Read;      // Callback to read touch data
  lv_indev_drv_register(&indev_drv);

  // Create a timer to increase LVGL tick
  const esp_timer_create_args_t lv_tick_timer_args
      = { .callback = &lv_tick_task,
          .arg      = (void *)portTICK_PERIOD_MS,
          .name     = "lv_tick_timer" };

  esp_timer_handle_t lv_tick_timer;
  esp_timer_create(&lv_tick_timer_args, &lv_tick_timer);
  esp_timer_start_periodic(lv_tick_timer, 5 * 1000); // Increase tick every 5ms

  ui_init();
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
lvgl_driver_init (void)
{
  ESP_LOGI(TAG, "Display hor size: %d, ver size: %d", LV_HOR_RES, LV_VER_RES);
  ESP_LOGI(TAG, "Display buffer size: %d", DISP_BUF_SIZE);

  DEV_ILI9341_Init(&s_ili9341_0);

  DEV_XPT2046_Init(&s_xpt2046_0);
}

static void
lv_tick_task (void *arg)
{
  lv_tick_inc(portTICK_PERIOD_MS); // Increase tick every 5ms
}

static void
APP_DISPLAY_Task (void *arg)
{
  while (1)
  {
    lv_timer_handler();            // Handle events and draw GUI
    vTaskDelay(pdMS_TO_TICKS(10)); // Yield CPU
  }
}
