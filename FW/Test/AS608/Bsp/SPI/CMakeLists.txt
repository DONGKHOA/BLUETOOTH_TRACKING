set(component_srcs "spi.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_timer GPIO)
