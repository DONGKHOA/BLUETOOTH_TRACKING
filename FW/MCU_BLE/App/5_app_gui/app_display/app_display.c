/******************************************************************************
 *      INCLUDES
 ******************************************************************************/

#include "app_display.h"
#include "app_data.h"
#include "ui.h"

#include "ili9341.h"
#include "xpt2046.h"

#include "esp_log.h"
#include "esp_timer.h"

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

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_DISPLAY_CreateTask (void)
{
  // Create task
  xTaskCreate(APP_DISPLAY_Task, "APP_DISPLAY_Task", 4096, NULL, 5, NULL);
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

  // Cấu hình driver hiển thị
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = 320;               // Độ phân giải ngang
  disp_drv.ver_res  = 240;               // Độ phân giải dọc
  disp_drv.flush_cb = DEV_ILI9341_Flush; // Callback gửi dữ liệu đến màn hình
  disp_drv.draw_buf = &draw_buf;         // Gắn buffer vẽ
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type    = LV_INDEV_TYPE_POINTER; // Thiết bị cảm ứng dạng pointer
  indev_drv.read_cb = DEV_XPT2046_Read;     // Callback đọc dữ liệu cảm ứng
  lv_indev_t *indev_touch = lv_indev_drv_register(&indev_drv);

  // Tạo timer để tăng tick LVGL
  const esp_timer_create_args_t lv_tick_timer_args
      = { .callback = &lv_tick_task,
          .arg      = (void *)portTICK_PERIOD_MS,
          .name     = "lv_tick_timer" };

  esp_timer_handle_t lv_tick_timer;
  esp_timer_create(&lv_tick_timer_args, &lv_tick_timer);
  esp_timer_start_periodic(lv_tick_timer, 5 * 1000); // Tăng tick mỗi 5ms

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
  lv_tick_inc(portTICK_PERIOD_MS); // Tăng tick mỗi 5ms
}

static void
APP_DISPLAY_Task (void *arg)
{
  while (1)
  {
    lv_timer_handler();            // Xử lý các sự kiện và vẽ GUI
    vTaskDelay(pdMS_TO_TICKS(10)); // Nhường CPU
  }
}