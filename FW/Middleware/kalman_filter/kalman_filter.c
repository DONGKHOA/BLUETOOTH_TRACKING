/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "kalman_filter.h"
#include <math.h>

/******************************************************************************
 *   GLOBAL FUNCTIONS
 *****************************************************************************/

/**
 * The function `KALMAN_FILTER_Init` initializes a Kalman filter structure with specified error values.
 *
 * @param p_Kalman `p_Kalman` is a pointer to a structure of type `Kalman_Filter_t`.
 * @param d_err_measure The `d_err_measure` parameter in the `KALMAN_FILTER_Init` function represents
 * the error in the measurement. It is used in the Kalman filter algorithm to model the uncertainty or
 * noise in the sensor measurements.
 * @param d_err_estimate The `d_err_estimate` parameter in the `KALMAN_FILTER_Init` function represents
 * the error in the initial estimate of the system state. It is used in the Kalman filter algorithm to
 * determine how much weight to give to the measurement versus the prediction when updating the state
 * estimate. A smaller value
 * @param d_q The parameter `d_q` in the `KALMAN_FILTER_Init` function represents the process noise
 * covariance. It is used in the Kalman filter algorithm to model the uncertainty in the system
 * dynamics. A higher value of `d_q` indicates higher uncertainty in the process model.
 */
void KALMAN_FILTER_Init(Kalman_Filter_t *p_Kalman, double d_err_measure, double d_err_estimate, double d_q)
{
  p_Kalman->d_err_measure  = d_err_measure;
  p_Kalman->d_err_estimate = d_err_estimate;
  p_Kalman->d_q            = d_q;
}

/**
 * The function implements a Kalman filter algorithm to estimate a value based on measurements and
 * previous estimates.
 *
 * @param p_Kalman The `p_Kalman` parameter is a pointer to a structure of type `Kalman_Filter_t`. This
 * structure likely contains the necessary variables for implementing a Kalman filter, such as
 * `d_kalman_gain`, `d_err_estimate`, `d_err_measure`, `d_current_estimate`,
 * @param d_mea The `d_mea` parameter in the `KALMAN_FILTER_GetRSSI` function represents the measured
 * value of the signal strength (RSSI) that you want to filter using a Kalman filter. This value is
 * used in the Kalman filter algorithm to update the estimate of the true signal
 *
 * @return The function `KALMAN_FILTER_GetRSSI` is returning the current estimate of the RSSI (Received
 * Signal Strength Indication) value calculated using a Kalman filter.
 */
double KALMAN_FILTER_GetRSSI(Kalman_Filter_t *p_Kalman, double d_mea)
{
  p_Kalman->d_kalman_gain =
  p_Kalman->d_err_estimate / (p_Kalman->d_err_estimate + p_Kalman->d_err_measure);

  p_Kalman->d_current_estimate = p_Kalman->d_last_estimate +
  p_Kalman->d_kalman_gain * (d_mea - p_Kalman->d_last_estimate);

  p_Kalman->d_err_estimate = (1.0f - p_Kalman->d_kalman_gain) * p_Kalman->d_err_estimate +
  fabsf(p_Kalman->d_last_estimate - p_Kalman->d_current_estimate) * p_Kalman->d_q;

  p_Kalman->d_last_estimate = p_Kalman->d_current_estimate;

  return p_Kalman->d_current_estimate;
}