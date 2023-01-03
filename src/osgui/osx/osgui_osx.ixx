/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui_osx.ixx
 *
 */

/* Operating system native gui */

#ifndef __OSGUI_OSX_IXX__
#define __OSGUI_OSX_IXX__

#include "nowarn.hxx"
#include <Cocoa/Cocoa.h>
#include "warn.hxx"
#include "osgui.ixx"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

typedef struct _view_listeners_t ViewListeners;
typedef struct _osscrollview_t OSScrollView;
typedef struct _ostext_attr_t OSTextAttr;

struct _view_listeners_t
{
    BOOL is_enabled;
    NSUInteger OnMouseMoved_event_source;
    Listener *OnDraw;
    Listener *OnEnter;
    Listener *OnExit;
    Listener *OnMoved;
    Listener *OnDown;
    Listener *OnUp;
    Listener *OnClick;
    Listener *OnDrag;
    Listener *OnWheel;
    Listener *OnKeyUp;
    Listener *OnKeyDown;
    NSUInteger modifier_flags;
};

struct _strsize_data_t
{
    NSDictionary *dict;
};

struct _ostext_attr_t
{
    Font *font;
    color_t color;
    align_t align;
};

#endif

