/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui.ixx
 *
 */

/* Operating system native gui */

#ifndef __OSGUI_IXX__
#define __OSGUI_IXX__

#include "osgui.hxx"

typedef struct _oswidget_t OSWidget;
typedef struct _oshotkey_t OSHotKey;
typedef struct _osframe_t OSFrame;
typedef struct _ostabstop_t OSTabStop;
typedef struct _osscrolls_t OSScrolls;
typedef struct _osscroll_t OSScroll;
typedef struct _strsize_data_t StringSizeData;

struct _oshotkey_t
{
    vkey_t key;
    uint32_t modifiers;
    Listener *listener;
};

struct _osframe_t
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

struct _ostabstop_t
{
    ArrPt(OSControl) *controls;
    ArrPt(OSControl) *tablist;
    uint32_t tabindex;
    gui_tab_t motion;
    OSWindow *window;
    OSButton *defbutton;
    OSControl *current;
    OSControl *transient;
    OSControl *next;
    bool_t cycle;
};

DeclSt(OSHotKey);
DeclPt(OSControl);
DeclPt(OSMenuItem);

#endif
