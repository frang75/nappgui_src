/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.m
 *
 */

/* Cocoa NSwindow wrapper */

#include "osgui_osx.inl"
#include "oswindow.h"
#include "oswindow.inl"
#include "osgui.inl"
#include "osbutton.inl"
#include "oscontrol.inl"
#include "ospanel.inl"
#include "osview.inl"
#include "cassert.h"
#include "arrst.h"
#include "event.h"
#include "heap.h"
#include "ptr.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

typedef struct _hotkey_t HotKey;

struct _hotkey_t
{
    vkey_t key;
    uint32_t modifiers;
    Listener *listener;
};

DeclSt(HotKey);

/*---------------------------------------------------------------------------*/

@interface OSXWindow : NSPanel
{
    @public
    NSPoint origin;
    BOOL in_window_destroy;
    BOOL last_moved_by_interface;
    BOOL can_become_key_window;
    BOOL launch_resize_event;
    NSResponder *keyboard_focus;
    uint32_t flags;
    CGFloat alpha;
    ArrSt(HotKey) *hotkeys;
    NSView *first_in_key_loop;
    NSView *current_in_key_loop;
    BOOL destroy_main_view;
    OSButton *defbutton;
}
@end

/*---------------------------------------------------------------------------*/

#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    @interface OSXWindowDelegate : NSObject<NSWindowDelegate>
#else
    @interface OSXWindowDelegate : NSObject
#endif
{
@public
    Listener *OnMoved;
    Listener *OnResize;
    Listener *OnClose;
}

@end

/*---------------------------------------------------------------------------*/

@implementation OSXWindowDelegate

- (void)dealloc
{
    [super dealloc];
    heap_auditor_delete("OSXWindowDelegate");
}

/*---------------------------------------------------------------------------*/

/*
- (void)windowDidBecomeKey:(NSNotification*)notification
{
    OSXWindow *window;
    window = [notification object];
    cassert_no_null(window);
    [window makeFirstResponder:nil];
}*/

/*---------------------------------------------------------------------------*/

- (void)windowWillMove:(NSNotification*)notification
{
    OSXWindow *window;
    window = [notification object];
    cassert_no_null(window);
    window->last_moved_by_interface = NO;
}

/*---------------------------------------------------------------------------*/

- (void)windowDidMove:(NSNotification*)notification
{
    OSXWindow *window;
    window = [notification object];
    cassert_no_null(window);
    if (self->OnMoved != NULL && window->last_moved_by_interface == NO)
    {
        NSRect frame;
        EvPos params;
        CGFloat origin_x, origin_y;
        frame = [window frame];
        _oscontrol_origin_in_screen_coordinates(&frame, &origin_x, &origin_y);
        params.x = (real32_t)origin_x;
        params.y = (real32_t)origin_y;
        listener_event(self->OnMoved, ekGUI_EVENT_WND_MOVED, (OSWindow*)window, &params, NULL, OSWindow, EvPos, void);
    }

    window->last_moved_by_interface = YES;
}

/*---------------------------------------------------------------------------*/

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize
{
    OSXWindow *window = (OSXWindow*)sender;
    if (self->OnResize != NULL && window->launch_resize_event == YES)
    {
        NSRect frame, content_frame;
        EvSize params;
        EvSize result;
        frame = NSMakeRect(0.f, 0.f, frameSize.width, frameSize.height);
        content_frame = [window contentRectForFrameRect:frame];
        params.width = (real32_t)content_frame.size.width;
        params.height = (real32_t)content_frame.size.height;

        // Called whenever graphics state updated (such as window resize)
        // OpenGL rendering is not synchronous with other rendering on the OSX.
        // Therefore, call disableScreenUpdatesUntilFlush so the window server
        // doesn't render non-OpenGL content in the window asynchronously from
        // OpenGL content, which could cause flickering.  (non-OpenGL content
        // includes the title bar and drawing done by the app with other APIs)
        [window disableScreenUpdatesUntilFlush];

        listener_event(self->OnResize, ekGUI_EVENT_WND_SIZING, (OSWindow*)window, &params, &result, OSWindow, EvSize, EvSize);
        listener_event(self->OnResize, ekGUI_EVENT_WND_SIZE, (OSWindow*)window, &result, NULL, OSWindow, EvSize, void);
        frame = [window frameRectForContentRect:NSMakeRect(0.f, 0.f, (CGFloat)result.width, (CGFloat)result.height)];
        return frame.size;
    }
    else
    {
        return frameSize;
    }
}

/*---------------------------------------------------------------------------*/

static BOOL i_close(OSXWindowDelegate *delegate, OSXWindow *window, const gui_close_t close_origin)
{
    if (delegate->OnClose != NULL)
    {
        EvWinClose params;
        bool_t result = FALSE;
        params.origin = close_origin;
        listener_event(delegate->OnClose, ekGUI_EVENT_WND_CLOSE, (OSWindow*)window, &params, &result, OSWindow, EvWinClose, bool_t);
        return (BOOL)result;
    }
    else
    {
        return YES;
    }
}

/*---------------------------------------------------------------------------*/

- (BOOL)windowShouldClose:(id)sender
{
    return i_close(self, sender, ekGUI_CLOSE_BUTTON);
}

/*---------------------------------------------------------------------------*/

- (BOOL)isFlipped
{
    return YES;
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(NSResponder *view, const bool_t focus)
{
    if ([view isKindOfClass:[NSView class]])
	{
		if (_osview_is((NSView*)view) == TRUE)
    	{
        	_osview_OnFocus((NSView*)view, focus);
        }
    }
}

/*---------------------------------------------------------------------------*/

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    OSXWindow *window;
    window = [notification object];
    cassert_no_null(window);
    i_OnFocus([window firstResponder], TRUE);
}

/*---------------------------------------------------------------------------*/

- (void)windowDidResignKey:(NSNotification*)notification
{
    OSXWindow *window;
    window = [notification object];
    cassert_no_null(window);
    i_OnFocus([window firstResponder], FALSE);
}

@end

/*---------------------------------------------------------------------------*/

@implementation OSXWindow

- (void)dealloc
{
    [super dealloc];
    heap_auditor_delete("OSXWindow");
}

/*---------------------------------------------------------------------------*/

- (BOOL) canBecomeKeyWindow
{
    return self->can_become_key_window;
}

/*---------------------------------------------------------------------------*/

- (BOOL)acceptsFirstMouse:(NSEvent*)theEvent
{
    unref(theEvent);
    return YES;
}

/*---------------------------------------------------------------------------*/

-(void)recalculateKeyViewLoop
{
    /* Prevents automatic key view loop re-compute.
       It seems that [NSWindow setAutorecalculatesKeyViewLoop:NO] doesn't work properly
       Tested in OSX El Capitan */
}

/*---------------------------------------------------------------------------*/

/* ESC key */
-(void)cancelOperation:(id)sender
{
    unref(sender);
    if (self->flags & ekWINDOW_ESC)
    {
        if (i_close([self delegate], self, ekGUI_CLOSE_ESC) == YES)
            [self orderOut:nil];
    }
}

/*---------------------------------------------------------------------------*/

/* INTRO key */
-(void)keyDown:(NSEvent *)theEvent
{
    /* '231' Comes from OSXEdit intro keyDown */
    unsigned short code = (theEvent == (NSEvent*)231) ? 36 : [theEvent keyCode];
    if (code == 36 || code == 76)
    {
        if (self->defbutton != NULL)
        {
            if (_osbutton_OnIntro((NSResponder*)self->defbutton) == YES)
                return;
        }

        if (self->flags & ekWINDOW_RETURN)
        {
            if (i_close([self delegate], self, ekGUI_CLOSE_INTRO) == YES)
                [self orderOut:nil];
            return;
        }

        /*
        {
            NSResponder *resp = [self firstResponder];
            if (_osbutton_OnIntro(resp) == YES)
                return;
        }
        */
    }
    else
    {
        /* TODO: hotkeys */
        [super keyDown:theEvent];
    }
}

@end

/*---------------------------------------------------------------------------*/

static NSUInteger i_window_style_mask(const uint32_t flags)
{
    NSUInteger style_mask = 0;
#if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    if (flags & ekWINDOW_TITLE)
        style_mask |= NSWindowStyleMaskTitled;

    if (flags & ekWINDOW_MIN)
        style_mask |= NSWindowStyleMaskMiniaturizable;

    if (flags & ekWINDOW_CLOSE)
        style_mask |= NSWindowStyleMaskClosable;

    if (flags & ekWINDOW_RESIZE)
        style_mask |= NSWindowStyleMaskResizable;
#else
    if (flags & ekWINDOW_TITLE)
        style_mask |= NSTitledWindowMask;

    if (flags & ekWINDOW_MIN)
        style_mask |= NSMiniaturizableWindowMask;

    if (flags & ekWINDOW_CLOSE)
        style_mask |= NSClosableWindowMask;

    if (flags & ekWINDOW_RESIZE)
        style_mask |= NSResizableWindowMask;
#endif
    return style_mask;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_create(const uint32_t flags)
{
    NSUInteger stylemask = 0;
    OSXWindow *window = NULL;
    OSXWindowDelegate *delegate = NULL;
    stylemask = i_window_style_mask(flags);
    heap_auditor_add("OSXWindow");
    window = [[OSXWindow alloc] initWithContentRect:NSZeroRect styleMask:stylemask backing:NSBackingStoreBuffered defer:NO];
    [window setAutorecalculatesKeyViewLoop:NO];
    window->origin.x = 0.f;
    window->origin.y = 0.f;
    window->in_window_destroy = FALSE;
    window->last_moved_by_interface = YES;
    window->can_become_key_window = YES;
    window->launch_resize_event = YES;
    window->destroy_main_view = YES;
    window->keyboard_focus = nil;
    window->defbutton = NULL;
    heap_auditor_add("OSXWindowDelegate");
    delegate = [OSXWindowDelegate alloc];
    window->flags = flags;
    window->alpha = .5f;
    window->hotkeys = NULL;
    window->first_in_key_loop = nil;
    window->current_in_key_loop = nil;
    delegate->OnMoved = NULL;
    delegate->OnResize = NULL;
    delegate->OnClose = NULL;
    [window setDelegate:delegate];
    [window setAcceptsMouseMovedEvents:YES];
    [window setIsVisible:NO];
    #if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [window setMovable:YES];
    #endif
    [window setWorksWhenModal:YES];
    [window setLevel:0];
    /* [window setFloatingPanel:NO]; */
    [window setHidesOnDeactivate:NO];
    [window setReleasedWhenClosed:NO];
    /* [window setBecomesKeyOnlyIfNeeded:YES]; */
    cassert([window contentView] != nil);
    return (OSWindow*)window;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_managed(void *native_ptr)
{
    unref(native_ptr);
    cassert(FALSE);
    return NULL;
}

/*---------------------------------------------------------------------------*/

static void i_remove_hotkey(HotKey *hotkey)
{
    listener_destroy(&hotkey->listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_destroy(OSWindow **window)
{
    OSXWindow *windowp = NULL;
    OSXWindowDelegate *delegate = NULL;
    cassert_no_null(window);
    windowp = (OSXWindow*)*window;
    cassert_no_null(windowp);
    delegate = [windowp delegate];
    cassert_no_null(delegate);

    if (windowp->destroy_main_view == YES)
    {
        OSPanel *panel = (OSPanel*)[windowp contentView];
        if (panel != NULL)
        {
            oswindow_detach_panel(*window, panel);
            _ospanel_destroy(&panel);
        }
    }

    cassert([windowp contentView] == nil);
    arrst_destopt(&windowp->hotkeys, i_remove_hotkey, HotKey);
    listener_destroy(&delegate->OnMoved);
    listener_destroy(&delegate->OnResize);
    listener_destroy(&delegate->OnClose);
    [windowp setDelegate:nil];
    [delegate release];
    [windowp close];
    [windowp release];
    *window = NULL;
}

/*---------------------------------------------------------------------------*/

void oswindow_OnMoved(OSWindow *window, Listener *listener)
{
    OSXWindowDelegate *delegate;
    cassert_no_null(window);
    delegate = [(OSXWindow*)window delegate];
    cassert_no_null(delegate);
    listener_update(&delegate->OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnResize(OSWindow *window, Listener *listener)
{
    OSXWindowDelegate *delegate;
    cassert_no_null(window);
    delegate = [(OSXWindow*)window delegate];
    cassert_no_null(delegate);
    listener_update(&delegate->OnResize, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnClose(OSWindow *window, Listener *listener)
{
    OSXWindowDelegate *delegate;
    cassert_no_null(window);
    delegate = [(OSXWindow*)window delegate];
    cassert_no_null(delegate);
    listener_update(&delegate->OnClose, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_title(OSWindow *window, const char_t *text)
{
    NSString *str;
    cassert_no_null(window);
#if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    cassert(([(OSXWindow*)window styleMask] & NSWindowStyleMaskTitled) == NSWindowStyleMaskTitled);
#else
    cassert(([(OSXWindow*)window styleMask] & NSTitledWindowMask) == NSTitledWindowMask);
#endif
    cassert_no_null(text);
    str = [[NSString alloc] initWithUTF8String:(const char*)text];
    [(OSXWindow*)window setTitle:str];
    [str release];
}

/*---------------------------------------------------------------------------*/

void oswindow_edited(OSWindow *window, const bool_t is_edited)
{
    cassert_no_null(window);
    [(OSXWindow*)window setDocumentEdited:(BOOL)is_edited];
}

/*---------------------------------------------------------------------------*/

void oswindow_movable(OSWindow *window, const bool_t is_movable)
{
    cassert_no_null(window);
    #if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [(OSXWindow*)window setMovable:(BOOL)is_movable];
    #endif
    [(OSXWindow*)window setMovableByWindowBackground:(BOOL)is_movable];
}

/*---------------------------------------------------------------------------*/

void oswindow_z_order(OSWindow *window, OSWindow *below_window)
{
    NSInteger below_level = 0;
    cassert_no_null(window);
    cassert_no_null(below_window);
    below_level = [(OSXWindow*)below_window level];
    [(OSXWindow*)window setLevel:below_level + 10];
}

/*---------------------------------------------------------------------------*/

void oswindow_alpha(OSWindow *window, const real32_t alpha)
{
    cassert_no_null(window);
    cassert(alpha >= 0.f && alpha <= 1.f);
    /* cassert(((OSXWindow*)window)->skin == ekGUI_WINDOW_SKIN_ROUNDED); */
    cassert(FALSE);
    ((OSXWindow*)window)->alpha = (CGFloat)alpha;
}

/*---------------------------------------------------------------------------*/

void oswindow_enable_mouse_events(OSWindow *window, const bool_t enabled)
{
    cassert_no_null(window);
    [(OSXWindow*)window setIgnoresMouseEvents:!(BOOL)enabled];
}

/*---------------------------------------------------------------------------*/

void oswindow_hotkey(OSWindow *window, const vkey_t key, const uint32_t modifiers, Listener *listener)
{
    OSXWindow *windowp = (OSXWindow*)window;
    cassert_no_null(windowp);

    if (windowp->hotkeys == NULL && listener != NULL)
        windowp->hotkeys = arrst_create(HotKey);

    /* Update the hotkey(if exists) */
    arrst_foreach(hotkey, windowp->hotkeys, HotKey)
        if (hotkey->key == key && hotkey->modifiers == modifiers)
        {
            listener_update(&hotkey->listener, listener);
            return;
        }
    arrst_end();

    /* Adds a new hotkey */
    if (listener != NULL)
    {
        HotKey *hotkey = arrst_new(windowp->hotkeys, HotKey);
        hotkey->key = key;
        hotkey->modifiers = modifiers;
        hotkey->listener = listener;
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_taborder(OSWindow *window, OSControl *control)
{
    OSXWindow *oswindow;
    NSView *oscontrol;
    cassert_no_null(window);
    cassert(control == NULL || [(NSObject*)control isKindOfClass:[NSView class]] == YES);
    oswindow = (OSXWindow*)window;
    oscontrol = (NSView*)control;

    if (oscontrol != nil)
    {
        bool_t with_tabstop = TRUE;

        //if ([(NSObject*)control isKindOfClass:[NSButton class]] == YES)
        //    with_tabstop = _osbutton_with_tabstop((NSButton*)control);

        if (with_tabstop == TRUE)
        {
            if (oswindow->first_in_key_loop == nil)
            {
                [[oswindow contentView] setNextKeyView:oscontrol];
                oswindow->first_in_key_loop = oscontrol;
            }

            if (oswindow->current_in_key_loop != NULL)
                [oswindow->current_in_key_loop setNextKeyView:oscontrol];

            [oscontrol setNextKeyView:oswindow->first_in_key_loop];
            oswindow->current_in_key_loop = oscontrol;
        }
    }
    else
    {
        oswindow->first_in_key_loop = nil;
        oswindow->current_in_key_loop = nil;
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_focus(OSWindow *window, OSControl *control)
{
    NSView *view = (NSView*)control;
    unref(window);
    [(OSXWindow*)window endEditingFor:nil];
    [(OSXWindow*)window makeFirstResponder:view];
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_panel(OSWindow *window, OSPanel *panel)
{
    cassert_no_null(window);
    cassert_no_null(panel);
    [(OSXWindow*)window setContentView:(NSView*)panel];
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_panel(OSWindow *window, OSPanel *panel)
{
    NSUInteger count = 0;
    cassert_no_null(window);
    cassert_no_null(panel);
    cassert([(OSXWindow*)window contentView] == (NSView*)panel);
    count = [(NSView*)panel retainCount];
    cassert(count > 0);
    ((OSXWindow*)window)->in_window_destroy = YES;
    [(OSXWindow*)window setContentView:nil/*[[NSView alloc] init]*/];
 /*   if (count == [(NSView*)main_view retainCount])
        [(NSView*)main_view release];
    cassert([(NSView*)main_view retainCount] == count - 1);*/
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_window(OSWindow *parent_window, OSWindow *child_window)
{
    cassert_no_null(parent_window);
    cassert_no_null(child_window);
    [(OSXWindow*)parent_window addChildWindow:(OSXWindow*)child_window ordered:NSWindowAbove];
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_window(OSWindow *parent_window, OSWindow *child_window)
{
    cassert_no_null(parent_window);
    cassert_no_null(child_window);
    [(OSXWindow*)parent_window removeChildWindow:(OSXWindow*)child_window];
}

/*---------------------------------------------------------------------------*/

void oswindow_launch(OSWindow *window, OSWindow *parent_window)
{
    OSXWindow *parent = NULL;
    cassert_no_null(window);
    if (parent_window != nil)
    {
        cassert([(NSObject*)parent_window isKindOfClass:[OSXWindow class]] == YES);
        parent = (OSXWindow*)parent_window;
    }
    else
    {
        parent = nil;/*(OSXWindow*)window;*/
    }

    [(OSXWindow*)window makeKeyAndOrderFront:(OSXWindow*)parent];
}

/*---------------------------------------------------------------------------*/

void oswindow_hide(OSWindow *window, OSWindow *parent_window)
{
    OSXWindow *parent = NULL;
    cassert_no_null(window);
    if (parent_window != nil)
    {
        cassert([(NSObject*)parent_window isKindOfClass:[OSXWindow class]] == YES);
        parent = (OSXWindow*)parent_window;
    }
    else
    {
        parent = (OSXWindow*)window;
    }

    [(OSXWindow*)window orderOut:parent];
}

/*---------------------------------------------------------------------------*/
/* https://forums.developer.apple.com/thread/88825 */
/*
 As of macOS 10.13, we are starting to see some crashes in our app related to
 launching a dialog during a windowDidBecomeMain, whether via [NSApp runModalForWindow:]
 or [NSAlert runModal].  This has worked up until 10.13 (in 10.12 we didn't crash but
 often there would be drawing anomolies like the window title bar not drawing, etc., and
 in 10.11/10.10 everything appeared to draw fine), but as we have been looking into it,
 we have noticed some messages in the console like this as well:
 -[NSAlert runModal] may not be invoked inside of transaction begin/commit pair, or inside
 of transaction commit (usually this means it was invoked inside of a view's -drawRect: method.)
 */
uint32_t oswindow_launch_modal(OSWindow *window, OSWindow *parent_window)
{
    OSXWindow *front_window = nil;
    NSInteger ret;
    cassert_no_null(window);

    if (parent_window != NULL)
    {
        front_window = (OSXWindow*)parent_window;
        front_window->keyboard_focus = [front_window firstResponder];
        [(OSXWindow*)parent_window setWorksWhenModal:NO];
    }
    else
    {
        front_window = (OSXWindow*)window;
    }

    [(OSXWindow*)window makeKeyAndOrderFront:(OSXWindow*)front_window];
    ret = [NSApp runModalForWindow:(OSXWindow*)window];

    if (parent_window != NULL)
    {
        [(OSXWindow*)parent_window makeFirstResponder:front_window->keyboard_focus];
        [(OSXWindow*)parent_window setWorksWhenModal:YES];
    }

    return (uint32_t)ret;
}

/*---------------------------------------------------------------------------*/

void oswindow_stop_modal(OSWindow *window, const uint32_t return_value)
{
    cassert_no_null(window);
    cassert([NSApp modalWindow] == (OSXWindow*)window);
    [(OSXWindow*)window close];
    [NSApp stopModalWithCode:(NSInteger)return_value];
}

/*---------------------------------------------------------------------------*/

//void oswindow_launch_sheet(OSWindow *window, OSWindow *parent)
//{
//    cassert_no_null(window);
//#if defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
//    cassert(FALSE);
//    [(OSXWindow*)parent beginSheet:(OSXWindow*)window completionHandler:nil];
//#else
//    [NSApp beginSheet:(OSXWindow*)window modalForWindow:(OSXWindow*)parent modalDelegate:nil didEndSelector:nil contextInfo:nil];
//#endif
//}

/*---------------------------------------------------------------------------*/

//void oswindow_stop_sheet(OSWindow *window, OSWindow *parent)
//{
//    cassert_no_null(window);
//    [(OSXWindow*)window orderOut:(OSXWindow*)window];
//    [NSApp endSheet:(OSXWindow*)window];
//    [(OSXWindow*)parent makeKeyAndOrderFront:nil];
//}

/*---------------------------------------------------------------------------*/

void oswindow_get_origin(const OSWindow *window, real32_t *x, real32_t *y)
{
    NSRect frame;
    CGFloat origin_x, origin_y;
    cassert_no_null(window);
    cassert_no_null(x);
    cassert_no_null(y);
    frame = [(OSXWindow*)window frame];
    _oscontrol_origin_in_screen_coordinates(&frame, &origin_x, &origin_y);
    *x = (real32_t)origin_x;
    *y = (real32_t)origin_y;
}

/*---------------------------------------------------------------------------*/

void oswindow_origin(OSWindow *window, const real32_t x, const real32_t y)
{
    NSRect window_frame;
    NSPoint origin;
    cassert_no_null(window);
    window_frame = [(OSXWindow*)window frame];
    window_frame.origin.x = (CGFloat)x;
    window_frame.origin.y = (CGFloat)y;
    _oscontrol_origin_in_screen_coordinates(&window_frame, &origin.x, &origin.y);
    [(OSXWindow*)window setFrameOrigin:origin];
}

/*---------------------------------------------------------------------------*/

void oswindow_get_size(const OSWindow *window, real32_t *width, real32_t *height)
{
    NSSize frame_size;
    cassert_no_null(window);
    cassert_no_null(width);
    cassert_no_null(height);
    frame_size = [(OSXWindow*)window frame].size;
    *width = (real32_t)frame_size.width;
    *height = (real32_t)frame_size.height;
}

/*---------------------------------------------------------------------------*/

void oswindow_size(OSWindow *window, const real32_t content_width, const real32_t content_height)
{
    NSSize size;
    real32_t x, y;
    cassert_no_null(window);
    size.width = (CGFloat)content_width;
    size.height = (CGFloat)content_height;
    oswindow_get_origin(window, &x, &y);
    ((OSXWindow*)window)->launch_resize_event = NO;
    [(OSXWindow*)window setContentSize:size];
    ((OSXWindow*)window)->launch_resize_event = YES;
    oswindow_origin(window, x, y);
}

/*---------------------------------------------------------------------------*/

void oswindow_set_default_pushbutton(OSWindow *window, OSButton *button)
{
    cassert_no_null(window);
    ((OSXWindow*)window)->defbutton = button;
    [(NSButton*)button setKeyEquivalent:@"\r"];
}

/*---------------------------------------------------------------------------*/

void oswindow_set_cursor(OSWindow *window, Cursor *cursor)
{
    unref(window);
    [(NSCursor*)cursor set];
}

/*---------------------------------------------------------------------------*/

void oswindow_property(OSWindow *window, const gui_prop_t property, const void *value)
{
    cassert_no_null(window);
    unref(value);
    switch (property)
    {
        case ekGUI_PROP_RESIZE:
            break;
        case ekGUI_PROP_CHILDREN:
            ((OSXWindow*)window)->destroy_main_view = NO;
            break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

BOOL _oswindow_in_destroy(NSWindow *window)
{
    cassert([window isKindOfClass:[OSXWindow class]] == YES);
    return ((OSXWindow*)window)->in_window_destroy;
}

/*---------------------------------------------------------------------------*/

NSView *_oswindow_main_view(OSWindow *window)
{
    cassert_no_null(window);
    return [(OSXWindow*)window contentView];
}

