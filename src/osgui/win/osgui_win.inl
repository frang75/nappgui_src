/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
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

uint16_t _osgui_unique_child_id(void);

void _osgui_add_accelerator(BYTE fVirt, WORD key, WORD cmd, HWND hwnd);

void _osgui_remove_accelerator(WORD cmd);

void _osgui_change_accelerator(BYTE fVirt, WORD key, WORD cmd);

HACCEL _osgui_accel_table(void);

HWND _osgui_hwnd_accelerator(WORD cmd);

LRESULT _osgui_nccalcsize(HWND hwnd, WPARAM wParam, LPARAM lParam, bool_t expand, INT ypadding, RECT *border);

LRESULT _osgui_ncpaint(HWND hwnd, const bool_t focused, const RECT *border, HBRUSH padding_bgcolor);

void _osgui_frame_without_shadows(const HWND hwnd, RECT *rect);

bool_t _osgui_dpi_aware(void);

void _osgui_activate_dpi_awareness(void);

UINT _osgui_dpi_for_window(HWND hwnd);

UINT _osgui_dpi_for_primary_monitor(void);

int _osgui_system_metrics_for_dpi(int index, UINT dpi);

BOOL _osgui_adjust_window_rect_ex_for_dpi(RECT *rect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);

vkey_t _osgui_vkey(const WORD key);

uint32_t _osgui_modifiers(void);

const WCHAR *_osgui_wstr_init(const char_t *text, WString *str);

void _osgui_wstr_remove(WString *str);

extern const WORD kVIRTUAL_KEY[];
extern uint32_t kNUM_VKEYS;
extern HWND kDEFAULT_PARENT_WINDOW;
extern HCURSOR kNORMAL_ARROW_CURSOR;
extern HCURSOR kSIZING_HORIZONTAL_CURSOR;
extern HCURSOR kSIZING_VERTICAL_CURSOR;
extern HBRUSH kCHESSBOARD_BRUSH;
extern const TCHAR *kWINDOW_CLASS;
extern const TCHAR *kVIEW_CLASS;
extern const TCHAR *kRICHEDIT_CLASS;
extern const TCHAR *kWEBVIEW_CLASS;
extern unicode_t kWINDOWS_UNICODE;

__END_C
