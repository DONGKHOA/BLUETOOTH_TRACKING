idf_component_register(
    SRCS "app_ble_tracking.c"
    INCLUDE_DIRS "."
    REQUIRES bt kalman_filter 06_app_data WiFi
)
