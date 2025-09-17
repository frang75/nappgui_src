/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: comwin.c
 *
 */

/* Common windows */

#include "comwin.h"
#include "window.inl"
#include <draw2d/guictx.h>
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

const char_t *comwin_select_dir(Window *parent, const char_t *caption, const char_t *start_dir)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = parent ? _window_ositem(parent) : NULL;
    cassert_no_null(context);
    return context->func_comwin_dir(ositem, caption, start_dir);
}

/*---------------------------------------------------------------------------*/

const char_t *comwin_open_file(Window *parent, const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = parent ? _window_ositem(parent) : NULL;
    cassert_no_null(context);
    return context->func_comwin_file(ositem, caption, ftypes, size, start_dir, TRUE);
}

/*---------------------------------------------------------------------------*/

const char_t *comwin_save_file(Window *parent, const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = parent ? _window_ositem(parent) : NULL;
    cassert_no_null(context);
    return context->func_comwin_file(ositem, caption, ftypes, size, start_dir, FALSE);
}

/*---------------------------------------------------------------------------*/

void comwin_color(Window *parent, const char_t *caption, const real32_t x, const real32_t y, const align_t halign, const align_t valign, const color_t current, color_t *colors, const uint32_t n, Listener *OnChange)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = parent ? _window_ositem(parent) : NULL;
    cassert_no_null(context);
    context->func_comwin_color(ositem, caption, x, y, halign, valign, current, colors, n, OnChange);
}
