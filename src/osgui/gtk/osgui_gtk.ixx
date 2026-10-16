/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: osgui_gtk.ixx
 *
 */

/* Operating system native gui */

#ifndef __OSGUIGTK_IXX__
#define __OSGUIGTK_IXX__

#include "../osgui.ixx"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

#include <sewer/nowarn.hxx>
#include <gtk/gtk.h>
#include <sewer/warn.hxx>

typedef struct _view_listeners_t ViewListeners;
typedef struct _osentry_t OSEntry;
typedef struct _osfiller_t OSFiller;
typedef GtkWidget *(*FPtr_filler)(OSControl *control);

struct _view_listeners_t
{
    bool_t is_enabled;
    gui_mouse_t button;
    gulong enter_signal;
    gulong leave_signal;
    gulong pressed_signal;
    gulong release_signal;
    gulong moved_signal;
    gulong wheel_signal;
    gulong keypressed_signal;
    gulong keyrelease_signal;
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

struct _osfiller_t
{
    GtkWidget *widget;
    FPtr_filler func_create;
    uint32_t width;
    uint32_t height;
    bool_t active;
};

struct _oscontrol_t
{
    gui_type_t type;
    GtkWidget *widget;
    OSFiller filler;

#if defined(__ASSERTS__)
    bool_t is_alive;
#endif
};

struct _strsize_data_t
{
    PangoLayout *layout;
};

#endif
