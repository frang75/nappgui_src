/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osstyleXP.inl
 *
 */

/* Windows XP Styles */

#include "osgui_win.ixx"

__EXTERN_C

void _osstyleXP_init(void);

void _osstyleXP_finish(void);

HTHEME _osstyleXP_OpenTheme(HWND hwnd, LPCWSTR pszClassList);

BOOL _osstyleXP_OpenThemeData(HWND hwnd, LPCWSTR pszClassList);

void _osstyleXP_CloseTheme(HTHEME theme);

void _osstyleXP_CloseThemeData(void);

void _osstyleXP_GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect);

void _osstyleXP_DrawThemeBackground2(HTHEME theme, int iPartId, int iStateId, HDC hdc, const RECT *rc);

void _osstyleXP_DrawThemeBackground(HWND hwnd, HDC hdc, int iPartId, int iStateId, const BOOL draw_parent_bg, const RECT *rc, RECT *border);

void _osstyleXP_DrawNonThemedButtonBackground(HWND hwnd, HDC hdc, BOOL has_focus, UINT state, const RECT *rc, RECT *border);

void _osstyleXP_DrawThemeEdge(HDC hdc, int iPartId, int iStateId, const RECT *rc);

BOOL _osstyleXP_HasThemeFont(HDC hdc, int iPartId, int iStateId, int iPropId);

void _osstyleXP_DrawThemeText2(HTHEME theme, HDC hdc, int iPartId, int iStateId, const WCHAR *text, int num_chars, DWORD flags, const RECT *rc);

void _osstyleXP_DrawThemeText(HDC hdc, int iPartId, int iStateId, const WCHAR *text, uint32_t num_chars, DWORD flags, const RECT *rc);

__END_C
