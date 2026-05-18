/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabs.m
 *
 */

/* Operating System native tabctrl */

#include "ostabs_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "oswindow_osx.inl"
#include "../ostabs.h"
#include "../osgui.inl"
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
@interface OSXTabsDelegate : NSObject < NSTabViewDelegate >
#else
@interface OSXTabsDelegate : NSObject
#endif
{
  @public
    bool_t launch_event;
    Listener *OnSelect;
}

@end

/*---------------------------------------------------------------------------*/

@interface OSXTabs : NSTabView
{
  @public
    uint32_t flags;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXTabs

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    if (_oswindow_mouse_down(cast(self, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

@end

/*---------------------------------------------------------------------------*/

@implementation OSXTabsDelegate

- (void)dealloc
{
    [super dealloc];
    heap_auditor_delete("OSXTabsDelegate");
}

/*---------------------------------------------------------------------------*/

- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
    cassert([tabView delegate] == self);
    if (self->launch_event == TRUE)
    {
        if (self->OnSelect != NULL)
        {
            NSInteger index = [tabView indexOfTabViewItem:tabViewItem];
            EvButton params;
            params.state = ekGUI_ON;
            params.index = (uint32_t)index;
            params.text = NULL;
            listener_event(self->OnSelect, ekGUI_EVENT_TABS, cast(self, OSTabs), &params, NULL, OSTabs, EvButton, void);
        }
    }
}

@end

/*---------------------------------------------------------------------------*/

OSTabs *ostabs_create(const uint32_t flags)
{
    OSXTabs *tabs = nil;
    OSXTabsDelegate *delegate = nil;
    NSTabViewType type = NSTopTabsBezelBorder;
    heap_auditor_add("OSXTabs");
    tabs = [[OSXTabs alloc] initWithFrame:NSZeroRect];
    tabs->flags = flags;
    _oscontrol_init(tabs);
    heap_auditor_add("OSXTabsDelegate");
    delegate = [OSXTabsDelegate alloc];
    delegate->launch_event = TRUE;
    delegate->OnSelect = NULL;
    [tabs setDelegate:delegate];

    switch (tabs_get_pos(flags))
    {
    case ekTABS_LEFT:
        type = NSLeftTabsBezelBorder;
        break;
    case ekTABS_TOP:
        type = NSTopTabsBezelBorder;
        break;
    case ekTABS_RIGHT:
        type = NSRightTabsBezelBorder;
        break;
    case ekTABS_BOTTOM:
        type = NSBottomTabsBezelBorder;
        break;
    default:
        cassert_default(tabs_get_pos(flags));
    }

    [tabs setTabViewType:type];
    return cast(tabs, OSTabs);
}

/*---------------------------------------------------------------------------*/

void ostabs_destroy(OSTabs **tabs)
{
    OSXTabs *ltabs = nil;
    OSXTabsDelegate *delegate = nil;
    cassert_no_null(tabs);
    ltabs = *dcast(tabs, OSXTabs);
    cassert_no_null(ltabs);
    delegate = [ltabs delegate];
    cassert_no_null(delegate);
    listener_destroy(&delegate->OnSelect);
    [delegate release];
    [ltabs release];
    *tabs = NULL;
    heap_auditor_delete("OSXTabs");
}

/*---------------------------------------------------------------------------*/

void ostabs_OnSelect(OSTabs *tabs, Listener *listener)
{
    OSXTabs *ltabs = nil;
    OSXTabsDelegate *delegate = nil;
    ltabs = cast(tabs, OSXTabs);
    cassert_no_null(ltabs);
    delegate = [ltabs delegate];
    cassert_no_null(delegate);
    listener_update(&delegate->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void ostabs_tooltip(OSTabs *tabs, const char_t *text)
{
    cassert_no_null(tabs);
    _oscontrol_tooltip_set(cast(tabs, OSXTabs), text);
}

/*---------------------------------------------------------------------------*/

void ostabs_font(OSTabs *tabs, const Font *font)
{
    unref(tabs);
    unref(font);
}

/*---------------------------------------------------------------------------*/

void ostabs_elem(OSTabs *tabs, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    OSXTabs *ltabs = cast(tabs, OSXTabs);
    OSXTabsDelegate *delegate = nil;
    cassert_no_null(ltabs);
    delegate = [ltabs delegate];
    cassert_no_null(delegate);
    unref(image);
    delegate->launch_event = FALSE;

    switch (op)
    {
    case ekCTRL_OP_ADD:
    case ekCTRL_OP_INS:
    {
        NSTabViewItem *item = [[NSTabViewItem alloc] init];
        NSView *dummy = [[NSView alloc] initWithFrame:NSZeroRect];
        [item setLabel:[NSString stringWithUTF8String:cast_const(text, char)]];
        [item setView:dummy];

        if (op == ekCTRL_OP_ADD)
            [ltabs addTabViewItem:item];
        else
            [ltabs insertTabViewItem:item atIndex:(NSInteger)index];
        break;
    }

    case ekCTRL_OP_SET:
    {
        NSTabViewItem *item = [ltabs tabViewItemAtIndex:(NSInteger)index];
        [item setLabel:[NSString stringWithUTF8String:cast_const(text, char)]];
        break;
    }

    case ekCTRL_OP_DEL:
    {
        NSTabViewItem *item = [ltabs tabViewItemAtIndex:(NSInteger)index];
        [ltabs removeTabViewItem:item];
        break;
    }

    default:
        cassert_default(op);
    }

    delegate->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void ostabs_selected(OSTabs *tabs, const uint32_t index)
{
    OSXTabs *ltabs = cast(tabs, OSXTabs);
    unref(ltabs);
    if (index != UINT32_MAX)
        [ltabs selectTabViewItemAtIndex:(NSInteger)index];
    else
        [ltabs selectTabViewItemAtIndex:-1];
}

/*---------------------------------------------------------------------------*/

uint32_t ostabs_get_selected(const OSTabs *tabs)
{
    OSXTabs *ltabs = cast(tabs, OSXTabs);
    unref(ltabs);
    return (uint32_t)[ltabs indexOfTabViewItem:[ltabs selectedTabViewItem]];
}

/*---------------------------------------------------------------------------*/

void ostabs_bounds(const OSTabs *tabs, const real32_t length, real32_t *width, real32_t *height)
{
    OSXTabs *ltabs = cast(tabs, OSXTabs);
    NSRect bounds = [ltabs bounds];
    NSRect content = [ltabs contentRect];
    cassert_no_null(ltabs);
    cassert_no_null(width);
    cassert_no_null(height);
    switch (tabs_get_pos(ltabs->flags))
    {
    case ekTABS_LEFT:
    case ekTABS_RIGHT:
        *width = (real32_t)30;
        *height = length;
        break;
    case ekTABS_TOP:
    case ekTABS_BOTTOM:
        *width = length;
        *height = (real32_t)30;
        break;
    default:
        cassert_default(tabs_get_pos(ltabs->flags));
    }
}

/*---------------------------------------------------------------------------*/

void ostabs_attach(OSTabs *tabs, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(tabs, NSView));
}

/*---------------------------------------------------------------------------*/

void ostabs_detach(OSTabs *tabs, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(tabs, NSView));
}

/*---------------------------------------------------------------------------*/

void ostabs_visible(OSTabs *tabs, const bool_t visible)
{
    _oscontrol_set_visible(cast(tabs, NSView), visible);
}

/*---------------------------------------------------------------------------*/

void ostabs_enabled(OSTabs *tabs, const bool_t enabled)
{
    unref(tabs);
    unref(enabled);
}

/*---------------------------------------------------------------------------*/

void ostabs_size(const OSTabs *tabs, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast(tabs, NSView), width, height);
}

/*---------------------------------------------------------------------------*/

void ostabs_origin(const OSTabs *tabs, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast(tabs, NSView), x, y);
}

/*---------------------------------------------------------------------------*/

void ostabs_frame(OSTabs *tabs, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(tabs, NSView), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

BOOL _ostabs_is(NSView *view)
{
    return [view isKindOfClass:[OSXTabs class]];
}
