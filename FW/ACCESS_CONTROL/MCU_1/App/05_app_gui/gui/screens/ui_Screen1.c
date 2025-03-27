

#include "../ui.h"
#include "esp_camera.h"
#include "app_data.h"

static lv_obj_t          *camera_canvas = NULL;
static lv_draw_rect_dsc_t rect_dsc;

static QueueHandle_t *p_camera_capture_queue;
static QueueHandle_t *p_camera_recognition_queue;
static QueueHandle_t *p_result_recognition_queue;

static camera_fb_t       *fb = NULL;
static coord_data_recognition_t s_coord_data_recognition;

static void camera_timer_cb(lv_timer_t *timer);

void
ui_Screen1_screen_init (void)
{
  p_camera_capture_queue     = &s_data_system.s_camera_capture_queue;
  p_camera_recognition_queue = &s_data_system.s_camera_recognition_queue;
  p_result_recognition_queue = &s_data_system.s_result_recognition_queue;

  ui_Screen1 = lv_obj_create(NULL);

  lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);

  camera_canvas = lv_canvas_create(ui_Screen1);

  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_opa       = LV_OPA_TRANSP; // transparent fill
  rect_dsc.border_width = 2;
  rect_dsc.border_color = lv_color_make(0, 255, 0);

  lv_timer_create(camera_timer_cb, 20, NULL);
}

static void
camera_timer_cb (lv_timer_t *timer)
{
  if (xQueueReceive(*p_result_recognition_queue, &s_coord_data_recognition, 1)
      == pdPASS)
  {
  }

  if (xQueueReceive(*p_camera_capture_queue, &fb, 1) == pdPASS)
  {
    if (!fb)
    {
      return;
    }

    lv_canvas_set_buffer(
        camera_canvas, fb->buf, fb->width, fb->height, LV_IMG_CF_TRUE_COLOR);

    int x = 50, y = 30, w = 100, h = 60;

    lv_area_t rect_area
        = { .x1 = x, .y1 = y, .x2 = x + w - 1, .y2 = y + h - 1 };

    lv_canvas_draw_rect(
        camera_canvas, rect_area.x1, rect_area.y1, w, h, &rect_dsc);

    lv_obj_invalidate(camera_canvas);
    esp_camera_fb_return(fb);
  }
}