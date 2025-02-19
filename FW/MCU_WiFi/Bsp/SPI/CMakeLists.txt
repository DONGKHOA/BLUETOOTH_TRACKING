set(component_srcs "spi.c" "sdspi.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES driver MyFATFS esp_timer GPIO)
