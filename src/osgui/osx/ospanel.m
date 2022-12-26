/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospanel.m
 *
 */

/* Operating System native panel */

#include "osgui_osx.inl"
#include "ospanel.h"
#include "ospanel.inl"
#include "osgui.inl"
#include "osbutton.inl"
#include "oscombo.inl"
#include "oscontrol.inl"
#include "osview.inl"
#include "osedit.inl"
#include "oslabel.inl"
#include "ospopup.inl"
#include "osprogress.inl"
#include "osslider.inl"
#include "ostext.inl"
#include "osupdown.inl"
#include "arrst.h"
#include "cassert.h"
#include "heap.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

typedef struct _area_t Area;
struct _area_t
{
    void *obj;
    NSRect rect;
    NSColor *bgcolor;
    NSColor *skcolor;
};

DeclSt(Area);

/*---------------------------------------------------------------------------*/

@interface OSXPanel : NSView
{
    @public
    NSScrollView *scroll;
    CGSize content_size;
    ArrSt(Area) *areas;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXPanel

/*---------------------------------------------------------------------------*/

- (void)dealloc
{
   [super dealloc];
    heap_auditor_delete("OSXPanel");
}

/*---------------------------------------------------------------------------*/

- (void)drawRect:(NSRect)rect
{
    [super drawRect:rect];
    if (self->areas != NULL)
    {
        arrst_foreach(area, self->areas, Area)
            if (area->bgcolor != nil)
            {
                [area->bgcolor set];
                NSRectFill(area->rect);
            }
            if (area->skcolor != NULL)
            {
                [area->skcolor set];
                NSFrameRect(area->rect);
            }
        arrst_end();
    }
}

/*---------------------------------------------------------------------------*/

- (BOOL)isFlipped
{
    return YES;
}

@end

/*---------------------------------------------------------------------------*/

OSPanel *ospanel_create(const uint32_t flags)
{
    OSXPanel *panel = nil;
    heap_auditor_add("OSXPanel");
    panel = [[OSXPanel alloc] initWithFrame:NSZeroRect];
    panel->areas = NULL;
    panel->content_size = CGSizeMake(-1, -1);
    [panel setAutoresizesSubviews:NO];

    if (flags & ekVIEW_HSCROLL || flags & ekVIEW_VSCROLL)
    {
        NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSZeroRect];
        [scroll setDrawsBackground:NO];
        [scroll setDocumentView:panel];
        [scroll setHasHorizontalScroller:(flags & ekVIEW_HSCROLL) ? YES : NO];
        [scroll setHasVerticalScroller:(flags & ekVIEW_VSCROLL) ? YES : NO];
        [scroll setAutohidesScrollers:YES];
        [scroll setBorderType:(flags & ekVIEW_BORDER) ? NSGrooveBorder : NSNoBorder];
        panel->scroll = scroll;
        return (OSPanel*)scroll;
    }
    else
    {
        panel->scroll = nil;
        return (OSPanel*)panel;
    }
}

/*---------------------------------------------------------------------------*/

static void i_remove_area(Area *area)
{
    if (area->bgcolor != nil)
        [area->bgcolor release];
    if (area->skcolor != nil)
        [area->skcolor release];
}

/*---------------------------------------------------------------------------*/

static OSXPanel *i_get_panel(const OSPanel *panel)
{
    cassert_no_null(panel);
    if([(NSView*)panel isKindOfClass:[OSXPanel class]])
    {
        return (OSXPanel*)panel;
    }
    else
    {
        NSScrollView *scroll = (NSScrollView*)panel;
        return [scroll documentView];
    }
}

/*---------------------------------------------------------------------------*/

void ospanel_destroy(OSPanel **panel)
{
    OSXPanel *lpanel;
    NSScrollView *scroll = nil;

    cassert_no_null(panel);
    lpanel = i_get_panel(*panel);
    cassert_no_null(lpanel);
    cassert([[lpanel subviews] count] == 0);

    if (lpanel->areas != NULL)
        arrst_destroy(&lpanel->areas, i_remove_area, Area);

    scroll = lpanel->scroll;
    [lpanel release];

    if (scroll != nil)
        [scroll release];

    *panel = NULL;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_child(NSView *child, OSPanel *panel)
{
    if (_oslabel_is(child) == YES)
    {
        _oslabel_detach_and_destroy((OSLabel**)&child, panel);
        return;
    }

    if (_osbutton_is(child) == YES)
    {
        _osbutton_detach_and_destroy((OSButton**)&child, panel);
        return;
    }

    if (_ospopup_is(child) == YES)
    {
        _ospopup_detach_and_destroy((OSPopUp**)&child, panel);
        return;
    }

    if (_osedit_is(child) == YES)
    {
        _osedit_detach_and_destroy((OSEdit**)&child, panel);
        return;
    }

    if (_oscombo_is(child) == YES)
    {
        _oscombo_detach_and_destroy((OSCombo**)&child, panel);
        return;
    }

    if (_osslider_is(child) == YES)
    {
        _osslider_detach_and_destroy((OSSlider**)&child, panel);
        return;
    }

    if (_osupdown_is(child) == YES)
    {
        _osupdown_detach_and_destroy((OSUpDown**)&child, panel);
        return;
    }

    if (_osprogress_is(child) == YES)
    {
        _osprogress_detach_and_destroy((OSProgress**)&child, panel);
        return;
    }

    if (_ostext_is(child) == YES)
    {
        _ostext_detach_and_destroy((OSText**)&child, panel);
        return;
    }

    if (_osview_is(child) == YES)
    {
        _osview_detach_and_destroy((OSView**)&child, panel);
        return;
    }

    if ([child isKindOfClass:[OSXPanel class]])
    {
        ospanel_detach((OSPanel*)child, panel);
        _ospanel_destroy((OSPanel**)&child);
        return;
    }

    cassert_fatal_msg(FALSE, "Unknown child type");
}

/*---------------------------------------------------------------------------*/

void _ospanel_destroy(OSPanel **panel)
{
    register OSXPanel *panelp;
    register NSArray *subviews;
    register NSUInteger i, num_elems;
    cassert_no_null(panel);
    panelp = i_get_panel(*panel);
    cassert_no_null(panelp);
    subviews = [panelp subviews];
    num_elems = [subviews count];
    for (i = 0; i < num_elems; ++i)
    {
        i_destroy_child((NSView*)[subviews objectAtIndex:0], *panel);
        cassert([subviews count] == num_elems - i - 1);
    }

    ospanel_destroy(panel);
}

/*---------------------------------------------------------------------------*/

void ospanel_area(OSPanel *panel, void *obj, const color_t bgcolor, const color_t skcolor, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXPanel *lpanel = i_get_panel(panel);
    cassert_no_null(lpanel);
    if (obj != NULL)
    {
        Area *area = NULL;

        if (lpanel->areas == NULL)
            lpanel->areas = arrst_create(Area);

        arrst_foreach(larea, lpanel->areas, Area)
        if (larea->obj == obj)
        {
            area = larea;
            break;
        }
        arrst_end();

        if (area == NULL)
        {
            area = arrst_new(lpanel->areas, Area);
            area->obj = obj;
            area->bgcolor = nil;
            area->skcolor = nil;
        }

        area->rect.origin.x = (CGFloat)x;
        area->rect.origin.y = (CGFloat)y;
        area->rect.size.width = (CGFloat)width;
        area->rect.size.height = (CGFloat)height;

        if (area->bgcolor != nil)
        {
            [area->bgcolor release];
            area->bgcolor = nil;
        }

        if (area->skcolor != nil)
        {
            [area->skcolor release];
            area->skcolor = nil;
        }

        if (bgcolor != 0)
            area->bgcolor = [_oscontrol_color(bgcolor) retain];

        if (skcolor != 0)
            area->skcolor = [_oscontrol_color(skcolor) retain];
    }
    else
    {
        if (lpanel->areas != NULL)
            arrst_clear(lpanel->areas, i_remove_area, Area);
    }
}

/*---------------------------------------------------------------------------*/

void ospanel_scroller_size(const OSPanel *panel, real32_t *width, real32_t *height)
{
    if ([(NSView*)panel isKindOfClass:[NSScrollView class]])
    {
        NSScrollView *scroll = (NSScrollView*)panel;

        if (width)
        {
            NSScroller *scroller = [scroll verticalScroller];
            *width = 0;
            if (scroller != nil)
                *width = (real32_t)[scroller frame].size.width;
        }

        if (height)
        {
            NSScroller *scroller = [scroll horizontalScroller];
            *height = 0;
            if (scroller != nil)
                *height = (real32_t)[scroller frame].size.height;
        }
    }
    else
    {
        if (width)
            *width = 16;

        if (height)
            *height = 16;
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnUpdateSize(OSPanel *panel)
{
    OSXPanel *lpanel = i_get_panel(panel);
    if ([(NSView*)panel isKindOfClass:[NSScrollView class]])
    {
        CGFloat diff = 0;
        NSScroller *scroller = [(NSScrollView*)panel verticalScroller];
        if (scroller != nil && [scroller isHidden] == NO)
            diff = [scroller frame].size.width;

        [lpanel setFrame:NSMakeRect(0, 0, lpanel->content_size.width - diff, lpanel->content_size.height)];
    }
    else
    {
        cassert(lpanel->content_size.width == -1);
        cassert(lpanel->content_size.height == -1);
    }
}

/*---------------------------------------------------------------------------*/

void ospanel_content_size(OSPanel *panel, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    OSXPanel *lpanel = i_get_panel(panel);
    cassert([(NSView*)panel isKindOfClass:[NSScrollView class]]);
    cassert(lpanel == [(NSScrollView*)panel documentView]);
    unref(line_width);
    unref(line_height);
    lpanel->content_size.width = (CGFloat)width;
    lpanel->content_size.height = (CGFloat)height;
    i_OnUpdateSize(panel);
}

/*---------------------------------------------------------------------------*/

void ospanel_display(OSPanel *panel)
{
    OSXPanel *lpanel = i_get_panel(panel);
    [lpanel setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

void ospanel_attach(OSPanel *panel, OSPanel *parent_panel)
{
    _oscontrol_attach_to_parent((NSView*)panel, (NSView*)parent_panel);
}

/*---------------------------------------------------------------------------*/

void ospanel_detach(OSPanel *panel, OSPanel *parent_panel)
{
    _oscontrol_detach_from_parent((NSView*)panel, (NSView*)parent_panel);
}

/*---------------------------------------------------------------------------*/

void ospanel_visible(OSPanel *panel, const bool_t is_visible)
{
    _oscontrol_set_visible((NSView*)panel, is_visible);
}

/*---------------------------------------------------------------------------*/

void ospanel_enabled(OSPanel *panel, const bool_t is_enabled)
{
    unref(panel);
    unref(is_enabled);
}

/*---------------------------------------------------------------------------*/

void ospanel_size(const OSPanel *panel, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((NSView*)panel, width, height);
}

/*---------------------------------------------------------------------------*/

void ospanel_origin(const OSPanel *panel, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((NSView*)panel, x, y);
}

/*---------------------------------------------------------------------------*/

void ospanel_frame(OSPanel *panel, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((NSView*)panel, x, y, width, height);
    i_OnUpdateSize(panel);
}

/*---------------------------------------------------------------------------*/

void _ospanel_attach_control(OSPanel *panel, NSView *control)
{
    OSXPanel *lpanel = i_get_panel(panel);
    _oscontrol_attach_to_parent(control, (NSView*)lpanel);
}

/*---------------------------------------------------------------------------*/

void _ospanel_detach_control(OSPanel *panel, NSView *control)
{
    OSXPanel *lpanel = i_get_panel(panel);
    _oscontrol_detach_from_parent(control, (NSView*)lpanel);
}
