/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osline.c
 *
 */

/* Operating System native line */

#include "osgui_win.inl"
#include "oscontrol_win.inl"
#include "ospanel_win.inl"
#include "../osline.h"
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _osline_t
{
    OSControl control;
    uint32_t flags;
};

/*---------------------------------------------------------------------------*/

OSLine *osline_create(const uint32_t flags)
{
    OSLine *line = NULL;
    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;

    if (line_get_type(flags) == ekLINE_HORZ)
        dwStyle |= SS_ETCHEDHORZ;
    else
        dwStyle |= SS_ETCHEDVERT;

    line = heap_new(OSLine);
    line->control.type = ekGUI_TYPE_PROGRESS;
    line->flags = flags;
    _oscontrol_init(cast(line, OSControl), PARAM(dwExStyle, 0), dwStyle, L"static", 0, 0, NULL, kDEFAULT_PARENT_WINDOW);
    return line;
}

/*---------------------------------------------------------------------------*/

void osline_destroy(OSLine **line)
{
    cassert_no_null(line);
    cassert_no_null(*line);
    _oscontrol_destroy(&(*line)->control);
    heap_delete(line, OSLine);
}

/*---------------------------------------------------------------------------*/

void osline_bounds(const OSLine *line, const real32_t length, real32_t *width, real32_t *height)
{
    cassert_no_null(line);
    cassert_no_null(width);
    cassert_no_null(height);
    if (line_get_type(line->flags) == ekLINE_HORZ)
    {
        *width = length;
        *height = 2;
    }
    else
    {
        *width = 2;
        *height = length;
    }
}

/*---------------------------------------------------------------------------*/

void osline_attach(OSLine *line, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(line, OSControl));
}

/*---------------------------------------------------------------------------*/

void osline_detach(OSLine *line, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(line, OSControl));
}

/*---------------------------------------------------------------------------*/

void osline_visible(OSLine *line, const bool_t visible)
{
    _oscontrol_set_visible(cast(line, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osline_enabled(OSLine *line, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(line, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osline_size(const OSLine *line, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(line, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osline_origin(const OSLine *line, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(line, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osline_frame(OSLine *line, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(line, OSControl), x, y, width, height);
}
