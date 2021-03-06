/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: globals.c
 *
 */

/* System globals */

#include "globals.h"
#include "guicontexth.inl"
#include "draw2d.ixx"
#include "cassert.h"
#include "color.h"

/*---------------------------------------------------------------------------*/

device_t globals_device(void)
{
    const GuiContext *context = gui_context_get_current();
    cassert_no_null(context);
    cassert_no_nullf(context->func_globals_device);
    return (device_t)context->func_globals_device(NULL);
}

/*---------------------------------------------------------------------------*/

void globals_resolution(S2Df *resolution)
{
    const GuiContext *context = gui_context_get_current();
    cassert_no_null(context);
    cassert_no_nullf(context->func_globals_resolution);
    cassert_no_null(resolution);
    context->func_globals_resolution(NULL, &resolution->width, &resolution->height);
}

/*---------------------------------------------------------------------------*/

V2Df globals_mouse_position(void)
{
    V2Df pos;
    const GuiContext *context = gui_context_get_current();
    cassert_no_null(context);
    cassert_no_nullf(context->func_globals_mouse_position);
    context->func_globals_mouse_position(NULL, &pos.x, &pos.y);
    return pos;
}

/*---------------------------------------------------------------------------*/

//uint32_t globals_scrollbar_width(void);
//uint32_t globals_scrollbar_width(void)
//{
//    uint32_t v = 0;
//    const GuiContext *context = gui_context_get_current();
//    cassert_no_null(context);
//    cassert_no_nullf(context->func_globals_value);
//    context->func_globals_value(0, (void*)&v);
//    return v;
//}
