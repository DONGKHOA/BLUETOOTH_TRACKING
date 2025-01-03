/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "kalman_filter.h"

/******************************************************************************
 *   GLOBAL FUNCTIONS
 *****************************************************************************/

double Kalman_GetRSSI(Kalman_t *Kalman, double newRSSI, double dt) {
    // 1. Dự đoán (Prediction)
    double rate = -Kalman->bias;       // RSSI không có bias (ở đây không sử dụng tốc độ thay đổi)
    Kalman->value += dt * rate;        // Dự đoán giá trị tiếp theo

    // Cập nhật ma trận hiệp phương sai
    Kalman->P[0][0] += dt * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0] + Kalman->Q_value);
    Kalman->P[0][1] -= dt * Kalman->P[1][1];
    Kalman->P[1][0] -= dt * Kalman->P[1][1];
    Kalman->P[1][1] += Kalman->Q_bias * dt;

    // 2. Đo lường (Measurement Update)
    double S = Kalman->P[0][0] + Kalman->R_measure; // Độ tin cậy của đo lường
    double K[2];                                   // Kalman Gain
    K[0] = Kalman->P[0][0] / S;
    K[1] = Kalman->P[1][0] / S;

    // Sai số đo lường (Innovation)
    double y = newRSSI - Kalman->value;

    // Cập nhật giá trị RSSI và bias
    Kalman->value += K[0] * y;
    Kalman->bias += K[1] * y;

    // Cập nhật ma trận hiệp phương sai
    double P00_temp = Kalman->P[0][0];
    double P01_temp = Kalman->P[0][1];

    Kalman->P[0][0] -= K[0] * P00_temp;
    Kalman->P[0][1] -= K[0] * P01_temp;
    Kalman->P[1][0] -= K[1] * P00_temp;
    Kalman->P[1][1] -= K[1] * P01_temp;

    // Trả về giá trị RSSI đã được làm mượt
    return Kalman->value;
}