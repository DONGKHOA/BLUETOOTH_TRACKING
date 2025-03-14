#ifndef XPT2046_H_
#define XPT2046_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include "lvgl.h"

#include "spi.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

#define XPT2046_AVG             4
#define XPT2046_X_MIN           200
#define XPT2046_Y_MIN           120
#define XPT2046_X_MAX           1900
#define XPT2046_Y_MAX           1900
#define XPT2046_X_INV           1
#define XPT2046_Y_INV           1
#define XPT2046_XY_SWAP         1
#define XPT2046_TOUCH_THRESHOLD 400 // Threshold for touch detection
#define XPT2046_TOUCH_IRQ       1
#define XPT2046_TOUCH_IRQ_PRESS 0
#define XPT2046_TOUCH_PRESS     0

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct
  {
    gpio_num_t           e_irq_pin;
    spi_device_handle_t *p_spi_Handle;
  } xpt2046_handle_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void DEV_XPT2046_Init(xpt2046_handle_t *p_xpt2046_handle);
  bool DEV_XPT2046_Read(lv_indev_drv_t *drv, lv_indev_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* XPT2046_H_ */