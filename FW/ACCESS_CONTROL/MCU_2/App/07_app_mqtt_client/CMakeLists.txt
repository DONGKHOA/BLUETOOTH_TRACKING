idf_component_register(SRCS "app_mqtt_client.c" 
                            "handle_data/decode.c" 
                            "handle_data/encode.c"
                       INCLUDE_DIRS "."
                       REQUIRES 00_app_data "mqtt" "json" Env WiFi
                       )