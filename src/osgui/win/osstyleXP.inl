/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osstyleXP.inl
 *
 */

/* Windows XP Styles */

#include "win/draw2d_gdi.ixx"

#include "nowarn.hxx"
#include <uxtheme.h>
#if _MSC_VER > 1400
#include <vssym32.h>
#include <vsstyle.h>
#else
#include <tmschema.h>
#endif
#include "warn.hxx"

__EXTERN_C

void osstyleXP_init(void);

void osstyleXP_remove(void);

HTHEME osstyleXP_OpenTheme(HWND hwnd, LPCWSTR pszClassList);

BOOL osstyleXP_OpenThemeData(HWND hwnd, LPCWSTR pszClassList);

void osstyleXP_CloseTheme(HTHEME theme);

void osstyleXP_CloseThemeData(void);

void osstyleXP_GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect);

void osstyleXP_DrawThemeBackgroundNoBorder(HTHEME theme, int iPartId, int iStateId, HDC hdc, const RECT *rc);

void osstyleXP_DrawThemeBackground2(HTHEME theme, int iPartId, int iStateId, HDC hdc, const RECT *rc);

void osstyleXP_DrawThemeBackground(HWND hwnd, HDC hdc, int iPartId, int iStateId, const BOOL draw_parent_bg, const RECT *rc, RECT *border);

void osstyleXP_DrawNonThemedButtonBackground(HWND hwnd, HDC hdc, BOOL has_focus, UINT state, const RECT *rc, RECT *border);

void osstyleXP_DrawThemeEdge(HDC hdc, int iPartId, int iStateId, const RECT *rc);

BOOL osstyleXP_HasThemeFont(HDC hdc, int iPartId, int iStateId, int iPropId);

void osstyleXP_DrawThemeText2(HTHEME theme, HDC hdc, int iPartId, int iStateId, const WCHAR *text, int num_chars, DWORD flags, const RECT *rc);

void osstyleXP_DrawThemeText(HDC hdc, int iPartId, int iStateId, const WCHAR *text, uint32_t num_chars, DWORD flags, const RECT *rc);

__END_C

/*

class CVisualStylesXP
{
private:
  HMODULE m_hThemeDll;
  void* GetProc(LPCSTR szProc, void* pfnFail);

  typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
  static HTHEME OpenThemeDataFail(HWND , LPCWSTR )
  {return NULL;}

  typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
  static HRESULT CloseThemeDataFail(HTHEME)
  {return E_FAIL;}

  typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect,  const RECT *pClipRect);
  static HRESULT DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT *, const RECT *)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    DWORD dwTextFlags2, const RECT *pRect);
  static HRESULT DrawThemeTextFail(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDCONTENTRECT)(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  const RECT *pBoundingRect, 
    RECT *pContentRect);
  static HRESULT GetThemeBackgroundContentRectFail(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  const RECT *pBoundingRect, 
    RECT *pContentRect)
  {return E_FAIL;}
  typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDEXTENT)(HTHEME hTheme,  HDC hdc,
    int iPartId, int iStateId, const RECT *pContentRect, 
    RECT *pExtentRect);
  static HRESULT GetThemeBackgroundExtentFail(HTHEME hTheme,  HDC hdc,
    int iPartId, int iStateId, const RECT *pContentRect, 
    RECT *pExtentRect)
  {return E_FAIL;}

  typedef HRESULT(__stdcall *PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize,  SIZE *psz);
  static HRESULT GetThemePartSizeFail(HTHEME, HDC, int, int, RECT *, enum THEMESIZE, SIZE *)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMETEXTEXTENT)(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
    DWORD dwTextFlags,  const RECT *pBoundingRect, 
    RECT *pExtentRect);
  static HRESULT GetThemeTextExtentFail(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
    DWORD dwTextFlags,  const RECT *pBoundingRect, 
    RECT *pExtentRect)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMETEXTMETRICS)(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  TEXTMETRIC* ptm);
  static HRESULT GetThemeTextMetricsFail(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  TEXTMETRIC* ptm)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDREGION)(HTHEME hTheme,  HDC hdc,  
    int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion);
  static HRESULT GetThemeBackgroundRegionFail(HTHEME hTheme,  HDC hdc,  
    int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNHITTESTTHEMEBACKGROUND)(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
    POINT ptTest,  WORD *pwHitTestCode);
  static HRESULT HitTestThemeBackgroundFail(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
    POINT ptTest,  WORD *pwHitTestCode)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNDRAWTHEMEEDGE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect);
  static HRESULT DrawThemeEdgeFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNDRAWTHEMEICON)(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
  static HRESULT DrawThemeIconFail(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex)
  {return E_FAIL;}

  typedef BOOL (__stdcall *PFNISTHEMEPARTDEFINED)(HTHEME hTheme, int iPartId, 
    int iStateId);
  static BOOL IsThemePartDefinedFail(HTHEME hTheme, int iPartId, 
    int iStateId)
  {return FALSE;}

  typedef BOOL (__stdcall *PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)(HTHEME hTheme, 
    int iPartId, int iStateId);
  static BOOL IsThemeBackgroundPartiallyTransparentFail(HTHEME hTheme, 
    int iPartId, int iStateId)
  {return FALSE;}

  typedef HRESULT (__stdcall *PFNGETTHEMECOLOR)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  COLORREF *pColor);
  static HRESULT GetThemeColorFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  COLORREF *pColor)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEMETRIC)(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  static HRESULT GetThemeMetricFail(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  int *piVal)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMESTRING)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars);
  static HRESULT GetThemeStringFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEBOOL)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  BOOL *pfVal);
  static HRESULT GetThemeBoolFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  BOOL *pfVal)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEINT)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  static HRESULT GetThemeIntFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEENUMVALUE)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  static HRESULT GetThemeEnumValueFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEPOSITION)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  POINT *pPoint);
  static HRESULT GetThemePositionFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  POINT *pPoint)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEFONT)(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  LOGFONT *pFont);
  static HRESULT GetThemeFontFail(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  LOGFONT *pFont)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMERECT)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  RECT *pRect);
  static HRESULT GetThemeRectFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  RECT *pRect)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEMARGINS)(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins);
  static HRESULT GetThemeMarginsFail(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEINTLIST)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  INTLIST *pIntList);
  static HRESULT GetThemeIntListFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  INTLIST *pIntList)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEPROPERTYORIGIN)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin);
  static HRESULT GetThemePropertyOriginFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, 
    LPCWSTR pszSubIdList);
  static HRESULT SetWindowThemeFail(HWND hwnd, LPCWSTR pszSubAppName, 
    LPCWSTR pszSubIdList)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEFILENAME)(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars);
  static HRESULT GetThemeFilenameFail(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars)
  {return E_FAIL;}

  typedef COLORREF (__stdcall *PFNGETTHEMESYSCOLOR)(HTHEME hTheme, int iColorId);
  static COLORREF GetThemeSysColorFail(HTHEME hTheme, int iColorId)
  {return RGB(255,255,255);}

  typedef HBRUSH (__stdcall *PFNGETTHEMESYSCOLORBRUSH)(HTHEME hTheme, int iColorId);
  static HBRUSH GetThemeSysColorBrushFail(HTHEME hTheme, int iColorId)
  {return NULL;}

  typedef BOOL (__stdcall *PFNGETTHEMESYSBOOL)(HTHEME hTheme, int iBoolId);
  static BOOL GetThemeSysBoolFail(HTHEME hTheme, int iBoolId)
  {return FALSE;}

  typedef int (__stdcall *PFNGETTHEMESYSSIZE)(HTHEME hTheme, int iSizeId);
  static int GetThemeSysSizeFail(HTHEME hTheme, int iSizeId)
  {return 0;}

  typedef HRESULT (__stdcall *PFNGETTHEMESYSFONT)(HTHEME hTheme, int iFontId,  LOGFONT *plf);
  static HRESULT GetThemeSysFontFail(HTHEME hTheme, int iFontId,  LOGFONT *plf)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMESYSSTRING)(HTHEME hTheme, int iStringId, 
    LPWSTR pszStringBuff, int cchMaxStringChars);
  static HRESULT GetThemeSysStringFail(HTHEME hTheme, int iStringId, 
    LPWSTR pszStringBuff, int cchMaxStringChars)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMESYSINT)(HTHEME hTheme, int iIntId, int *piValue);
  static HRESULT GetThemeSysIntFail(HTHEME hTheme, int iIntId, int *piValue)
  {return E_FAIL;}

  typedef BOOL (__stdcall *PFNISTHEMEACTIVE)();
  static BOOL IsThemeActiveFail()
  {return FALSE;}

  typedef BOOL(__stdcall *PFNISAPPTHEMED)();
  static BOOL IsAppThemedFail()
  {return FALSE;}

  typedef HTHEME (__stdcall *PFNGETWINDOWTHEME)(HWND hwnd);
  static HTHEME GetWindowThemeFail(HWND hwnd)
  {return NULL;}

  typedef HRESULT (__stdcall *PFNENABLETHEMEDIALOGTEXTURE)(HWND hwnd, DWORD dwFlags);
  static HRESULT EnableThemeDialogTextureFail(HWND hwnd, DWORD dwFlags)
  {return E_FAIL;}

  typedef BOOL (__stdcall *PFNISTHEMEDIALOGTEXTUREENABLED)(HWND hwnd);
  static BOOL IsThemeDialogTextureEnabledFail(HWND hwnd)
  {return FALSE;}

  typedef DWORD (__stdcall *PFNGETTHEMEAPPPROPERTIES)();
  static DWORD GetThemeAppPropertiesFail()
  {return 0;}

  typedef void (__stdcall *PFNSETTHEMEAPPPROPERTIES)(DWORD dwFlags);
  static void SetThemeAppPropertiesFail(DWORD dwFlags)
  {return;}

  typedef HRESULT (__stdcall *PFNGETCURRENTTHEMENAME)(
    LPWSTR pszThemeFileName, int cchMaxNameChars, 
    LPWSTR pszColorBuff, int cchMaxColorChars,
    LPWSTR pszSizeBuff, int cchMaxSizeChars);
  static HRESULT GetCurrentThemeNameFail(
    LPWSTR pszThemeFileName, int cchMaxNameChars, 
    LPWSTR pszColorBuff, int cchMaxColorChars,
    LPWSTR pszSizeBuff, int cchMaxSizeChars)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNGETTHEMEDOCUMENTATIONPROPERTY)(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars);
  static HRESULT GetThemeDocumentationPropertyFail(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hwnd, HDC hdc,  RECT* prc);
  static HRESULT DrawThemeParentBackgroundFail(HWND hwnd, HDC hdc,  RECT* prc)
  {return E_FAIL;}

  typedef HRESULT (__stdcall *PFNENABLETHEMING)(BOOL fEnable);
  static HRESULT EnableThemingFail(BOOL fEnable)
  {return E_FAIL;}




public:
  HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
  HRESULT CloseThemeData(HTHEME hTheme);
  HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
  HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    DWORD dwTextFlags2, const RECT *pRect);
  HRESULT GetThemeBackgroundContentRect(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  const RECT *pBoundingRect, 
    RECT *pContentRect);
  HRESULT GetThemeBackgroundExtent(HTHEME hTheme,  HDC hdc,
    int iPartId, int iStateId, const RECT *pContentRect, 
    RECT *pExtentRect);
  HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize, SIZE *psz);
  HRESULT GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
    DWORD dwTextFlags,  const RECT *pBoundingRect, 
    RECT *pExtentRect);
  HRESULT GetThemeTextMetrics(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  TEXTMETRIC* ptm);
  HRESULT GetThemeBackgroundRegion(HTHEME hTheme,  HDC hdc,  
    int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion);
  HRESULT HitTestThemeBackground(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
    POINT ptTest,  WORD *pwHitTestCode);
  HRESULT DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect);
  HRESULT DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
  BOOL IsThemePartDefined(HTHEME hTheme, int iPartId, 
    int iStateId);
  BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, 
    int iPartId, int iStateId);
  HRESULT GetThemeColor(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  COLORREF *pColor);
  HRESULT GetThemeMetric(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  HRESULT GetThemeString(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars);
  HRESULT GetThemeBool(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  BOOL *pfVal);
  HRESULT GetThemeInt(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  HRESULT GetThemeEnumValue(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  HRESULT GetThemePosition(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  POINT *pPoint);
  HRESULT GetThemeFont(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  LOGFONT *pFont);
  HRESULT GetThemeRect(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  RECT *pRect);
  HRESULT GetThemeMargins(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins);
  HRESULT GetThemeIntList(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  INTLIST *pIntList);
  HRESULT GetThemePropertyOrigin(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin);
  HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
    LPCWSTR pszSubIdList);
  HRESULT GetThemeFilename(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars);
  COLORREF GetThemeSysColor(HTHEME hTheme, int iColorId);
  HBRUSH GetThemeSysColorBrush(HTHEME hTheme, int iColorId);
  BOOL GetThemeSysBool(HTHEME hTheme, int iBoolId);
  int GetThemeSysSize(HTHEME hTheme, int iSizeId);
  HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId,  LOGFONT *plf);
  HRESULT GetThemeSysString(HTHEME hTheme, int iStringId, 
    LPWSTR pszStringBuff, int cchMaxStringChars);
  HRESULT GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue);
  BOOL IsThemeActive();
  BOOL IsAppThemed();  
  HTHEME GetWindowTheme(HWND hwnd);
  HRESULT EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);
  BOOL IsThemeDialogTextureEnabled(HWND hwnd);
  DWORD GetThemeAppProperties();
  void SetThemeAppProperties(DWORD dwFlags);
  HRESULT GetCurrentThemeName(
    LPWSTR pszThemeFileName, int cchMaxNameChars, 
    LPWSTR pszColorBuff, int cchMaxColorChars,
    LPWSTR pszSizeBuff, int cchMaxSizeChars);
  HRESULT GetThemeDocumentationProperty(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars);
  HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc,  RECT* prc);
  HRESULT EnableTheming(BOOL fEnable);
public:
  CVisualStylesXP(void);
  ~CVisualStylesXP(void);
};

extern CVisualStylesXP g_xpStyle;

*/

/*

HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
  HRESULT CloseThemeData(HTHEME hTheme);
  HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
  HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    DWORD dwTextFlags2, const RECT *pRect);
  HRESULT GetThemeBackgroundContentRect(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  const RECT *pBoundingRect, 
    RECT *pContentRect);
  HRESULT GetThemeBackgroundExtent(HTHEME hTheme,  HDC hdc,
    int iPartId, int iStateId, const RECT *pContentRect, 
    RECT *pExtentRect);
  HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize, SIZE *psz);
  HRESULT GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
    DWORD dwTextFlags,  const RECT *pBoundingRect, 
    RECT *pExtentRect);
  HRESULT GetThemeTextMetrics(HTHEME hTheme,  HDC hdc, 
    int iPartId, int iStateId,  TEXTMETRIC* ptm);
  HRESULT GetThemeBackgroundRegion(HTHEME hTheme,  HDC hdc,  
    int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion);
  HRESULT HitTestThemeBackground(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
    POINT ptTest,  WORD *pwHitTestCode);
  HRESULT DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect);
  HRESULT DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
  BOOL IsThemePartDefined(HTHEME hTheme, int iPartId, 
    int iStateId);
  BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, 
    int iPartId, int iStateId);
  HRESULT GetThemeColor(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  COLORREF *pColor);
  HRESULT GetThemeMetric(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  HRESULT GetThemeString(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars);
  HRESULT GetThemeBool(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  BOOL *pfVal);
  HRESULT GetThemeInt(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  HRESULT GetThemeEnumValue(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  int *piVal);
  HRESULT GetThemePosition(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  POINT *pPoint);
  HRESULT GetThemeFont(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  LOGFONT *pFont);
  HRESULT GetThemeRect(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  RECT *pRect);
  HRESULT GetThemeMargins(HTHEME hTheme,  HDC hdc, int iPartId, 
    int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins);
  HRESULT GetThemeIntList(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  INTLIST *pIntList);
  HRESULT GetThemePropertyOrigin(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin);
  HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
    LPCWSTR pszSubIdList);
  HRESULT GetThemeFilename(HTHEME hTheme, int iPartId, 
    int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars);
  COLORREF GetThemeSysColor(HTHEME hTheme, int iColorId);
  HBRUSH GetThemeSysColorBrush(HTHEME hTheme, int iColorId);
  BOOL GetThemeSysBool(HTHEME hTheme, int iBoolId);
  int GetThemeSysSize(HTHEME hTheme, int iSizeId);
  HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId,  LOGFONT *plf);
  HRESULT GetThemeSysString(HTHEME hTheme, int iStringId, 
    LPWSTR pszStringBuff, int cchMaxStringChars);
  HRESULT GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue);
  BOOL IsThemeActive();
  BOOL IsAppThemed();  
  HTHEME GetWindowTheme(HWND hwnd);
  HRESULT EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);
  BOOL IsThemeDialogTextureEnabled(HWND hwnd);
  DWORD GetThemeAppProperties();
  void SetThemeAppProperties(DWORD dwFlags);
  HRESULT GetCurrentThemeName(
    LPWSTR pszThemeFileName, int cchMaxNameChars, 
    LPWSTR pszColorBuff, int cchMaxColorChars,
    LPWSTR pszSizeBuff, int cchMaxSizeChars);
  HRESULT GetThemeDocumentationProperty(LPCWSTR pszThemeName,
    LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars);
  HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc,  RECT* prc);
  HRESULT EnableTheming(BOOL fEnable);

*/
