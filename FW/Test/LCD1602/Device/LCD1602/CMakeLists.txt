idf_component_register(
    SRCS "lcd1602.c"
    INCLUDE_DIRS "."
    REQUIRES I2C driver
    PRIV_REQUIRES "freertos"
)