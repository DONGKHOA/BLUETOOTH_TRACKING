set(component_srcs "wifi.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES "freertos" "lwip" "esp_wifi" NVS
                       )