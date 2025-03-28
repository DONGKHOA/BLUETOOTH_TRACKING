SET(SOURCES components/ui_comp_popupmenupanel.c
    components/ui_comp.c
    screens/ui_Home.c
    screens/ui_Menu.c
    screens/ui_Attendance.c
    screens/ui_UserData.c
    screens/ui_Authenticate.c
    screens/ui_Enroll.c
    screens/ui_FaceIDEnroll.c
    screens/ui_FingerEnroll.c
    screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c)

add_library(ui ${SOURCES})
