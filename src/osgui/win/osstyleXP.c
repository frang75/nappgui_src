/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osstyleXP.c
 *
 */

/* Windows XP Styles */

/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2001-2002 by David Yuheng Zhao
//
// Distribute and change freely, except: don't remove my name from the source 
//
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Partly based on the _ThemeHelper struct in MFC7.0 source code (winctrl3.cpp), 
// and the difference is that this implementation wraps the full set of 
// visual style APIs from the platform SDK August 2001
//
// If you have any questions, I can be reached as follows:
//  yuheng_zhao@yahoo.com
//
//
// How to use:
// Instead of calling the API directly, 
//    OpenThemeData(...);
// use the global variable
//    g_xpStyle.OpenThemeData(...);
//
/////////////////////////////////////////////////////////////////////////////

#include "osstyleXP.inl"
#include "heap.h"
#include "cassert.h"

#pragma comment (lib, "uxtheme.lib")

typedef struct _osstyleXP_t OSStyleXP;

struct _osstyleXP_t
{
    HMODULE themeDll;
    HTHEME theme;
};

static OSStyleXP i_STYLEXP = {NULL, NULL};

typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);

/*---------------------------------------------------------------------------*/

/*
static void* i_GetProc(HMODULE module, LPCSTR szProc, void* pfnFail)
{
    void* pRet = pfnFail;
    if (module != NULL)
        pRet = GetProcAddress(module, szProc);
    return pRet;
}*/

/*---------------------------------------------------------------------------*/

/*
static HTHEME i_OpenThemeDataFail(HWND, LPCWSTR)
{
    return NULL;
}*/

/*---------------------------------------------------------------------------*/

void osstyleXP_init(void)
{
    cassert(i_STYLEXP.themeDll == NULL);
    cassert(i_STYLEXP.theme == NULL);
    i_STYLEXP.themeDll = NULL;// LoadLibrary(L"UxTheme.dll");
    i_STYLEXP.theme = NULL;
}

/*---------------------------------------------------------------------------*/

void osstyleXP_remove(void)
{
    cassert(i_STYLEXP.theme == NULL);
    cassert(i_STYLEXP.themeDll == NULL);
    /*if ((*style)->themeDll != NULL)
        FreeLibrary((*style)->themeDll);*/
}

/*---------------------------------------------------------------------------*/

//BOOL osstyleXP_IsThemeActive(OSStyleXP *style);
//BOOL osstyleXP_IsThemeActive(OSStyleXP *style)
//{
//    unref(style);
//    return IsThemeActive();
//}

/*---------------------------------------------------------------------------*/

//BOOL osstyleXP_IsAppThemed(OSStyleXP *style);
//BOOL osstyleXP_IsAppThemed(OSStyleXP *style)
//{
//    unref(style);
//    return IsAppThemed();
//}

/*---------------------------------------------------------------------------*/

HTHEME osstyleXP_OpenTheme(HWND hwnd, LPCWSTR pszClassList)
{
    HTHEME theme = NULL;
    if (IsAppThemed() == TRUE && IsThemeActive() == TRUE)
        theme = OpenThemeData(hwnd, pszClassList);
    return theme;

    /*
    PFNOPENTHEMEDATA func = NULL;
    cassert_no_null(style);
    if (style->m_hThemeDll == NULL) 
        return NULL;
    func = (PFNOPENTHEMEDATA)GetProcAddress(style->m_hThemeDll, "OpenThemeData");
    return (*func)(hwnd, pszClassList);
    */
}

/*---------------------------------------------------------------------------*/

BOOL osstyleXP_OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
    cassert(i_STYLEXP.theme == NULL);

    if (IsAppThemed() == TRUE && IsThemeActive() == TRUE)
    {
        i_STYLEXP.theme = OpenThemeData(hwnd, pszClassList);
        cassert_no_null(i_STYLEXP.theme);
    }

    return (i_STYLEXP.theme != NULL) ? TRUE : FALSE;

    /*
    PFNOPENTHEMEDATA func = NULL;
    cassert_no_null(style);
    if (style->m_hThemeDll == NULL) 
        return NULL;
    func = (PFNOPENTHEMEDATA)GetProcAddress(style->m_hThemeDll, "OpenThemeData");
    return (*func)(hwnd, pszClassList);
    */
}

/*---------------------------------------------------------------------------*/

void osstyleXP_CloseTheme(HTHEME theme)
{
    HRESULT res = 0;
    cassert(theme != NULL);
    res = CloseThemeData(theme);
    cassert_unref(res == S_OK, res);
}

/*---------------------------------------------------------------------------*/

void osstyleXP_CloseThemeData(void)
{
    HRESULT res = 0;
    cassert(i_STYLEXP.theme != NULL);
    res = CloseThemeData(i_STYLEXP.theme);
    cassert_unref(res == S_OK, res);
    i_STYLEXP.theme = NULL;
}

/*---------------------------------------------------------------------------*/

void osstyleXP_GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect)
{
    HRESULT res = GetThemeBackgroundContentRect(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);
    cassert_unref(res == S_OK, res);
}

/*---------------------------------------------------------------------------*/

void osstyleXP_DrawThemeBackgroundNoBorder(HTHEME theme, int iPartId, int iStateId, HDC hdc, const RECT *rc)
{
    DTBGOPTS opts;
    HRESULT res;
    opts.dwSize = sizeof(DTBGOPTS);
    opts.dwFlags = DTBG_OMITBORDER;
    res = DrawThemeBackgroundEx(theme, hdc, iPartId, iStateId, rc, &opts);
    cassert_unref(res == S_OK, res);
}

/*---------------------------------------------------------------------------*/

void osstyleXP_DrawThemeBackground2(HTHEME theme, int iPartId, int iStateId, HDC hdc, const RECT *rc)
{
    HRESULT res = DrawThemeBackground(theme, hdc, iPartId, iStateId, rc, NULL);
    cassert_unref(res == S_OK, res);
}

/*---------------------------------------------------------------------------*/

void osstyleXP_DrawThemeBackground(HWND hwnd, HDC hdc, int iPartId, int iStateId, const BOOL draw_parent_bg, const RECT *rc, RECT *border)
{
    HRESULT res = 0;
    cassert_no_null(i_STYLEXP.theme);
    if (draw_parent_bg == TRUE)
    {
    #if _MSC_VER > 1400
        if (IsThemeBackgroundPartiallyTransparent(i_STYLEXP.theme, iPartId, iStateId) == TRUE)
            DrawThemeParentBackgroundEx(hwnd, hdc, DTPB_WINDOWDC | DTPB_USECTLCOLORSTATIC, (RECT*)rc);
    #else
        unref(hwnd);
    #endif
    }

    res = DrawThemeBackground(i_STYLEXP.theme, hdc, iPartId, iStateId, rc, NULL);
    cassert_unref(res == S_OK, res);

    if (border != NULL)
    {
        res = GetThemeBackgroundContentRect(i_STYLEXP.theme, hdc, iPartId, iStateId, rc, border);
        cassert_unref(res == S_OK, res);
    }
}

/*---------------------------------------------------------------------------*/

void osstyleXP_DrawNonThemedButtonBackground(HWND hwnd, HDC hdc, BOOL has_focus, UINT state, const RECT *rc, RECT *border)
{
    RECT rcback;
    HBRUSH brush = NULL;
    cassert_no_null(border);

    {
        HWND parent = GetParent(hwnd);
        if (parent != NULL) 
            brush = (HBRUSH)SendMessage(parent, WM_CTLCOLORBTN, (WPARAM)hdc, (LPARAM)hwnd);
        else
            brush = (HBRUSH)GetStockObject(GRAY_BRUSH);
    }

    FillRect(hdc, rc, brush);

    rcback = *border;
    if (has_focus & ((state & DFCS_PUSHED) == 0))
        InflateRect(&rcback, -1, -1);

    DrawFrameControl(hdc, &rcback, DFC_BUTTON, state);
    InflateRect(border, -4, -4);
}

/*---------------------------------------------------------------------------*/

void osstyleXP_DrawThemeEdge(HDC hdc, int iPartId, int iStateId, const RECT *rc)
{
    HRESULT res = 0;
    cassert_no_null(i_STYLEXP.theme);
    res = DrawThemeEdge(i_STYLEXP.theme, hdc, iPartId, iStateId, rc, EDGE_BUMP, BF_TOP | BF_LEFT | BF_RIGHT | BF_BOTTOM, NULL);
    cassert_unref(res == S_OK, res);
}

/*---------------------------------------------------------------------------*/

BOOL osstyleXP_HasThemeFont(HDC hdc, int iPartId, int iStateId, int iPropId)
{
    LOGFONTW font;
    HRESULT res = 0;
    cassert_no_null(i_STYLEXP.theme);
    res = GetThemeFont(i_STYLEXP.theme, hdc, iPartId, iStateId, iPropId, &font);
    return (BOOL)(res == S_OK);
}

/*---------------------------------------------------------------------------*/

void osstyleXP_DrawThemeText2(HTHEME theme, HDC hdc, int iPartId, int iStateId, const WCHAR *text, int num_chars, DWORD flags, const RECT *rc)
{
    HRESULT res = 0;
    cassert_no_null(theme);
    res = DrawThemeText(theme, hdc, iPartId, iStateId, text, (int)num_chars, flags, 0, rc);
    cassert_unref(res == S_OK, res);
}

/*---------------------------------------------------------------------------*/

void osstyleXP_DrawThemeText(HDC hdc, int iPartId, int iStateId, const WCHAR *text, uint32_t num_chars, DWORD flags, const RECT *rc)
{
    HRESULT res = 0;
    cassert_no_null(i_STYLEXP.theme);
    res = DrawThemeText(i_STYLEXP.theme, hdc, iPartId, iStateId, text, (int)num_chars, flags, 0, rc);
    cassert_unref(res == S_OK, res);
}

/*---------------------------------------------------------------------------*/

//BOOL osstyleXP_IsThemeBackgroundPartiallyTransparent(OSStyleXP *style, HTHEME hTheme, int iPartId, int iStateId);
//BOOL osstyleXP_IsThemeBackgroundPartiallyTransparent(OSStyleXP *style, HTHEME hTheme, int iPartId, int iStateId)
//{
//    unref(style);
//    return IsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId);
//}

/*---------------------------------------------------------------------------*/

//HRESULT osstyleXP_DrawThemeParentBackground(OSStyleXP *style, HWND hwnd, HDC hdc, RECT* pRect);
//HRESULT osstyleXP_DrawThemeParentBackground(OSStyleXP *style, HWND hwnd, HDC hdc, RECT* pRect)
//{
//    unref(style);
//    return DrawThemeParentBackground(hwnd, hdc, pRect);
//}

/*---------------------------------------------------------------------------*/

/*
HRESULT osstyleXP_DrawThemeBackground(OSStyleXP *style, HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
HRESULT osstyleXP_DrawThemeBackground(OSStyleXP *style, HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
{
    unref(style);
    return DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}*/

/*---------------------------------------------------------------------------*/



//CVisualStylesXP g_xpStyle;
//
//CVisualStylesXP::CVisualStylesXP(void)
//{
//  m_hThemeDll = LoadLibrary(L"UxTheme.dll");
//}
//
//CVisualStylesXP::~CVisualStylesXP(void)
//{
//  if (m_hThemeDll!=NULL)
//    FreeLibrary(m_hThemeDll);
//  m_hThemeDll = NULL;
//}
//
//void* CVisualStylesXP::GetProc(LPCSTR szProc, void* pfnFail)
//{
//  void* pRet = pfnFail;
//  if (m_hThemeDll != NULL)
//    pRet = GetProcAddress(m_hThemeDll, szProc);
//  return pRet;
//}
//
//HTHEME CVisualStylesXP::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
//{
//  if (m_hThemeDll==NULL) return OpenThemeDataFail(hwnd,pszClassList);
//  PFNOPENTHEMEDATA pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProc("OpenThemeData", (void*)OpenThemeDataFail);
//  return (*pfnOpenThemeData)(hwnd, pszClassList);
//}
//
//
//HRESULT CVisualStylesXP::DrawThemeBackground(HTHEME hTheme, HDC hdc, 
//                       int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
//{
//  PFNDRAWTHEMEBACKGROUND pfnDrawThemeBackground = 
//    (PFNDRAWTHEMEBACKGROUND)GetProc("DrawThemeBackground", (void*)DrawThemeBackgroundFail);
//  return (*pfnDrawThemeBackground)(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
//}
//
//
//HRESULT CVisualStylesXP::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
//                     int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
//                     DWORD dwTextFlags2, const RECT *pRect)
//{
//  PFNDRAWTHEMETEXT pfn = (PFNDRAWTHEMETEXT)GetProc("DrawThemeText", (void*)DrawThemeTextFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
//}
//HRESULT CVisualStylesXP::GetThemeBackgroundContentRect(HTHEME hTheme,  HDC hdc, 
//                             int iPartId, int iStateId,  const RECT *pBoundingRect, 
//                             RECT *pContentRect)
//{
//  PFNGETTHEMEBACKGROUNDCONTENTRECT pfn = (PFNGETTHEMEBACKGROUNDCONTENTRECT)GetProc("GetThemeBackgroundContentRect", (void*)GetThemeBackgroundContentRectFail);
//  return (*pfn)(hTheme,  hdc, iPartId, iStateId,  pBoundingRect, pContentRect);
//}
//HRESULT CVisualStylesXP::GetThemeBackgroundExtent(HTHEME hTheme,  HDC hdc,
//                          int iPartId, int iStateId, const RECT *pContentRect, 
//                          RECT *pExtentRect)
//{
//  PFNGETTHEMEBACKGROUNDEXTENT pfn = (PFNGETTHEMEBACKGROUNDEXTENT)GetProc("GetThemeBackgroundExtent", (void*)GetThemeBackgroundExtentFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, pContentRect, pExtentRect);
//}
//HRESULT CVisualStylesXP::GetThemePartSize(HTHEME hTheme, HDC hdc, 
//                      int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize, SIZE *psz)
//{
//  PFNGETTHEMEPARTSIZE pfnGetThemePartSize = 
//    (PFNGETTHEMEPARTSIZE)GetProc("GetThemePartSize", (void*)GetThemePartSizeFail);
//  return (*pfnGetThemePartSize)(hTheme, hdc, iPartId, iStateId, pRect, eSize, psz);
//}
//
//HRESULT CVisualStylesXP::GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
//                      int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
//                      DWORD dwTextFlags,  const RECT *pBoundingRect, 
//                      RECT *pExtentRect)
//{
//  PFNGETTHEMETEXTEXTENT pfn = (PFNGETTHEMETEXTEXTENT)GetProc("GetThemeTextExtent", (void*)GetThemeTextExtentFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags,  pBoundingRect, pExtentRect);
//}
//
//HRESULT CVisualStylesXP::GetThemeTextMetrics(HTHEME hTheme,  HDC hdc, 
//                       int iPartId, int iStateId,  TEXTMETRIC* ptm)
//{
//  PFNGETTHEMETEXTMETRICS pfn = (PFNGETTHEMETEXTMETRICS)GetProc("GetThemeTextMetrics", (void*)GetThemeTextMetricsFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId,  ptm);
//}
//
//HRESULT CVisualStylesXP::GetThemeBackgroundRegion(HTHEME hTheme,  HDC hdc,  
//                          int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion)
//{
//  PFNGETTHEMEBACKGROUNDREGION pfn = (PFNGETTHEMEBACKGROUNDREGION)GetProc("GetThemeBackgroundRegion", (void*)GetThemeBackgroundRegionFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, pRect, pRegion);
//}
//
//HRESULT CVisualStylesXP::HitTestThemeBackground(HTHEME hTheme,  HDC hdc, int iPartId, 
//                        int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
//                        POINT ptTest,  WORD *pwHitTestCode)
//{
//  PFNHITTESTTHEMEBACKGROUND pfn = (PFNHITTESTTHEMEBACKGROUND)GetProc("HitTestThemeBackground", (void*)HitTestThemeBackgroundFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, dwOptions, pRect, hrgn, ptTest, pwHitTestCode);
//}
//
//HRESULT CVisualStylesXP::DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
//                     const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect)
//{
//  PFNDRAWTHEMEEDGE pfn = (PFNDRAWTHEMEEDGE)GetProc("DrawThemeEdge", (void*)DrawThemeEdgeFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, pDestRect, uEdge, uFlags, pContentRect);
//}
//
//HRESULT CVisualStylesXP::DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
//                     int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex)
//{
//  PFNDRAWTHEMEICON pfn = (PFNDRAWTHEMEICON)GetProc("DrawThemeIcon", (void*)DrawThemeIconFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, pRect, himl, iImageIndex);
//}
//
//BOOL CVisualStylesXP::IsThemePartDefined(HTHEME hTheme, int iPartId, 
//                     int iStateId)
//{
//  PFNISTHEMEPARTDEFINED pfn = (PFNISTHEMEPARTDEFINED)GetProc("IsThemePartDefined", (void*)IsThemePartDefinedFail);
//  return (*pfn)(hTheme, iPartId, iStateId);
//}
//
//BOOL CVisualStylesXP::IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, 
//                              int iPartId, int iStateId)
//{
//  PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT pfn = (PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)GetProc("IsThemeBackgroundPartiallyTransparent", (void*)IsThemeBackgroundPartiallyTransparentFail);
//  return (*pfn)(hTheme, iPartId, iStateId);
//}
//
//HRESULT CVisualStylesXP::GetThemeColor(HTHEME hTheme, int iPartId, 
//                     int iStateId, int iPropId,  COLORREF *pColor)
//{
//  PFNGETTHEMECOLOR pfn = (PFNGETTHEMECOLOR)GetProc("GetThemeColor", (void*)GetThemeColorFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, pColor);
//}
//
//HRESULT CVisualStylesXP::GetThemeMetric(HTHEME hTheme,  HDC hdc, int iPartId, 
//                    int iStateId, int iPropId,  int *piVal)
//{
//  PFNGETTHEMEMETRIC pfn = (PFNGETTHEMEMETRIC)GetProc("GetThemeMetric", (void*)GetThemeMetricFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, iPropId, piVal);
//}
//
//HRESULT CVisualStylesXP::GetThemeString(HTHEME hTheme, int iPartId, 
//                    int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars)
//{
//  PFNGETTHEMESTRING pfn = (PFNGETTHEMESTRING)GetProc("GetThemeString", (void*)GetThemeStringFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, pszBuff, cchMaxBuffChars);
//}
//
//HRESULT CVisualStylesXP::GetThemeBool(HTHEME hTheme, int iPartId, 
//                    int iStateId, int iPropId,  BOOL *pfVal)
//{
//  PFNGETTHEMEBOOL pfn = (PFNGETTHEMEBOOL)GetProc("GetThemeBool", (void*)GetThemeBoolFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, pfVal);
//}
//
//HRESULT CVisualStylesXP::GetThemeInt(HTHEME hTheme, int iPartId, 
//                   int iStateId, int iPropId,  int *piVal)
//{
//  PFNGETTHEMEINT pfn = (PFNGETTHEMEINT)GetProc("GetThemeInt", (void*)GetThemeIntFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, piVal);
//}
//
//HRESULT CVisualStylesXP::GetThemeEnumValue(HTHEME hTheme, int iPartId, 
//                       int iStateId, int iPropId,  int *piVal)
//{
//  PFNGETTHEMEENUMVALUE pfn = (PFNGETTHEMEENUMVALUE)GetProc("GetThemeEnumValue", (void*)GetThemeEnumValueFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, piVal);
//}
//
//HRESULT CVisualStylesXP::GetThemePosition(HTHEME hTheme, int iPartId, 
//                      int iStateId, int iPropId,  POINT *pPoint)
//{
//  PFNGETTHEMEPOSITION pfn = (PFNGETTHEMEPOSITION)GetProc("GetThemePosition", (void*)GetThemePositionFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, pPoint);
//}
//
//HRESULT CVisualStylesXP::GetThemeFont(HTHEME hTheme,  HDC hdc, int iPartId, 
//                    int iStateId, int iPropId,  LOGFONT *pFont)
//{
//  PFNGETTHEMEFONT pfn = (PFNGETTHEMEFONT)GetProc("GetThemeFont", (void*)GetThemeFontFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, iPropId, pFont);
//}
//
//HRESULT CVisualStylesXP::GetThemeRect(HTHEME hTheme, int iPartId, 
//                    int iStateId, int iPropId,  RECT *pRect)
//{
//  PFNGETTHEMERECT pfn = (PFNGETTHEMERECT)GetProc("GetThemeRect", (void*)GetThemeRectFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, pRect);
//}
//
//HRESULT CVisualStylesXP::GetThemeMargins(HTHEME hTheme,  HDC hdc, int iPartId, 
//                     int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins)
//{
//  PFNGETTHEMEMARGINS pfn = (PFNGETTHEMEMARGINS)GetProc("GetThemeMargins", (void*)GetThemeMarginsFail);
//  return (*pfn)(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
//}
//
//HRESULT CVisualStylesXP::GetThemeIntList(HTHEME hTheme, int iPartId, 
//                     int iStateId, int iPropId,  INTLIST *pIntList)
//{
//  PFNGETTHEMEINTLIST pfn = (PFNGETTHEMEINTLIST)GetProc("GetThemeIntList", (void*)GetThemeIntListFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, pIntList);
//}
//
//HRESULT CVisualStylesXP::GetThemePropertyOrigin(HTHEME hTheme, int iPartId, 
//                        int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin)
//{
//  PFNGETTHEMEPROPERTYORIGIN pfn = (PFNGETTHEMEPROPERTYORIGIN)GetProc("GetThemePropertyOrigin", (void*)GetThemePropertyOriginFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId, pOrigin);
//}
//
//HRESULT CVisualStylesXP::SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
//                    LPCWSTR pszSubIdList)
//{
//  PFNSETWINDOWTHEME pfn = (PFNSETWINDOWTHEME)GetProc("SetWindowTheme", (void*)SetWindowThemeFail);
//  return (*pfn)(hwnd, pszSubAppName, pszSubIdList);
//}
//
//HRESULT CVisualStylesXP::GetThemeFilename(HTHEME hTheme, int iPartId, 
//                      int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars)
//{
//  PFNGETTHEMEFILENAME pfn = (PFNGETTHEMEFILENAME)GetProc("GetThemeFilename", (void*)GetThemeFilenameFail);
//  return (*pfn)(hTheme, iPartId, iStateId, iPropId,  pszThemeFileName, cchMaxBuffChars);
//}
//
//COLORREF CVisualStylesXP::GetThemeSysColor(HTHEME hTheme, int iColorId)
//{
//  PFNGETTHEMESYSCOLOR pfn = (PFNGETTHEMESYSCOLOR)GetProc("GetThemeSysColor", (void*)GetThemeSysColorFail);
//  return (*pfn)(hTheme, iColorId);
//}
//
//HBRUSH CVisualStylesXP::GetThemeSysColorBrush(HTHEME hTheme, int iColorId)
//{
//  PFNGETTHEMESYSCOLORBRUSH pfn = (PFNGETTHEMESYSCOLORBRUSH)GetProc("GetThemeSysColorBrush", (void*)GetThemeSysColorBrushFail);
//  return (*pfn)(hTheme, iColorId);
//}
//
//BOOL CVisualStylesXP::GetThemeSysBool(HTHEME hTheme, int iBoolId)
//{
//  PFNGETTHEMESYSBOOL pfn = (PFNGETTHEMESYSBOOL)GetProc("GetThemeSysBool", (void*)GetThemeSysBoolFail);
//  return (*pfn)(hTheme, iBoolId);
//}
//
//int CVisualStylesXP::GetThemeSysSize(HTHEME hTheme, int iSizeId)
//{
//  PFNGETTHEMESYSSIZE pfn = (PFNGETTHEMESYSSIZE)GetProc("GetThemeSysSize", (void*)GetThemeSysSizeFail);
//  return (*pfn)(hTheme, iSizeId);
//}
//
//HRESULT CVisualStylesXP::GetThemeSysFont(HTHEME hTheme, int iFontId,  LOGFONT *plf)
//{
//  PFNGETTHEMESYSFONT pfn = (PFNGETTHEMESYSFONT)GetProc("GetThemeSysFont", (void*)GetThemeSysFontFail);
//  return (*pfn)(hTheme, iFontId, plf);
//}
//
//HRESULT CVisualStylesXP::GetThemeSysString(HTHEME hTheme, int iStringId, 
//                       LPWSTR pszStringBuff, int cchMaxStringChars)
//{
//  PFNGETTHEMESYSSTRING pfn = (PFNGETTHEMESYSSTRING)GetProc("GetThemeSysString", (void*)GetThemeSysStringFail);
//  return (*pfn)(hTheme, iStringId, pszStringBuff, cchMaxStringChars);
//}
//
//HRESULT CVisualStylesXP::GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue)
//{
//  PFNGETTHEMESYSINT pfn = (PFNGETTHEMESYSINT)GetProc("GetThemeSysInt", (void*)GetThemeSysIntFail);
//  return (*pfn)(hTheme, iIntId, piValue);
//}
//
//BOOL CVisualStylesXP::IsThemeActive()
//{
//  if (m_hThemeDll==NULL) return IsThemeActiveFail();
//  PFNISTHEMEACTIVE pfn = (PFNISTHEMEACTIVE)GetProc("IsThemeActive", (void*)IsThemeActiveFail);
//  return (*pfn)();
//}
//
//BOOL CVisualStylesXP::IsAppThemed()
//{
//  if (m_hThemeDll==NULL) return IsAppThemedFail();
//  PFNISAPPTHEMED pfnIsAppThemed = (PFNISAPPTHEMED)GetProc("IsAppThemed", (void*)IsAppThemedFail);
//  return (*pfnIsAppThemed)();
//}
//
//HTHEME CVisualStylesXP::GetWindowTheme(HWND hwnd)
//{
//  PFNGETWINDOWTHEME pfn = (PFNGETWINDOWTHEME)GetProc("GetWindowTheme", (void*)GetWindowThemeFail);
//  return (*pfn)(hwnd);
//}
//
//HRESULT CVisualStylesXP::EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags)
//{
//  PFNENABLETHEMEDIALOGTEXTURE pfn = (PFNENABLETHEMEDIALOGTEXTURE)GetProc("EnableThemeDialogTexture", (void*)EnableThemeDialogTextureFail);
//  return (*pfn)(hwnd, dwFlags);
//}
//
//BOOL CVisualStylesXP::IsThemeDialogTextureEnabled(HWND hwnd)
//{
//  PFNISTHEMEDIALOGTEXTUREENABLED pfn = (PFNISTHEMEDIALOGTEXTUREENABLED)GetProc("IsThemeDialogTextureEnabled", (void*)IsThemeDialogTextureEnabledFail);
//  return (*pfn)(hwnd);
//}
//
//DWORD CVisualStylesXP::GetThemeAppProperties()
//{
//  PFNGETTHEMEAPPPROPERTIES pfn = (PFNGETTHEMEAPPPROPERTIES)GetProc("GetThemeAppProperties", (void*)GetThemeAppPropertiesFail);
//  return (*pfn)();
//}
//
//void CVisualStylesXP::SetThemeAppProperties(DWORD dwFlags)
//{
//  PFNSETTHEMEAPPPROPERTIES pfn = (PFNSETTHEMEAPPPROPERTIES)GetProc("SetThemeAppProperties", (void*)SetThemeAppPropertiesFail);
//  (*pfn)(dwFlags);
//}
//
//HRESULT CVisualStylesXP::GetCurrentThemeName(
//  LPWSTR pszThemeFileName, int cchMaxNameChars, 
//  LPWSTR pszColorBuff, int cchMaxColorChars,
//  LPWSTR pszSizeBuff, int cchMaxSizeChars)
//{
//  PFNGETCURRENTTHEMENAME pfn = (PFNGETCURRENTTHEMENAME)GetProc("GetCurrentThemeName", (void*)GetCurrentThemeNameFail);
//  return (*pfn)(pszThemeFileName, cchMaxNameChars, pszColorBuff, cchMaxColorChars, pszSizeBuff, cchMaxSizeChars);
//}
//
//HRESULT CVisualStylesXP::GetThemeDocumentationProperty(LPCWSTR pszThemeName,
//                             LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars)
//{
//  PFNGETTHEMEDOCUMENTATIONPROPERTY pfn = (PFNGETTHEMEDOCUMENTATIONPROPERTY)GetProc("GetThemeDocumentationProperty", (void*)GetThemeDocumentationPropertyFail);
//  return (*pfn)(pszThemeName, pszPropertyName, pszValueBuff, cchMaxValChars);
//}
//
//
//HRESULT CVisualStylesXP::DrawThemeParentBackground(HWND hwnd, HDC hdc,  RECT* prc)
//{
//  PFNDRAWTHEMEPARENTBACKGROUND pfn = (PFNDRAWTHEMEPARENTBACKGROUND)GetProc("DrawThemeParentBackground", (void*)DrawThemeParentBackgroundFail);
//  return (*pfn)(hwnd, hdc, prc);
//}
//
//HRESULT CVisualStylesXP::EnableTheming(BOOL fEnable)
//{
//  PFNENABLETHEMING pfn = (PFNENABLETHEMING)GetProc("EnableTheming", (void*)EnableThemingFail);
//  return (*pfn)(fEnable);
//}
