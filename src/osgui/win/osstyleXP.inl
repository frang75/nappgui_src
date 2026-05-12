/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
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

void _osstyleXP_CloseTheme(HTHEME theme);

void _osstyleXP_GetThemeBackgroundContentRect(HTHEME theme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect);

void _osstyleXP_DrawThemeBackground2(HTHEME theme, int iPartId, int iStateId, HDC hdc, const RECT *rc);

void _osstyleXP_DrawThemeBackground(HTHEME theme, HWND hwnd, HDC hdc, int iPartId, int iStateId, const BOOL draw_parent_bg, const RECT *rc, RECT *border);

void _osstyleXP_DrawNonThemedButtonBackground(HWND hwnd, HDC hdc, BOOL has_focus, UINT state, const RECT *rc, RECT *border);

void _osstyleXP_DrawThemeText2(HTHEME theme, HDC hdc, int iPartId, int iStateId, const WCHAR *text, int num_chars, DWORD flags, const RECT *rc);

void _osstyleXP_DrawThemeText(HTHEME theme, HDC hdc, int iPartId, int iStateId, const WCHAR *text, uint32_t num_chars, DWORD flags, const RECT *rc);

__END_C
