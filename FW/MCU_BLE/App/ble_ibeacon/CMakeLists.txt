idf_component_register(
    SRCS "app_ble_ibeacon.c"
    INCLUDE_DIRS "."
    REQUIRES kalman_filter Linked_List Data main bt
)
