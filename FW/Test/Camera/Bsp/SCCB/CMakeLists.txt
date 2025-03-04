set(component_srcs "sccb.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_timer GPIO)
