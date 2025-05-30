// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////

// SCREEN: ui_Home
void ui_Home_screen_init(void);
lv_obj_t * ui_Home;
lv_obj_t * ui_HomeText;
lv_obj_t * ui_POPUPHomePanel;
void ui_event_ButtonHomeToMenu(lv_event_t * e);
lv_obj_t * ui_ButtonHomeToMenu;
// CUSTOM VARIABLES

// SCREEN: ui_Menu
void ui_Menu_screen_init(void);
lv_obj_t * ui_Menu;
lv_obj_t * ui_MenuText;
lv_obj_t * ui_POPUPMenuPanel;
void ui_event_AttendanceButton(lv_event_t * e);
lv_obj_t * ui_AttendanceButton;
lv_obj_t * ui_Label1;
void ui_event_EnrollButton(lv_event_t * e);
lv_obj_t * ui_EnrollButton;
lv_obj_t * ui_Label2;
void ui_event_ButtonMenuToHome(lv_event_t * e);
lv_obj_t * ui_ButtonMenuToHome;
// CUSTOM VARIABLES

// SCREEN: ui_Attendance
void ui_Attendance_screen_init(void);
void ui_event_Attendance(lv_event_t * e);
lv_obj_t * ui_Attendance;
// CUSTOM VARIABLES

// SCREEN: ui_Authenticate
void ui_Authenticate_screen_init(void);
void ui_event_Authenticate(lv_event_t * e);
lv_obj_t * ui_Authenticate;
void ui_event_ESCAuthenticateButton(lv_event_t * e);
lv_obj_t * ui_ESCAuthenticateButton;
lv_obj_t * ui_ESCAuthenticateText;
void ui_event_HomeAuthenticateButton(lv_event_t * e);
lv_obj_t * ui_HomeAuthenticateButton;
lv_obj_t * ui_HomeEnrollText2;
lv_obj_t * ui_KeyboardAuthenticatePanel;
void ui_event_ButtonAuthenticate1(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate1;
lv_obj_t * ui_Number20;
void ui_event_ButtonAuthenticate2(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate2;
lv_obj_t * ui_Number10;
void ui_event_ButtonAuthenticate5(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate5;
lv_obj_t * ui_Number11;
void ui_event_ButtonAuthenticate8(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate8;
lv_obj_t * ui_Number12;
void ui_event_ButtonAuthenticate4(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate4;
lv_obj_t * ui_Number13;
void ui_event_ButtonAuthenticate7(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate7;
lv_obj_t * ui_Number14;
void ui_event_ButtonAuthenticate3(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate3;
lv_obj_t * ui_Number15;
void ui_event_ButtonAuthenticate6(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate6;
lv_obj_t * ui_Number16;
void ui_event_ButtonAuthenticate9(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate9;
lv_obj_t * ui_Number17;
void ui_event_DelButtonAuthenticate(lv_event_t * e);
lv_obj_t * ui_DelButtonAuthenticate;
void ui_event_EnterButtonAuthenticate(lv_event_t * e);
lv_obj_t * ui_EnterButtonAuthenticate;
void ui_event_ButtonAuthenticate0(lv_event_t * e);
lv_obj_t * ui_ButtonAuthenticate0;
lv_obj_t * ui_Number18;
lv_obj_t * ui_PassIDAuthenticatePanel;
lv_obj_t * ui_AuthenticateText;
// CUSTOM VARIABLES

// SCREEN: ui_Enroll
void ui_Enroll_screen_init(void);
lv_obj_t * ui_Enroll;
void ui_event_ESCEnrollButton(lv_event_t * e);
lv_obj_t * ui_ESCEnrollButton;
lv_obj_t * ui_ESCEnrollText;
void ui_event_HomeEnrollButton(lv_event_t * e);
lv_obj_t * ui_HomeEnrollButton;
lv_obj_t * ui_HomeEnrollText;
lv_obj_t * ui_IDEnrollPane;
lv_obj_t * ui_IDEnrollText;
lv_obj_t * ui_EnrollText;
lv_obj_t * ui_KeyboardEnrollPanel;
void ui_event_Button1(lv_event_t * e);
lv_obj_t * ui_Button1;
lv_obj_t * ui_Number1;
void ui_event_Button2(lv_event_t * e);
lv_obj_t * ui_Button2;
lv_obj_t * ui_Number2;
void ui_event_Button5(lv_event_t * e);
lv_obj_t * ui_Button5;
lv_obj_t * ui_Number5;
void ui_event_Button8(lv_event_t * e);
lv_obj_t * ui_Button8;
lv_obj_t * ui_Number8;
void ui_event_Button4(lv_event_t * e);
lv_obj_t * ui_Button4;
lv_obj_t * ui_Number4;
void ui_event_Button7(lv_event_t * e);
lv_obj_t * ui_Button7;
lv_obj_t * ui_Number7;
void ui_event_Button3(lv_event_t * e);
lv_obj_t * ui_Button3;
lv_obj_t * ui_Number3;
void ui_event_Button6(lv_event_t * e);
lv_obj_t * ui_Button6;
lv_obj_t * ui_Number6;
void ui_event_Button9(lv_event_t * e);
lv_obj_t * ui_Button9;
lv_obj_t * ui_Number9;
void ui_event_DelButton(lv_event_t * e);
lv_obj_t * ui_DelButton;
void ui_event_EnterButton(lv_event_t * e);
lv_obj_t * ui_EnterButton;
void ui_event_Button0(lv_event_t * e);
lv_obj_t * ui_Button0;
lv_obj_t * ui_Number0;
// CUSTOM VARIABLES

// SCREEN: ui_UserInfo
void ui_UserInfo_screen_init(void);
void ui_event_UserInfo(lv_event_t * e);
lv_obj_t * ui_UserInfo;
lv_obj_t * ui_POPUPMenuPanel3;
lv_obj_t * ui_IDTextEnroll;
lv_obj_t * ui_Panel1;
lv_obj_t * ui_Panel3;
lv_obj_t * ui_Finger;
lv_obj_t * ui_FaceID;
void ui_event_EnrollFingerButton(lv_event_t * e);
lv_obj_t * ui_EnrollFingerButton;
lv_obj_t * ui_Label4;
void ui_event_EnrollFaceIDButton(lv_event_t * e);
lv_obj_t * ui_EnrollFaceIDButton;
lv_obj_t * ui_Label5;
lv_obj_t * ui_UserInfoText;
void ui_event_ESCButton(lv_event_t * e);
lv_obj_t * ui_ESCButton;
lv_obj_t * ui_ESCText;
void ui_event_HomeButton(lv_event_t * e);
lv_obj_t * ui_HomeButton;
lv_obj_t * ui_HomeText1;
// CUSTOM VARIABLES

// SCREEN: ui_FaceIDEnroll
void ui_FaceIDEnroll_screen_init(void);
void ui_event_FaceIDEnroll(lv_event_t * e);
lv_obj_t * ui_FaceIDEnroll;
// CUSTOM VARIABLES

// SCREEN: ui_FingerEnroll
void ui_FingerEnroll_screen_init(void);
lv_obj_t * ui_FingerEnroll;
lv_obj_t * ui_FP_Enroll;
lv_obj_t * ui_POPUPMenuPanel4;
lv_obj_t * ui_IDTextEnroll3;
// CUSTOM VARIABLES

// EVENTS
void ui_event____initial_actions0(lv_event_t * e);
lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=1
    #error "LV_COLOR_16_SWAP should be 1 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_ButtonHomeToMenu(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Menu, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_Menu_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Home_After(e);
        EVENT_Menu_Before(e);
    }
}

void ui_event_AttendanceButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Attendance, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_Attendance_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Attendance_Before(e);
        EVENT_Menu_After(e);
    }
}

void ui_event_EnrollButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Menu_To_Authenticate(e);
        EVENT_Menu_After(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Authenticate, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_Authenticate_screen_init);
    }
}

void ui_event_ButtonMenuToHome(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Home_Before(e);
        EVENT_Menu_After(e);
    }
}

void ui_event_Attendance(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Attendance_After(e);
        EVENT_Home_Before(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);
    }
}

void ui_event_Authenticate(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_To_Enroll(e);
    }
}

void ui_event_ESCAuthenticateButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Menu_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Menu_Before(e);
        EVENT_Authenticate_After(e);
    }
}

void ui_event_HomeAuthenticateButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Home_Before(e);
        EVENT_Authenticate_After(e);
    }
}

void ui_event_ButtonAuthenticate1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button1(e);
    }
}

void ui_event_ButtonAuthenticate2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button2(e);
    }
}

void ui_event_ButtonAuthenticate5(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button5(e);
    }
}

void ui_event_ButtonAuthenticate8(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button8(e);
    }
}

void ui_event_ButtonAuthenticate4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button4(e);
    }
}

void ui_event_ButtonAuthenticate7(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button7(e);
    }
}

void ui_event_ButtonAuthenticate3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button3(e);
    }
}

void ui_event_ButtonAuthenticate6(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button6(e);
    }
}

void ui_event_ButtonAuthenticate9(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button9(e);
    }
}

void ui_event_DelButtonAuthenticate(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_DelButton(e);
    }
}

void ui_event_EnterButtonAuthenticate(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_EnterButton(e);
    }
}

void ui_event_ButtonAuthenticate0(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_Button0(e);
    }
}

void ui_event_ESCEnrollButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Menu_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Menu_Before(e);
        EVENT_Enroll_After(e);
    }
}

void ui_event_HomeEnrollButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Home_Before(e);
        EVENT_Enroll_After(e);
    }
}

void ui_event_Button1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button1(e);
    }
}

void ui_event_Button2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button2(e);
    }
}

void ui_event_Button5(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button5(e);
    }
}

void ui_event_Button8(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button8(e);
    }
}

void ui_event_Button4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button4(e);
    }
}

void ui_event_Button7(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button7(e);
    }
}

void ui_event_Button3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button3(e);
    }
}

void ui_event_Button6(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button6(e);
    }
}

void ui_event_Button9(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button9(e);
    }
}

void ui_event_DelButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_DelButton(e);
    }
}

void ui_event_EnterButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_EnterButton(e);
    }
}

void ui_event_Button0(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_Button0(e);
    }
}

void ui_event_UserInfo(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Enroll_To_UserInfo(e);
        EVENT_Enroll_Finger(e);
        EVENT_Enroll_FaceID(e);
    }
}

void ui_event_EnrollFingerButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_UserInfo_Finger(e);
    }
}

void ui_event_EnrollFaceIDButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        EVENT_UserInfo_FaceID(e);
    }
}

void ui_event_ESCButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Enroll, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Enroll_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Authenticate_To_Enroll(e);
        EVENT_UserInfo_Back(e);
    }
}

void ui_event_HomeButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Home_Before(e);
        EVENT_UserInfo_Back(e);
    }
}

void ui_event_FaceIDEnroll(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_Home_screen_init);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_FaceID_Back(e);
    }
    if(event_code == LV_EVENT_CLICKED) {
        EVENT_Home_Before(e);
    }
}

void ui_event____initial_actions0(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_SCREEN_LOAD_START) {
        EVENT_Home_Before(e);
    }
}

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_Home_screen_init();
    ui_Menu_screen_init();
    ui_Attendance_screen_init();
    ui_Authenticate_screen_init();
    ui_Enroll_screen_init();
    ui_UserInfo_screen_init();
    ui_FaceIDEnroll_screen_init();
    ui_FingerEnroll_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_obj_add_event_cb(ui____initial_actions0, ui_event____initial_actions0, LV_EVENT_ALL, NULL);

    lv_disp_load_scr(ui____initial_actions0);
    lv_disp_load_scr(ui_Home);
}
