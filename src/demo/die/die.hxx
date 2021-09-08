/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: die.hxx
 * https://nappgui.com/en/demo/die/die.html
 *
 */

/* Die Types */

#ifndef __DIE_HXX__
#define __DIE_HXX__

#include "gui.hxx"

typedef struct _app_t App;

struct _app_t
{
    real32_t padding;
    real32_t corner;
    real32_t radius;
    uint32_t face;
    View *view;
    Window *window;
};

#endif