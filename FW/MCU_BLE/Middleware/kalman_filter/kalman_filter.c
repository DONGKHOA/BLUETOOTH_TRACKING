/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "kalman_filter.h"
#include <math.h>

/******************************************************************************
 *   GLOBAL FUNCTIONS
 *****************************************************************************/

void
KALMAN_FILTER_Init (Kalman_Filter_t *p_Kalman,
                    double           d_err_measure,
                    double           d_err_estimate,
                    double           d_q,
                    double           d_start_value)
{
  p_Kalman->d_err_measure      = d_err_measure;
  p_Kalman->d_err_estimate     = d_err_estimate;
  p_Kalman->d_q                = d_q;
  p_Kalman->d_current_estimate = d_start_value;
  p_Kalman->d_last_estimate    = d_start_value;
}

double
KALMAN_FILTER_GetRSSI (Kalman_Filter_t *p_Kalman, double d_mea)
{
  p_Kalman->d_kalman_gain
      = p_Kalman->d_err_estimate
        / (p_Kalman->d_err_estimate + p_Kalman->d_err_measure);

  p_Kalman->d_current_estimate
      = p_Kalman->d_last_estimate
        + p_Kalman->d_kalman_gain * (d_mea - p_Kalman->d_last_estimate);

  p_Kalman->d_err_estimate
      = (1.0f - p_Kalman->d_kalman_gain) * p_Kalman->d_err_estimate
        + fabs(p_Kalman->d_last_estimate - p_Kalman->d_current_estimate)
              * p_Kalman->d_q;

  p_Kalman->d_last_estimate = p_Kalman->d_current_estimate;

  return p_Kalman->d_current_estimate;
}