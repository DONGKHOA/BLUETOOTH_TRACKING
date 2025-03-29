/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "esp_camera.h"
#include "esp_log.h"

#include "app_handle_camera.h"
#include "app_data.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_HANDLE_CAMERA"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t      *p_camera_capture_queue;
  EventGroupHandle_t *p_display_event;
} app_handle_camera_t;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_HANDLE_CAMERA_task(void *arg);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static app_handle_camera_t s_app_handle_camera;
uint8_t                    *psram_array;

static camera_config_t camera_config = {
  .pin_pwdn     = CAM_PIN_PWDN,
  .pin_reset    = CAM_PIN_RESET,
  .pin_xclk     = CAM_PIN_XCLK,
  .pin_sscb_sda = CAM_PIN_SIOD,
  .pin_sscb_scl = CAM_PIN_SIOC,

  .pin_d7    = CAM_PIN_D7,
  .pin_d6    = CAM_PIN_D6,
  .pin_d5    = CAM_PIN_D5,
  .pin_d4    = CAM_PIN_D4,
  .pin_d3    = CAM_PIN_D3,
  .pin_d2    = CAM_PIN_D2,
  .pin_d1    = CAM_PIN_D1,
  .pin_d0    = CAM_PIN_D0,
  .pin_vsync = CAM_PIN_VSYNC,
  .pin_href  = CAM_PIN_HREF,
  .pin_pclk  = CAM_PIN_PCLK,

  // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
  .xclk_freq_hz = 16000000,
  .ledc_timer   = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,

  .pixel_format = PIXFORMAT_RGB565, // YUV422,GRAYSCALE,RGB565,JPEG
  .frame_size
  = FRAMESIZE_QVGA, // QQVGA-UXGA Do not use sizes above QVGA when not JPEG

  .jpeg_quality = 10, // 0-63 lower number means higher quality
  .fb_count
  = 2, // if more than one, i2s runs in continuous mode. Use only with JPEG

  .fb_location = CAMERA_FB_IN_PSRAM,
  .grab_mode   = CAMERA_GRAB_WHEN_EMPTY,
};

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_HANDLE_CAMERA_CreateTask (void)
{
  xTaskCreate(
      APP_HANDLE_CAMERA_task, "handle camera task", 1024 * 4, NULL, 6, NULL);
}
void
APP_HANDLE_CAMERA_Init (void)
{
  s_app_handle_camera.p_camera_capture_queue
      = &s_data_system.s_camera_capture_queue;

  s_app_handle_camera.p_display_event = &s_data_system.s_display_event;

  // initialize the camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Camera Init Failed");
  }
  psram_array = (uint8_t *)heap_caps_malloc(153600 * sizeof(uint8_t),
                                            MALLOC_CAP_SPIRAM);

  if (psram_array == NULL)
  {
    printf("Failed to allocate memory in PSRAM\r\n");
    return;
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_HANDLE_CAMERA_task (void *arg)
{
  EventBits_t  uxBits;
  camera_fb_t *fb = NULL;

  while (1)
  {
    uxBits = xEventGroupWaitBits(*s_app_handle_camera.p_display_event,
                                 ATTENDANCE_BIT | ENROLL_FACE_ID_BIT,
                                 pdFALSE,
                                 pdFALSE,
                                 portMAX_DELAY);

    if (uxBits == 0)
    {
      continue;
    }

    fb = esp_camera_fb_get();

    if (!fb)
    {
      ESP_LOGE(TAG, "Camera Capture Failed");
      continue;
    }

    xQueueSend(*s_app_handle_camera.p_camera_capture_queue, &fb, 0);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}