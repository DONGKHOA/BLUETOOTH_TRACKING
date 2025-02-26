set(component_srcs "dht22.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES driver GPIO esp_timer)