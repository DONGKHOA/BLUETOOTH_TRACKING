SET(SOURCES screens/ui_Attendance.c
    screens/ui_Enroll.c
    screens/ui_Authenticate.c
    screens/ui_FaceIDPopup.c
    screens/ui_FingerPopup.c
    screens/ui_ExpirePopup.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c)

add_library(ui ${SOURCES})
