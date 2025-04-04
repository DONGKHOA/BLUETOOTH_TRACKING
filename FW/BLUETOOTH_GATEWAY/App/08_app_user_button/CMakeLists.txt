idf_component_register(
    SRCS "app_user_button.c"
    INCLUDE_DIRS "."
    REQUIRES GPIO 00_app_data EXTI
)