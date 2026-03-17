/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osline.m
 *
 */

/* Operating System native line */

#include "osline_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "../osline.h"
#include "../osgui.inl"
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXLine : NSBox
{
  @public
    uint32_t flags;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXLine

@end

/*---------------------------------------------------------------------------*/

OSLine *osline_create(const uint32_t flags)
{
    OSXLine *line = nil;
    heap_auditor_add("OSXLine");
    line = [[OSXLine alloc] initWithFrame:NSZeroRect];
    _oscontrol_init(line);
    [line setBoxType:NSBoxSeparator];
    line->flags = flags;
    return cast(line, OSLine);
}

/*---------------------------------------------------------------------------*/

void osline_destroy(OSLine **line)
{
    OSXLine *lline = nil;
    cassert_no_null(line);
    lline = *dcast(line, OSXLine);
    cassert_no_null(lline);
    [lline release];
    *line = NULL;
    heap_auditor_delete("OSXLine");
}

/*---------------------------------------------------------------------------*/

void osline_bounds(const OSLine *line, const real32_t length, real32_t *width, real32_t *height)
{
    OSXLine *lline = cast(line, OSXLine);
    cassert_no_null(lline);
    cassert_no_null(width);
    cassert_no_null(height);
    if (line_get_type(lline->flags) == ekLINE_HORZ)
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
    _ospanel_attach_control(panel, cast(line, NSView));
}

/*---------------------------------------------------------------------------*/

void osline_detach(OSLine *line, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(line, NSView));
}

/*---------------------------------------------------------------------------*/

void osline_visible(OSLine *line, const bool_t visible)
{
    _oscontrol_set_visible(cast(line, NSView), visible);
}

/*---------------------------------------------------------------------------*/

void osline_enabled(OSLine *line, const bool_t enabled)
{
    cassert_no_null(line);
    unref(enabled);
}

/*---------------------------------------------------------------------------*/

void osline_size(const OSLine *line, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast(line, NSView), width, height);
}

/*---------------------------------------------------------------------------*/

void osline_origin(const OSLine *line, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast(line, NSView), x, y);
}

/*---------------------------------------------------------------------------*/

void osline_frame(OSLine *line, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(line, NSView), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

BOOL _osline_is(NSView *view)
{
    return [view isKindOfClass:[OSXLine class]];
}
