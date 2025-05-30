// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "components/ui_comp.h"
#include "components/ui_comp_hook.h"
#include "ui_events.h"


// SCREEN: ui_Home
void ui_Home_screen_init(void);
extern lv_obj_t * ui_Home;
extern lv_obj_t * ui_HomeText;
extern lv_obj_t * ui_POPUPHomePanel;
void ui_event_ButtonHomeToMenu(lv_event_t * e);
extern lv_obj_t * ui_ButtonHomeToMenu;
// CUSTOM VARIABLES

// SCREEN: ui_Menu
void ui_Menu_screen_init(void);
extern lv_obj_t * ui_Menu;
extern lv_obj_t * ui_MenuText;
extern lv_obj_t * ui_POPUPMenuPanel;
void ui_event_AttendanceButton(lv_event_t * e);
extern lv_obj_t * ui_AttendanceButton;
extern lv_obj_t * ui_Label1;
void ui_event_EnrollButton(lv_event_t * e);
extern lv_obj_t * ui_EnrollButton;
extern lv_obj_t * ui_Label2;
void ui_event_ButtonMenuToHome(lv_event_t * e);
extern lv_obj_t * ui_ButtonMenuToHome;
// CUSTOM VARIABLES

// SCREEN: ui_Attendance
void ui_Attendance_screen_init(void);
void ui_event_Attendance(lv_event_t * e);
extern lv_obj_t * ui_Attendance;
// CUSTOM VARIABLES

// SCREEN: ui_Authenticate
void ui_Authenticate_screen_init(void);
void ui_event_Authenticate(lv_event_t * e);
extern lv_obj_t * ui_Authenticate;
void ui_event_ESCAuthenticateButton(lv_event_t * e);
extern lv_obj_t * ui_ESCAuthenticateButton;
extern lv_obj_t * ui_ESCAuthenticateText;
void ui_event_HomeAuthenticateButton(lv_event_t * e);
extern lv_obj_t * ui_HomeAuthenticateButton;
extern lv_obj_t * ui_HomeEnrollText2;
extern lv_obj_t * ui_KeyboardAuthenticatePanel;
void ui_event_ButtonAuthenticate1(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate1;
extern lv_obj_t * ui_Number20;
void ui_event_ButtonAuthenticate2(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate2;
extern lv_obj_t * ui_Number10;
void ui_event_ButtonAuthenticate5(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate5;
extern lv_obj_t * ui_Number11;
void ui_event_ButtonAuthenticate8(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate8;
extern lv_obj_t * ui_Number12;
void ui_event_ButtonAuthenticate4(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate4;
extern lv_obj_t * ui_Number13;
void ui_event_ButtonAuthenticate7(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate7;
extern lv_obj_t * ui_Number14;
void ui_event_ButtonAuthenticate3(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate3;
extern lv_obj_t * ui_Number15;
void ui_event_ButtonAuthenticate6(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate6;
extern lv_obj_t * ui_Number16;
void ui_event_ButtonAuthenticate9(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate9;
extern lv_obj_t * ui_Number17;
void ui_event_DelButtonAuthenticate(lv_event_t * e);
extern lv_obj_t * ui_DelButtonAuthenticate;
void ui_event_EnterButtonAuthenticate(lv_event_t * e);
extern lv_obj_t * ui_EnterButtonAuthenticate;
void ui_event_ButtonAuthenticate0(lv_event_t * e);
extern lv_obj_t * ui_ButtonAuthenticate0;
extern lv_obj_t * ui_Number18;
extern lv_obj_t * ui_PassIDAuthenticatePanel;
extern lv_obj_t * ui_AuthenticateText;
// CUSTOM VARIABLES

// SCREEN: ui_Enroll
void ui_Enroll_screen_init(void);
extern lv_obj_t * ui_Enroll;
void ui_event_ESCEnrollButton(lv_event_t * e);
extern lv_obj_t * ui_ESCEnrollButton;
extern lv_obj_t * ui_ESCEnrollText;
void ui_event_HomeEnrollButton(lv_event_t * e);
extern lv_obj_t * ui_HomeEnrollButton;
extern lv_obj_t * ui_HomeEnrollText;
extern lv_obj_t * ui_IDEnrollPane;
extern lv_obj_t * ui_IDEnrollText;
extern lv_obj_t * ui_EnrollText;
extern lv_obj_t * ui_KeyboardEnrollPanel;
void ui_event_Button1(lv_event_t * e);
extern lv_obj_t * ui_Button1;
extern lv_obj_t * ui_Number1;
void ui_event_Button2(lv_event_t * e);
extern lv_obj_t * ui_Button2;
extern lv_obj_t * ui_Number2;
void ui_event_Button5(lv_event_t * e);
extern lv_obj_t * ui_Button5;
extern lv_obj_t * ui_Number5;
void ui_event_Button8(lv_event_t * e);
extern lv_obj_t * ui_Button8;
extern lv_obj_t * ui_Number8;
void ui_event_Button4(lv_event_t * e);
extern lv_obj_t * ui_Button4;
extern lv_obj_t * ui_Number4;
void ui_event_Button7(lv_event_t * e);
extern lv_obj_t * ui_Button7;
extern lv_obj_t * ui_Number7;
void ui_event_Button3(lv_event_t * e);
extern lv_obj_t * ui_Button3;
extern lv_obj_t * ui_Number3;
void ui_event_Button6(lv_event_t * e);
extern lv_obj_t * ui_Button6;
extern lv_obj_t * ui_Number6;
void ui_event_Button9(lv_event_t * e);
extern lv_obj_t * ui_Button9;
extern lv_obj_t * ui_Number9;
void ui_event_DelButton(lv_event_t * e);
extern lv_obj_t * ui_DelButton;
void ui_event_EnterButton(lv_event_t * e);
extern lv_obj_t * ui_EnterButton;
void ui_event_Button0(lv_event_t * e);
extern lv_obj_t * ui_Button0;
extern lv_obj_t * ui_Number0;
// CUSTOM VARIABLES

// SCREEN: ui_UserInfo
void ui_UserInfo_screen_init(void);
void ui_event_UserInfo(lv_event_t * e);
extern lv_obj_t * ui_UserInfo;
extern lv_obj_t * ui_POPUPMenuPanel3;
extern lv_obj_t * ui_IDTextEnroll;
extern lv_obj_t * ui_Panel1;
extern lv_obj_t * ui_Panel3;
extern lv_obj_t * ui_Finger;
extern lv_obj_t * ui_FaceID;
void ui_event_EnrollFingerButton(lv_event_t * e);
extern lv_obj_t * ui_EnrollFingerButton;
extern lv_obj_t * ui_Label4;
void ui_event_EnrollFaceIDButton(lv_event_t * e);
extern lv_obj_t * ui_EnrollFaceIDButton;
extern lv_obj_t * ui_Label5;
extern lv_obj_t * ui_UserInfoText;
void ui_event_ESCButton(lv_event_t * e);
extern lv_obj_t * ui_ESCButton;
extern lv_obj_t * ui_ESCText;
void ui_event_HomeButton(lv_event_t * e);
extern lv_obj_t * ui_HomeButton;
extern lv_obj_t * ui_HomeText1;
// CUSTOM VARIABLES

// SCREEN: ui_FaceIDEnroll
void ui_FaceIDEnroll_screen_init(void);
void ui_event_FaceIDEnroll(lv_event_t * e);
extern lv_obj_t * ui_FaceIDEnroll;
// CUSTOM VARIABLES

// SCREEN: ui_FingerEnroll
void ui_FingerEnroll_screen_init(void);
extern lv_obj_t * ui_FingerEnroll;
extern lv_obj_t * ui_FP_Enroll;
extern lv_obj_t * ui_POPUPMenuPanel4;
extern lv_obj_t * ui_IDTextEnroll3;
// CUSTOM VARIABLES

// EVENTS

void ui_event____initial_actions0(lv_event_t * e);
extern lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS
LV_IMG_DECLARE(ui_img_1101766569);    // assets/right-arrow.png
LV_IMG_DECLARE(ui_img_388382192);    // assets/left-arrow.png
LV_IMG_DECLARE(ui_img_1048813395);    // assets/x-icon-resize1.png
LV_IMG_DECLARE(ui_img_1406808990);    // assets/tick-icon-resize1.png

// UI INIT
void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
