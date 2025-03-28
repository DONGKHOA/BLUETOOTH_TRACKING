file(GLOB_RECURSE gui_sources 
    "gui/components/*.c"
    "gui/screens/*.c"
    "gui/*.c"
)

list(APPEND sources ${gui_sources})

set (sources    "app_display/app_display.c"
                "events/attendance_event.c"
                "events/enroll_event.c"
                "events/data_event.c")

set (includes   "gui"
                "app_display")

idf_component_register(SRCS ${sources} ${gui_sources}
                           INCLUDE_DIRS ${includes}
                           REQUIRES lvgl driver ILI9341 XPT2046 00_app_data
                           PRIV_REQUIRES espressif__esp32-camera "freertos")