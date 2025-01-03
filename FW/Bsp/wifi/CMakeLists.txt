idf_component_register(SRCS "wifi.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES "freertos" "lwip" "esp_wifi" nvs 
                       )