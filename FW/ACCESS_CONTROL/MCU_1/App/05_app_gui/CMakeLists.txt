add_subdirectory(gui)

set (sources  "app_display/app_display.c")

set (includes   "gui"
                "app_display")

idf_component_register(SRCS ${sources}
                           INCLUDE_DIRS ${includes}
                           REQUIRES lvgl driver ILI9341 XPT2046 00_app_data
                           PRIV_REQUIRES espressif__esp32-camera)