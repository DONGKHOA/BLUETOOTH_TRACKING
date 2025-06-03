idf_component_register(
    SRCS "app_display.c"
    INCLUDE_DIRS "."
    REQUIRES 00_app_data LCD1602 I2C 
)