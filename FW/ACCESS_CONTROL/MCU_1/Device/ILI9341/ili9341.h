#ifndef ILI9341_H_
#define ILI9341_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "lvgl.h"

#include "spi.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct
  {
    gpio_num_t e_dc_pin;
    gpio_num_t e_rst_pin;
    spi_device_handle_t *p_spi_Handle;
  } ili9341_handle_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void DEV_ILI9341_Init(ili9341_handle_t *p_ili9341_handle);
  void DEV_ILI9341_Flush(lv_disp_drv_t   *drv,
                         const lv_area_t *area,
                         lv_color_t      *color_map);
  void DEV_ILI9341_EnableBacklight(bool backlight);
  void DEV_ILI9341_SleepIn(void);
  void DEV_ILI9341_SleepOut(void);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_H_ */