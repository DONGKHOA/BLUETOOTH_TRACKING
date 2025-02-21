set(component_srcs "rs485.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES driver UART esp_timer GPIO
                       )