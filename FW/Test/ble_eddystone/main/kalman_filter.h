#ifndef KALMAN_FILTER_H_
#define KALMAN_FILTER_H_

#include <stdint.h>

// Định nghĩa cấu trúc Kalman
typedef struct {
    double Q_value;     // Độ nhiễu của quá trình (process noise covariance)
    double Q_bias;      // Độ nhiễu bias (process noise covariance for bias)
    double R_measure;   // Độ nhiễu của đo lường (measurement noise covariance)
    double value;       // Giá trị RSSI được ước tính bởi bộ lọc Kalman
    double bias;        // Bias được ước tính
    double P[2][2];     // Ma trận hiệp phương sai
} Kalman_t;

double Kalman_GetRSSI(Kalman_t *Kalman, double newRSSI, double dt);

#endif