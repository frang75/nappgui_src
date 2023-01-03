/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui_win.cpp
 *
 */

/* Operating system native gui */

#include "osgui.inl"
#include "osgui_win.inl"
#include "osmenu.inl"
#include "ospanel.inl"
#include "oswindow.inl"
#include "win/osstyleXP.inl"
#include "arrst.h"
#include "cassert.h"
#include "core.h"
#include "heap.h"
#include "unicode.h"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

#pragma comment (lib, "gdiplus.lib")
/* #pragma comment (lib, "Msimg32.lib") TransparentBlt (_osimage_scale_hbitmap) */
#pragma comment (lib, "Shlwapi.lib")
#pragma comment (lib, "comctl32.lib")
//#pragma comment (lib, "d3d9.lib")

/* Avoid Microsoft Warnings */
#include "nowarn.hxx" 
#include <Commctrl.h>
#include <Richedit.h>
#include <uxtheme.h>
#include <gdiplus.h>
#include "warn.hxx" 

/*---------------------------------------------------------------------------*/

const WORD kVIRTUAL_KEY[] = 
{ 
    UINT16_MAX,     /*ekKEY_UNASSIGNED      = 0*/
    'A',            /*ekKEY_A               = 1*/
    'S',            /*ekKEY_S               = 2*/
    'D',            /*ekKEY_D               = 3*/
    'F',            /*ekKEY_F               = 4*/
    'H',            /*ekKEY_H               = 5*/
    'G',            /*ekKEY_G               = 6*/
    'Z',            /*ekKEY_Z               = 7*/
    'X',            /*ekKEY_X               = 8*/
    'C',            /*ekKEY_C               = 9*/

    'V',            /*ekKEY_V               = 10*/
    VK_OEM_5,       /*ekKEY_BSLASH          = 11*/
    'B',            /*ekKEY_B               = 12*/
    'Q',            /*ekKEY_Q               = 13*/
    'W',            /*ekKEY_W               = 14*/
    'E',            /*ekKEY_E               = 15*/
    'R',            /*ekKEY_R               = 16*/
    'Y',            /*ekKEY_Y               = 17*/
    'T',            /*ekKEY_T               = 18*/
    '1',            /*ekKEY_1               = 19*/

    '2',            /*ekKEY_2               = 20*/
    '3',            /*ekKEY_3               = 21*/
    '4',            /*ekKEY_4               = 22*/
    '6',            /*ekKEY_6               = 23*/
    '5',            /*ekKEY_5               = 24*/
    '9',            /*ekKEY_9               = 25*/
    '7',            /*ekKEY_7               = 26*/
    '8',            /*ekKEY_8               = 27*/
    '0',            /*ekKEY_0               = 28*/
    VK_OEM_2,       /*ekKEY_RCURLY          = 29*/

    'O',            /*ekKEY_O               = 30*/
    'U',            /*ekKEY_U               = 31*/
    VK_OEM_7,       /*ekKEY_LCURLY          = 32*/
    'I',            /*ekKEY_I               = 33*/
    'P',            /*ekKEY_P               = 34*/
    VK_RETURN,      /*ekKEY_RETURN          = 35*/
    'L',            /*ekKEY_L               = 36*/
    'J',            /*ekKEY_J               = 37*/
    ';',            /*ekKEY_SEMICOLON       = 38*/
    'K',            /*ekKEY_K               = 39*/

    VK_OEM_4,       /*ekKEY_QUEST           = 40*/
    VK_OEM_COMMA,   /*ekKEY_COMMA           = 41*/
    VK_OEM_MINUS,   /*ekKEY_MINUS           = 42*/
    'N',            /*ekKEY_N               = 43*/
    'M',            /*ekKEY_M               = 44*/
    VK_OEM_PERIOD,  /*ekKEY_PERIOD          = 45*/
    VK_TAB,         /*ekKEY_TAB             = 46*/
    VK_SPACE,       /*ekKEY_SPACE           = 47*/
    VK_OEM_102,     /*ekKEY_GTLT            = 48*/
    VK_BACK,        /*ekKEY_BACK            = 49*/

    VK_ESCAPE,      /*ekKEY_ESCAPE          = 50*/
    VK_F17,         /*ekKEY_F17             = 51*/
    VK_DECIMAL,     /*ekKEY_NUMDECIMAL      = 52*/
    VK_MULTIPLY,    /*ekKEY_NUMMULT         = 53*/
    VK_ADD,         /*ekKEY_NUMADD          = 54*/
    VK_NUMLOCK,     /*ekKEY_NUMLOCK         = 55*/
    VK_DIVIDE,      /*ekKEY_NUMDIV          = 56*/
    VK_RETURN,      /*ekKEY_NUMRET          = 57*/
    VK_SUBTRACT,    /*ekKEY_NUMMINUS        = 58*/
    VK_F18,         /*ekKEY_F18             = 59*/

    VK_F19,         /*ekKEY_F19             = 60*/
    ' ',            /*ekKEY_NUMPAD_EQUAL    = 61*/
    VK_NUMPAD0,     /*ekKEY_NUM0            = 62*/
    VK_NUMPAD1,     /*ekKEY_NUM1            = 63*/
    VK_NUMPAD2,     /*ekKEY_NUM2            = 64*/
    VK_NUMPAD3,     /*ekKEY_NUM3            = 65*/
    VK_NUMPAD4,     /*ekKEY_NUM4            = 66*/
    VK_NUMPAD5,     /*ekKEY_NUM5            = 67*/
    VK_NUMPAD6,     /*ekKEY_NUM6            = 68*/
    VK_NUMPAD7,     /*ekKEY_NUM7            = 69*/

    VK_NUMPAD8,     /*ekKEY_NUM8            = 70*/
    VK_NUMPAD9,     /*ekKEY_NUM9            = 71*/
    VK_F5,          /*ekKEY_F5              = 72*/
    VK_F6,          /*ekKEY_F6              = 73*/
    VK_F7,          /*ekKEY_F7              = 74*/
    VK_F3,          /*ekKEY_F3              = 75*/
    VK_F8,          /*ekKEY_F8              = 76*/
    VK_F9,          /*ekKEY_F9              = 77*/
    VK_F11,         /*ekKEY_F11             = 78*/
    VK_F13,         /*ekKEY_F13             = 79*/

    VK_F16,         /*ekKEY_F16             = 80*/
    VK_F14,         /*ekKEY_F14             = 81*/
    VK_F10,         /*ekKEY_F10             = 82*/
    VK_F12,         /*ekKEY_F12             = 83*/
    VK_F15,         /*ekKEY_F15             = 84*/
    VK_PRIOR,       /*ekKEY_PAGEUP          = 85*/
    VK_HOME,        /*ekKEY_HOME            = 86*/
    VK_DELETE,      /*ekKEY_SUPR            = 87*/
    VK_F4,          /*ekKEY_F4              = 88*/
    VK_NEXT,        /*ekKEY_PAGEDOWN        = 89*/

    VK_F2,          /*ekKEY_F2              = 90*/
    VK_END,         /*ekKEY_END             = 91*/
    VK_F1,          /*ekKEY_F1              = 92*/
    VK_LEFT,        /*ekKEY_LEFT            = 93*/
    VK_RIGHT,       /*ekKEY_RIGHT           = 94*/
    VK_DOWN,        /*ekKEY_DOWN            = 95*/
    VK_UP,          /*ekKEY_UP              = 96*/
    VK_LSHIFT,      /*ekKEY_LSHIFT          = 97*/
    VK_RSHIFT,      /*ekKEY_RSHIFT          = 98*/
    VK_LCONTROL,    /*ekKEY_LCTRL           = 99 */
    VK_RCONTROL,    /*ekKEY_RCTRL           = 100 */
    VK_LMENU,       /*ekKEY_LALT            = 101 */
    VK_RMENU,       /*ekKEY_RALT            = 102 */
    VK_INSERT,      /*ekKEY_INSERT          = 103 */
    VK_OEM_6,       /*ekKEY_EXCLAM          = 104 */
    VK_APPS,        /*ekKEY_MENU            = 105 */
    VK_LWIN,        /*ekKEY_LWIN            = 106 */
    VK_RWIN,        /*ekKEY_RWIN            = 107 */
    VK_CAPITAL,     /*ekKEY_CAPS            = 108 */
    VK_OEM_3,       /*ekKEY_TILDE           = 109 */
    VK_OEM_1,       /*ekKEY_GRAVE           = 110 */
    VK_OEM_PLUS     /*ekKEY_PLUS            = 111 */
};

uint32_t kNUM_VKEYS = sizeof(kVIRTUAL_KEY) / sizeof(WORD);
DeclSt(ACCEL);

/*---------------------------------------------------------------------------*/

static HINSTANCE i_INSTANCE = NULL;
static ULONG_PTR i_GDIPLUSTOKEN = 0L;
static OSPanel *i_DEFAULT_OSPANEL = NULL;
HWND kDEFAULT_PARENT_WINDOW = NULL;
HCURSOR kNORMAL_ARROW_CURSOR = NULL;
HCURSOR kSIZING_HORIZONTAL_CURSOR = NULL;
HCURSOR kSIZING_VERTICAL_CURSOR = NULL;
HBRUSH kCHESSBOARD_BRUSH = NULL;
const TCHAR *kWINDOW_CLASS = L"com.nappgui.window";
const TCHAR *kVIEW_CLASS = L"com.nappgui.view";
const TCHAR *kRICHEDIT_CLASS = NULL;
unicode_t kWINDOWS_UNICODE = ENUM_MAX(unicode_t);
int kLOG_PIXY_GUI = 0;
LONG kTWIPS_PER_PIXEL_GUI = 0;
static ArrSt(ACCEL) *kACCELERATORS = NULL;
static HACCEL kACCEL_TABLE = NULL;

/*---------------------------------------------------------------------------*/

static void i_registry_custom_window_class(void)
{
    WNDCLASSEX wc;
    cassert(i_INSTANCE != NULL);
	wc.cbSize		 = sizeof(WNDCLASSEX);
    wc.style		 = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;    
	wc.lpfnWndProc	 = DefWindowProc;
    wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
    wc.hInstance	 = i_INSTANCE;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);

    // Avoid warning C4306: 'type cast' : conversion from 'int' to 'HBRUSH' of greater size
    #if defined (__x64__)
	wc.hbrBackground = (HBRUSH)(uint64_t)(COLOR_BTNFACE);
    #else
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    #endif 

	wc.lpszMenuName  = NULL;
	wc.lpszClassName = kWINDOW_CLASS;
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

    {
        ATOM ret = 0;
        ret = RegisterClassEx(&wc);
        cassert(ret != 0);
    }
}

/*---------------------------------------------------------------------------*/

static void i_registry_view_class(void)
{
    WNDCLASSEX wc;
    cassert(i_INSTANCE != NULL);
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_GLOBALCLASS;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(char*) * 2;
    wc.hInstance = i_INSTANCE;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = kVIEW_CLASS;
    wc.hIconSm = LoadCursor(NULL, IDC_ARROW);

    {
        ATOM ret = RegisterClassEx(&wc);
        cassert_unref(ret != 0, ret);
    }
}

/*---------------------------------------------------------------------------*/

// https://www.codeproject.com/Articles/13723/Themed-RichTextBox-A-RichTextBox-with-XP-styled-bo
LRESULT _osgui_nccalcsize(HWND hwnd, WPARAM wParam, LPARAM lParam, bool_t expand, RECT *border)
{
    LRESULT res = 0;
    RECT *window_rect = NULL;
    RECT content_rect;
    HDC hdc = GetWindowDC(hwnd);
    HTHEME theme = osstyleXP_OpenTheme(hwnd, L"EDIT");

    cassert_no_null(border);

    // LParam points to a NCCALCSIZE_PARAMS struct
	if(wParam == TRUE) 
	{
        NCCALCSIZE_PARAMS *cp = (NCCALCSIZE_PARAMS*)lParam;
        window_rect = &cp->rgrc[0];
	}
    // LParam points to a RECT struct
	else 
	{
        window_rect = (RECT*)lParam;
	}

    osstyleXP_GetThemeBackgroundContentRect(theme, hdc, EP_EDITTEXT, ETS_NORMAL, window_rect, &content_rect);

    // shrink the client area the make more space for containing text.
    if (expand == TRUE)
        InflateRect(&content_rect, -1, -1);

    // remember the space of the borders
    border->left = content_rect.left - window_rect->left;
    border->top = content_rect.top - window_rect->top;
    border->right = window_rect->right - content_rect.right;
    border->bottom = window_rect->bottom - content_rect.bottom;

    // LParam points to a NCCALCSIZE_PARAMS struct
	if(wParam == TRUE) 
	{
        NCCALCSIZE_PARAMS *cp = (NCCALCSIZE_PARAMS*)lParam;
        cp->rgrc[0] = content_rect;
        res = WVR_REDRAW;
	}
    // LParam points to a RECT struct
	else 
	{
        *(RECT*)lParam = content_rect;
        res = 0;
	}

    osstyleXP_CloseTheme(theme);
    ReleaseDC(hwnd, hdc);
    return res;
}

/*---------------------------------------------------------------------------*/

#if _MSC_VER > 1400
#define _CP_BORDER CP_BORDER
#define _CBB_NORMAL CBB_NORMAL
#define _CBB_FOCUSED CBB_FOCUSED
#define _CBB_DISABLED CBB_DISABLED
#else
#define _CP_BORDER 4
#define _CBB_NORMAL CBXS_NORMAL
#define _CBB_FOCUSED CBXS_PRESSED
#define _CBB_DISABLED CBXS_DISABLED
#endif

/*---------------------------------------------------------------------------*/

LRESULT _osgui_ncpaint(HWND hwnd, const RECT *border)
{
	HDC hdc = GetWindowDC(hwnd);
    HTHEME theme = NULL;
    int partId = _CP_BORDER;
    int stateId = _CBB_NORMAL;
    RECT window_rect, client_rect;

    if (IsWindowEnabled(hwnd) == TRUE)
    {
        if (GetFocus() == hwnd)
        {
            theme = osstyleXP_OpenTheme(hwnd, L"COMBOBOX");
            partId = _CP_BORDER;
            stateId = _CBB_FOCUSED;
        }
        else
        {
            theme = osstyleXP_OpenTheme(hwnd, L"EDIT");
            partId = EP_EDITTEXT;
            stateId = ETS_NORMAL;
        }
    }
    else
    {
        theme = osstyleXP_OpenTheme(hwnd, L"COMBOBOX");
        partId = _CP_BORDER;
        stateId = _CBB_DISABLED;
    }
    
    GetWindowRect(hwnd, &window_rect);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    window_rect.left = 0;
    window_rect.top = 0;

    client_rect = window_rect;
    client_rect.left += border->left;
    client_rect.top += border->top;
    client_rect.right -= border->right;
    client_rect.bottom -= border->bottom;
	ExcludeClipRect(hdc, client_rect.left, client_rect.top, client_rect.right, client_rect.bottom);

    osstyleXP_DrawThemeBackground2(theme, partId, stateId, hdc, &window_rect);
    osstyleXP_CloseTheme(theme);
    ReleaseDC(hwnd, hdc);
    return 0;
}

/*---------------------------------------------------------------------------*/

void _osgui_start_imp(void)
{
    /* Application instance */
    cassert(i_INSTANCE == NULL);
    i_INSTANCE = (HINSTANCE)GetModuleHandle(NULL);     
    cassert_no_null(i_INSTANCE);

    /* Window Class */
    i_registry_custom_window_class();

    /* View Class */
    i_registry_view_class();

    /* Common controls */
    {
        INITCOMMONCONTROLSEX commctrl;
        BOOL ok;
        commctrl.dwSize = sizeof(INITCOMMONCONTROLSEX);
        commctrl.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS;
        ok = InitCommonControlsEx(&commctrl);
        cassert_unref(ok == TRUE, ok);
    }

    /* GDI Plus */
    /* OJO!!! guiplus de inicia en OSDRAW */
    /* TODO*/
    {
        Gdiplus::GdiplusStartupInput startup;
        Gdiplus::GdiplusStartup(&i_GDIPLUSTOKEN, &startup, NULL);
    }

    /* Default parent Window (an OSView for message callback)
    I found an MSKB article (Article ID: Q104069
    http://support.microsoft.com/support/kb/articles/Q104/0/69.asp) that looks
    like it /might/ be in some way related to what's happening, but when I tried
    implementing what it says to do I still get the same results.  The article
    states:

    An edit, list box, or combo box control sends notifications to the original
    parent window even after SetParent has been used to change the control's
    parent.

    If it is required that notifications go to the new parent window, code must
    be added to the old parent's window procedure to pass on the notifications
    to the new parent. 
    */
    i_DEFAULT_OSPANEL = _ospanel_create_default();
    kDEFAULT_PARENT_WINDOW = ((OSControl*)i_DEFAULT_OSPANEL)->hwnd;

    /* Cursors */
    kNORMAL_ARROW_CURSOR = LoadCursor(NULL, IDC_ARROW);
    kSIZING_HORIZONTAL_CURSOR = LoadCursor(NULL, IDC_SIZENS);
    kSIZING_VERTICAL_CURSOR = LoadCursor(NULL, IDC_SIZEWE);

    /* Brushes and pens */
    {
        unsigned short chess_board[] = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };
        HBITMAP bitmap = CreateBitmap(8, 8, 1, 1, chess_board);
        kCHESSBOARD_BRUSH = CreatePatternBrush(bitmap);
        DeleteObject(bitmap);
    }

    /* RichText Edit Control */
    {
        HMODULE msft_mod = LoadLibrary(L"Msftedit.dll");
        cassert_unref(msft_mod != NULL, msft_mod);
        kRICHEDIT_CLASS = MSFTEDIT_CLASS;
    }

    /* Unicode format for Windows GUI */
    kWINDOWS_UNICODE = ekUTF16;

    /* TWIPS for Font Size */
    {
        HWND hwnd = GetDesktopWindow();
        HDC hdc = GetDC(hwnd);
        kLOG_PIXY_GUI = GetDeviceCaps(hdc, LOGPIXELSY);
        int ret = ReleaseDC(hwnd, hdc);
        cassert_unref(ret == 1, ret);
        kTWIPS_PER_PIXEL_GUI = 1440 / kLOG_PIXY_GUI;
    }

    /* Accelerators */
    kACCELERATORS = arrst_create(ACCEL);
    kACCEL_TABLE = NULL;
}

/*---------------------------------------------------------------------------*/

void _osgui_finish_imp(void)
{
    /* Accelerators */
    if (kACCEL_TABLE != NULL)
    {
        BOOL ok = FALSE;
        cassert(arrst_size(kACCELERATORS, ACCEL) > 0);
        ok = DestroyAcceleratorTable(kACCEL_TABLE);
        cassert_unref(ok == TRUE, ok);
    }
    else
    {
        cassert(arrst_size(kACCELERATORS, ACCEL) == 0);
    }

    arrst_destroy(&kACCELERATORS, NULL, ACCEL);

    /* Unicode */
    kWINDOWS_UNICODE = ENUM_MAX(unicode_t);

    /* RichText Edit Control */
    kRICHEDIT_CLASS = NULL;

    /* Default Parent Window */
    _ospanel_destroy_default(&i_DEFAULT_OSPANEL);
    kDEFAULT_PARENT_WINDOW = NULL;

    /* View Class */
    {
        BOOL ret = UnregisterClass(kVIEW_CLASS, NULL);
        cassert_unref(ret != 0, ret);
    }

    /* Window Class */
    {
        BOOL ret = 0;
        ret = UnregisterClass(kWINDOW_CLASS, NULL);
        cassert(ret != 0);
    }

    /* Brushes and pens */
    DeleteObject(kCHESSBOARD_BRUSH);

    /* OJO!!! guiplus de inicia en OSDRAW */
    /* GDI Plus */
    Gdiplus::GdiplusShutdown(i_GDIPLUSTOKEN);
    i_GDIPLUSTOKEN = 0L;

    /* Application instance */
    i_INSTANCE = NULL;
}

/*---------------------------------------------------------------------------*/

HINSTANCE _osgui_instance(void)
{
    return i_INSTANCE;
}

/*---------------------------------------------------------------------------*/

BOOL _osgui_hit_test(HWND hwnd)
{
    if (hwnd == NULL)
        return FALSE;

    // If mouse captured by someone other than us, indicate no hit...
    {
        HWND capture_hwnd = GetCapture();
        if ((capture_hwnd != NULL) && (capture_hwnd != hwnd))
            return FALSE;
    }

    {
        POINT pt;
        HWND mouse_hwnd = NULL;
        GetCursorPos(&pt);

        // Quit if mouse not over us...
        {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            if (!PtInRect(&rect, pt))
                return FALSE;
        }

        // Get the top-level window that is under the mouse cursor...
        mouse_hwnd = WindowFromPoint(pt);

        if (!IsWindowEnabled(mouse_hwnd))
            return FALSE;

        if (mouse_hwnd == hwnd)
            return TRUE;

        // Convert (x,y) from screen to parent window's client coordinates...
        ScreenToClient(mouse_hwnd, &pt);

        // Verify child window (if any) is under the mouse cursor is us...
        return (ChildWindowFromPointEx(mouse_hwnd, pt, CWP_ALL) == hwnd);
    }
}

/*---------------------------------------------------------------------------*/

void _osgui_add_accelerator(BYTE fVirt, WORD key, WORD cmd)
{
    ACCEL *accel = arrst_new(kACCELERATORS, ACCEL);
    accel->fVirt = fVirt;
    accel->key = key;
    accel->cmd = cmd;

    if (kACCEL_TABLE != NULL)
    {
        BOOL ok = DestroyAcceleratorTable(kACCEL_TABLE);
        cassert_unref(ok == TRUE, ok);
        cassert(arrst_size(kACCELERATORS, ACCEL) > 1);
    }
    else
    {
        cassert(arrst_size(kACCELERATORS, ACCEL) == 1);
    }

    kACCEL_TABLE = CreateAcceleratorTable(arrst_all(kACCELERATORS, ACCEL), (int)arrst_size(kACCELERATORS, ACCEL));
    cassert_no_null(kACCEL_TABLE);
}

/*---------------------------------------------------------------------------*/

void _osgui_remove_accelerator(WORD cmd)
{
    register uint32_t i = UINT32_MAX;
    BOOL ok = FALSE;

    arrst_foreach(accel, kACCELERATORS, ACCEL)
        if (accel->cmd == cmd)
        {
            i = accel_i;
            break;
        }
    arrst_end();

    cassert(i < arrst_size(kACCELERATORS, ACCEL));
    cassert_no_null(kACCEL_TABLE);
    arrst_delete(kACCELERATORS, i, NULL, ACCEL);
    ok = DestroyAcceleratorTable(kACCEL_TABLE);
    cassert_unref(ok == TRUE, ok);

    if (arrst_size(kACCELERATORS, ACCEL) > 0)
    {
        kACCEL_TABLE = CreateAcceleratorTable(arrst_all(kACCELERATORS, ACCEL), (int)arrst_size(kACCELERATORS, ACCEL));
        cassert_no_null(kACCEL_TABLE);
    }
    else
    {
        kACCEL_TABLE = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void _osgui_change_accelerator(BYTE fVirt, WORD key, WORD cmd)
{
    register uint32_t i = UINT32_MAX;
    BOOL ok = FALSE;

    arrst_foreach(accel, kACCELERATORS, ACCEL)
        if (accel->cmd == cmd)
        {
            i = accel_i;
            accel->fVirt = fVirt;
            accel->key = key;
            break;
        }
    arrst_end();

    cassert(i < arrst_size(kACCELERATORS, ACCEL));
    cassert_no_null(kACCEL_TABLE);
    ok = DestroyAcceleratorTable(kACCEL_TABLE);
    cassert_unref(ok == TRUE, ok);
    kACCEL_TABLE = CreateAcceleratorTable(arrst_all(kACCELERATORS, ACCEL), (int)arrst_size(kACCELERATORS, ACCEL));
    cassert_no_null(kACCEL_TABLE);
}

/*---------------------------------------------------------------------------*/

void _osgui_attach_menubar(OSWindow *window, OSMenu *menu)
{
    HMENU hmenu = _osmenu_menubar(menu, window);
    _oswindow_set_menubar(window, hmenu);
}

/*---------------------------------------------------------------------------*/

void _osgui_detach_menubar(OSWindow *window, OSMenu *menu)
{
    HMENU hmenu = _osmenu_menubar_unlink(menu, window);
    _oswindow_unset_menubar(window, hmenu);
}

/*---------------------------------------------------------------------------*/

void _osgui_change_menubar(OSWindow *window, OSMenu *previous_menu, OSMenu *new_menu)
{
    HMENU prev_hmenu = _osmenu_menubar_unlink(previous_menu, window);
    HMENU new_hmenu = _osmenu_menubar(new_menu, window);
    _oswindow_change_menubar(window, prev_hmenu, new_hmenu);
}

/*---------------------------------------------------------------------------*/

bool_t _osgui_process_message(void *msg)
{
    return _oswindow_proccess_message((MSG*)msg, kACCEL_TABLE);
}

/*---------------------------------------------------------------------------*/

void _osgui_word_size(StringSizeData *data, const char_t *word, real32_t *width, real32_t *height)
{
    SIZE word_size;
    uint32_t num_chars = 0, num_bytes = 0;
    WCHAR wword[256];
    BOOL ret = 0;
    cassert_no_null(data);
    cassert_no_null(width);
    cassert_no_null(height);
    num_chars = unicode_nchars(word, ekUTF8);
    num_bytes = unicode_convers(word, (char_t*)wword, ekUTF8, ekUTF16, sizeof(wword));
    cassert(num_bytes < sizeof(wword));
    ret = GetTextExtentPoint32(data->hdc, wword, (int)num_chars, &word_size);
    cassert(ret != 0);
    *width = (real32_t)word_size.cx;
    *height = (real32_t)word_size.cy;
}

/*---------------------------------------------------------------------------*/
