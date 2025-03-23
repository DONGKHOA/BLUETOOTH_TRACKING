idf_component_register(SRCS "app_mqtt_client.c" "json_generate.c"
                       INCLUDE_DIRS "."
                       REQUIRES 06_app_data "mqtt" WiFi "json"
                       )