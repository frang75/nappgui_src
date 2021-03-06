/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui_win.inl
 *
 */

/* Operating system native gui */

#include "osgui_win.ixx"

__EXTERN_C

HINSTANCE _osgui_instance(void);

BOOL _osgui_hit_test(HWND hwnd);

void _osgui_add_accelerator(BYTE fVirt, WORD key, WORD cmd);

void _osgui_remove_accelerator(WORD cmd);

void _osgui_change_accelerator(BYTE fVirt, WORD key, WORD cmd);

bool_t _osgui_OnMsg(MSG *msg);

extern const WORD kVIRTUAL_KEY[];
extern uint32_t kNUM_VKEYS;
extern HWND kDEFAULT_PARENT_WINDOW;
extern HCURSOR kNORMAL_ARROW_CURSOR;
extern HCURSOR kSIZING_HORIZONTAL_CURSOR;
extern HCURSOR kSIZING_VERTICAL_CURSOR;
extern HBRUSH kCHESSBOARD_BRUSH;
extern const TCHAR *kWINDOW_CLASS;
extern const TCHAR *kVIEW_CLASS;
//extern const TCHAR *kLISTVIEW_CLASS;
extern const TCHAR *kRICHEDIT_CLASS;
extern unicode_t kWINDOWS_UNICODE;

__END_C
