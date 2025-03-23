set (sources  "gui/ui.c" 
              "gui/ui_helpers.c" 
              "gui/components/ui_comp_hook.c" 
              "gui/screens/ui_Screen1.c" 
              "gui/screens/ui_Screen2.c"
              "app_display/app_display.c")

set (includes   "gui"
                "app_display")

idf_component_register(SRCS ${sources}
                           INCLUDE_DIRS ${includes}
                           REQUIRES lvgl driver ILI9341 XPT2046 00_app_data)