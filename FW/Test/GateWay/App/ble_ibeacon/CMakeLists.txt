idf_component_register(
    SRCS "app_ble_ibeacon.c"
    INCLUDE_DIRS "."
    REQUIRES kalman_filter Data main bt
)
