idf_component_register(
    SRCS "xpt2046.c"
    INCLUDE_DIRS "."
    REQUIRES lvgl SPI driver freertos
)