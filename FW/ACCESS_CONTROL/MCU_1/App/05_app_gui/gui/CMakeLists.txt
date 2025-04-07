SET(SOURCES components/ui_comp_popupmenupanel.c
    components/ui_comp.c
    screens/ui_Home.c
    screens/ui_Menu.c
    screens/ui_Attendance.c
    screens/ui_Authenticate.c
    screens/ui_Enroll.c
    screens/ui_UserInfo.c
    screens/ui_FaceIDEnroll.c
    screens/ui_FingerEnroll.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_1101766569.c
    images/ui_img_1052123464.c
    images/ui_img_694049672.c
    images/ui_img_988180266.c
    images/ui_img_1985633929.c
    images/ui_img_388382192.c
    images/ui_img_1048813395.c
    images/ui_img_1406808990.c)

add_library(ui ${SOURCES})
