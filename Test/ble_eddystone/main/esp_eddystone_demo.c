/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */


/****************************************************************************
*
* This file is used for eddystone receiver.
*
****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "esp_bt.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"
#include "esp_gap_ble_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_eddystone_protocol.h"
#include "esp_eddystone_api.h"
// #include "kalman_filter.h"

// Định nghĩa cấu trúc Kalman
typedef struct {
    double Q_value;     // Độ nhiễu của quá trình (process noise covariance)
    double Q_bias;      // Độ nhiễu bias (process noise covariance for bias)
    double R_measure;   // Độ nhiễu của đo lường (measurement noise covariance)
    double value;       // Giá trị RSSI được ước tính bởi bộ lọc Kalman
    double bias;        // Bias được ước tính
    double P[2][2];     // Ma trận hiệp phương sai
} Kalman_t;

static const char* DEMO_TAG = "EDDYSTONE_DEMO";

/* declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

Kalman_t Kalman_rssi = {
    .Q_value = 0.00005f,
    .Q_bias = 0.00005f,
    .R_measure = 0.15f
};

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

static TimerHandle_t kalman_filter_timer;
static int count = 0;
static int timer = 0;

static void KALMAN_FILTER_timer_cb(TimerHandle_t xTimer)
{
    count++;
    timer++;
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param)
{
    esp_err_t err;

    switch(event)
    {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
            uint32_t duration = 0;
            esp_ble_gap_start_scanning(duration);
            break;
        }
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
            if((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(DEMO_TAG,"Scan start failed: %s", esp_err_to_name(err));
            }
            else {
                ESP_LOGI(DEMO_TAG,"Start scanning...");
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            esp_ble_gap_cb_param_t* scan_result = (esp_ble_gap_cb_param_t*)param;
            switch(scan_result->scan_rst.search_evt)
            {
                case ESP_GAP_SEARCH_INQ_RES_EVT: {
                    esp_eddystone_result_t eddystone_res;
                    memset(&eddystone_res, 0, sizeof(eddystone_res));
                    esp_err_t ret = esp_eddystone_decode(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len, &eddystone_res);
                    if (ret) {
                        // error:The received data is not an eddystone frame packet or a correct eddystone frame packet.
                        // just return
                        return;
                    } else {
                        // The received adv data is a correct eddystone frame packet.
                        // Here, we get the eddystone information in eddystone_res, we can use the data in res to do other things.
                        // For example, just print them:
                        // ESP_LOGI(DEMO_TAG, "--------Eddystone Found----------");
                        // esp_log_buffer_hex("EDDYSTONE_DEMO: Device address:", scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);

                        // ESP_LOGI(DEMO_TAG, "RSSI RAW: %d dbm\n", scan_result->scan_rst.rssi);
                        double smoothedRSSI  = Kalman_GetRSSI(&Kalman_rssi, scan_result->scan_rst.rssi, count/1000);
                        printf("%d,%.0f,%d\n",timer, smoothedRSSI, scan_result->scan_rst.rssi);
                        count = 0;
                        if (timer >= 300000)
                        {
                            ESP_LOGI(DEMO_TAG, "--------Eddystone Found----------");
                            timer = 0;
                        }
                        
                        // ESP_LOGI(DEMO_TAG, "RSSI FILTER: %d dbm\n\n", (int)smoothedRSSI);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

void esp_eddystone_appRegister(void)
{
    esp_err_t status;

    ESP_LOGI(DEMO_TAG,"Register callback");

    /*<! register the scan callback function to the gap module */
    if((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
        ESP_LOGE(DEMO_TAG,"gap register error: %s", esp_err_to_name(status));
        return;
    }
}

void esp_eddystone_init(void)
{
    esp_bluedroid_init();
    esp_bluedroid_enable();
    esp_eddystone_appRegister();
}

void app_main(void)
{
    kalman_filter_timer = xTimerCreate("kalman filter subscribe",
                                    1 / portTICK_PERIOD_MS,
                                    pdTRUE, (void *)0, KALMAN_FILTER_timer_cb);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);

    esp_eddystone_init();

    /*<! set scan parameters */
    esp_ble_gap_set_scan_params(&ble_scan_params);
    xTimerStart(kalman_filter_timer, 0);
}
