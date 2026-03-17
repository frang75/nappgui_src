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

#include "osgui_gtk.inl"
#include "oscontrol_gtk.inl"
#include "osglobals_gtk.inl"
#include "ospanel_gtk.inl"
#include "../osline.h"
#include "../osline.inl"
#include "../osgui.inl"
#include <sewer/cassert.h>
#include <core/heap.h>
#include <core/strings.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _osline_t
{
    OSControl control;
    uint32_t flags;
};

/*---------------------------------------------------------------------------*/

OSLine *osline_create(const uint32_t flags)
{
    OSLine *line = heap_new(OSLine);
    GtkWidget *widget = gtk_separator_new(line_get_type(flags) == ekLINE_HORZ ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
    _oscontrol_init(&line->control, ekGUI_TYPE_LINE, widget, widget, TRUE);
    line->flags = flags;
    return line;
}

/*---------------------------------------------------------------------------*/

void osline_destroy(OSLine **line)
{
    cassert_no_null(line);
    cassert_no_null(*line);
    _oscontrol_destroy(*dcast(line, OSControl));
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
        *height = 1;
    }
    else
    {
        *width = 1;
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
