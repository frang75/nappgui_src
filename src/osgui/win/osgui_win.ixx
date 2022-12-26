/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui_win.ixx
 *
 */

/* Operating system native gui */

#ifndef __OSGUIWIN_IXX__
#define __OSGUIWIN_IXX__

#include "osgui.ixx"

#if _MSC_VER == 1400
#define _WIN32_WINNT 0x501
#endif

#include "nowarn.hxx"
#include <windows.h>
#include <uxtheme.h>
#if _MSC_VER > 1400
#include <vssym32.h>
#include <vsstyle.h>
#else
#include <tmschema.h>
#endif
#include "warn.hxx"

#define WCHAR_BUFFER_SIZE		512

typedef struct _view_listeners_t ViewListeners;
typedef struct _osimgdata_t OSImgData;
typedef struct _osimglist_t OSImgList;
typedef struct _osscroll_t OSScroll;
typedef struct _osdraw_t OSDraw;

struct _view_listeners_t
{
    BOOL is_mouse_inside;
    BOOL enabled;
	gui_mouse_t button;
    LONG OnMouseMoved_event_source;
    real32_t mouse_down_x;
    real32_t mouse_down_y;
    uint64_t mouse_down_time;
    Listener *OnDraw;
    Listener *OnEnter;
    Listener *OnExit;
    Listener *OnMoved;
    Listener *OnDown;
    Listener *OnUp;
    Listener *OnClick;
    Listener *OnDrag;
    Listener *OnWheel;
    Listener *OnKeyDown;
    Listener *OnKeyUp;
};

struct _oscontrol_t
{
    gui_type_t type;
    HWND hwnd;
    HWND tooltip_hwnd;
    WNDPROC def_wnd_proc;
    int x;
    int y;
    bool_t visible;
};

struct _strsize_data_t
{
    HDC hdc;
};

struct _osdraw_t
{
    HTHEME button_theme;
    HTHEME list_theme;
    HTHEME header_theme;
    SIZE sort_size;
};

#endif

