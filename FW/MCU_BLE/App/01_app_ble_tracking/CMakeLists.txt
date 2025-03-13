idf_component_register(
    SRCS "app_ble_tracking.c"
    INCLUDE_DIRS "."
    REQUIRES bt kalman_filter 08_app_data
)
