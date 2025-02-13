idf_component_register(
    SRCS "app_ble_tracking.c"
    INCLUDE_DIRS "."
    REQUIRES bt ble_eddystone kalman_filter Data main
)
