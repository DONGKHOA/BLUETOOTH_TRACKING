#ifndef KALMAN_FILTER_H_
#define KALMAN_FILTER_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct
  {
    double d_err_measure;
    double d_err_estimate;
    double d_q;
    double d_current_estimate;
    double d_last_estimate;
    double d_kalman_gain;
  } Kalman_Filter_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void   KALMAN_FILTER_Init(Kalman_Filter_t *p_Kalman,
                            double           d_err_measure,
                            double           d_err_estimate,
                            double           d_q,
                            double           d_start_value);
  double KALMAN_FILTER_GetRSSI(Kalman_Filter_t *p_Kalman, double d_mea);

#ifdef __cplusplus
}
#endif

#endif /* KALMAN_FILTER_H_ */