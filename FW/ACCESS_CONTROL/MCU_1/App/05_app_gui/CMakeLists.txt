set (sources  "gui/ui.c" 
              "gui/ui_helpers.c" 
              "gui/screens/ui_Home.c"
              "gui/screens/ui_Menu.c"
              "gui/screens/ui_Attendance.c"
              "gui/screens/ui_UserData.c"
              "gui/screens/ui_Authenticate.c"
              "gui/screens/ui_Enroll.c"
              "gui/screens/ui_FaceIDEnroll.c"
              "gui/screens/ui_FingerEnroll.c"
              "gui/components/ui_comp_hook.c"
              "app_display/app_display.c")

set (includes   "gui"
                "app_display")

idf_component_register(SRCS ${sources}
                           INCLUDE_DIRS ${includes}
                           REQUIRES lvgl driver ILI9341 XPT2046 00_app_data
                           PRIV_REQUIRES espressif__esp32-camera)