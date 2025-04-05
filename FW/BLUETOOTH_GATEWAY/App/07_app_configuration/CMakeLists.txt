idf_component_register(
    SRCS "app_configuration.c"
    INCLUDE_DIRS "."
    REQUIRES WiFi 00_app_data bt NVS
)