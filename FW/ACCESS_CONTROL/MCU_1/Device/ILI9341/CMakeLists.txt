idf_component_register(
    SRCS "ili9341.c"
    INCLUDE_DIRS "."
    REQUIRES lvgl SPI driver freertos
)