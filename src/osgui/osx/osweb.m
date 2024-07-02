/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osweb.m
 *
 */

/* Cocoa WebView wrapper */

#include "osgui_osx.inl"
#include "osweb.h"
#include "osweb_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

#if defined(NAPPGUI_WEB_SUPPORT)
/* WKWebView only available from Yosemite 10.10 */
#if !defined(MAC_OS_X_VERSION_10_10) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_10
#undef NAPPGUI_WEB_SUPPORT
#endif
#endif

#if defined(NAPPGUI_WEB_SUPPORT)

#include <WebKit/WebKit.h>

/*---------------------------------------------------------------------------*/

@interface OSXWebView : WKWebView
{
  @public
    Listener *OnFocus;
}
@end

#else

@interface OSXWebView : NSView
{
  @public
    Listener *OnFocus;
}
@end

#endif

/*---------------------------------------------------------------------------*/

@implementation OSXWebView

- (void)dealloc
{
    [super dealloc];
}

/*---------------------------------------------------------------------------*/

- (void)drawRect:(NSRect)rect
{
    [super drawRect:rect];
}

/*---------------------------------------------------------------------------*/

- (void)drawFocusRingMask
{
    NSRectFill([self bounds]);
}

/*---------------------------------------------------------------------------*/

- (NSRect)focusRingMaskBounds
{
    return [self bounds];
}

@end

/*---------------------------------------------------------------------------*/

OSWeb *osweb_create(const uint32_t flags)
{
    OSXWebView *view = [[OSXWebView alloc] initWithFrame:NSZeroRect];
    unref(flags);
    heap_auditor_add("OSXWebView");
    view->OnFocus = NULL;
    return cast(view, OSWeb);
}

/*---------------------------------------------------------------------------*/

void osweb_destroy(OSWeb **view)
{
    OSXWebView *lview = nil;
    cassert_no_null(view);
    lview = cast(*view, OSXWebView);
    cassert_no_null(lview);
    listener_destroy(&lview->OnFocus);
    heap_auditor_delete("OSXWebView");
    [lview release];
    *view = NULL;
}

/*---------------------------------------------------------------------------*/

void osweb_OnFocus(OSWeb *view, Listener *listener)
{
    OSXWebView *lview = cast(view, OSXWebView);
    cassert_no_null(lview);
    listener_update(&lview->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osweb_command(OSWeb *view, const gui_web_t cmd, const void *param, void *result)
{
#if defined(NAPPGUI_WEB_SUPPORT)
    OSXWebView *lview = cast(view, OSXWebView);
    cassert_no_null(lview);
    unref(result);
    switch (cmd)
    {
    case ekGUI_WEB_NAVIGATE:
    {
        NSString *str = [NSString stringWithUTF8String:cast_const(param, char)];
        NSURL *url = [NSURL URLWithString:str];
        NSURLRequest *request = [NSURLRequest requestWithURL:url];
        [lview loadRequest:request];
        break;
    }

    case ekGUI_WEB_BACK:
        [lview goBack];
        break;

    case ekGUI_WEB_FORWARD:
        [lview goForward];
        break;

        cassert_default();
    }
#else
    unref(view);
    unref(cmd);
    unref(param);
    unref(result);
#endif
}

/*---------------------------------------------------------------------------*/

void osweb_scroller_visible(OSWeb *view, const bool_t horizontal, const bool_t vertical)
{
    unref(view);
    unref(horizontal);
    unref(vertical);
}

/*---------------------------------------------------------------------------*/

void osweb_set_need_display(OSWeb *view)
{
    unref(view);
}

/*---------------------------------------------------------------------------*/

void osweb_clipboard(OSWeb *view, const clipboard_t clipboard)
{
    unref(view);
    unref(clipboard);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void osweb_attach(OSWeb *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(view, NSView));
}

/*---------------------------------------------------------------------------*/

void osweb_detach(OSWeb *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(view, NSView));
}

/*---------------------------------------------------------------------------*/

void osweb_visible(OSWeb *view, const bool_t visible)
{
    _oscontrol_set_visible(cast(view, NSView), visible);
}

/*---------------------------------------------------------------------------*/

void osweb_enabled(OSWeb *view, const bool_t enabled)
{
    unref(view);
    unref(enabled);
}

/*---------------------------------------------------------------------------*/

void osweb_size(const OSWeb *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(view, NSView), width, height);
}

/*---------------------------------------------------------------------------*/

void osweb_origin(const OSWeb *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(view, NSView), x, y);
}

/*---------------------------------------------------------------------------*/

void osweb_frame(OSWeb *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(view, NSView), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

BOOL _osweb_is(NSView *view)
{
    return [view isKindOfClass:[OSXWebView class]];
}
