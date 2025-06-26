/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.m
 *
 */

/* Cocoa NSwindow wrapper */

#include "osbutton_osx.inl"
#include "oscontrol_osx.inl"
#include "osgui_osx.inl"
#include "ospanel_osx.inl"
#include "osview_osx.inl"
#include "oswindow_osx.inl"
#include "../oswindow.h"
#include "../oswindow.inl"
#include "../oscontrol.inl"
#include "../ostabstop.inl"
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXText : NSTextView
{
  @public
    uint32_t empty;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXWindow : NSPanel
{
  @public
    NSPoint origin;
    BOOL in_window_destroy;
    BOOL destroy_main_view;
    BOOL last_moved_by_interface;
    BOOL launch_resize_event;
    uint32_t flags;
    gui_role_t role;
    CGFloat alpha;
    OSTabStop tabstop;
    ArrSt(OSHotKey) *hotkeys;
    OSXText *text_editor;
    NSView *current_focus;
}
@end

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
@interface OSXWindowDelegate : NSObject < NSWindowDelegate >
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

@implementation OSXText

/*---------------------------------------------------------------------------*/

- (void)keyDown:(NSEvent *)theEvent
{
    if (_oswindow_key_down(cast(self, OSControl), theEvent) == FALSE)
        [super keyDown:theEvent];
}

/*---------------------------------------------------------------------------*/

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

- (void)windowDidMove:(NSNotification *)notification
{
    OSXWindow *window = [notification object];
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
        listener_event(self->OnMoved, ekGUI_EVENT_WND_MOVED, cast(window, OSWindow), &params, NULL, OSWindow, EvPos, void);
    }

    window->last_moved_by_interface = NO;
}

/*---------------------------------------------------------------------------*/

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    OSXWindow *window = cast(sender, OSXWindow);
    if (self->OnResize != NULL && window->launch_resize_event == YES)
    {
        NSRect frame, content_frame;
        EvSize params;
        EvSize result;
        frame = NSMakeRect(0.f, 0.f, frameSize.width, frameSize.height);
        content_frame = [window contentRectForFrameRect:frame];
        params.width = (real32_t)content_frame.size.width;
        params.height = (real32_t)content_frame.size.height;

        /* Called whenever graphics state updated (such as window resize)
         * OpenGL rendering is not synchronous with other rendering on the OSX.
         * Therefore, call disableScreenUpdatesUntilFlush so the window server
         * doesn't render non-OpenGL content in the window asynchronously from
         * OpenGL content, which could cause flickering.  (non-OpenGL content
         * includes the title bar and drawing done by the app with other APIs)
         */
#if defined(MAC_OS_VERSION_15_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_15
        /* 'disableScreenUpdatesUntilFlush' is deprecated: first deprecated in macOS 15.0
         * This method does not do anything and should not be called.
         */
#else
        [window disableScreenUpdatesUntilFlush];
#endif

        listener_event(self->OnResize, ekGUI_EVENT_WND_SIZING, cast(window, OSWindow), &params, &result, OSWindow, EvSize, EvSize);
        listener_event(self->OnResize, ekGUI_EVENT_WND_SIZE, cast(window, OSWindow), &result, NULL, OSWindow, EvSize, void);
        frame = [window frameRectForContentRect:NSMakeRect(0.f, 0.f, (CGFloat)result.width, (CGFloat)result.height)];
        return frame.size;
    }
    else
    {
        return frameSize;
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_close(OSXWindowDelegate *delegate, OSXWindow *window, const gui_close_t close_origin)
{
    bool_t closed = TRUE;
    cassert_no_null(window);
    cassert_no_null(delegate);

    /* Checks if the current control allows the window to be closed */
    if (close_origin == ekGUI_CLOSE_INTRO)
        closed = _ostabstop_can_close_window(&window->tabstop);

    /* Notify the user and check if allows the window to be closed */
    if (closed == TRUE && delegate->OnClose != NULL)
    {
        EvWinClose params;
        params.origin = close_origin;
        listener_event(delegate->OnClose, ekGUI_EVENT_WND_CLOSE, cast(window, OSWindow), &params, &closed, OSWindow, EvWinClose, bool_t);
    }

    return closed;
}

/*---------------------------------------------------------------------------*/

- (BOOL)windowShouldClose:(id)sender
{
    return (BOOL)i_close(self, sender, ekGUI_CLOSE_BUTTON);
}

/*---------------------------------------------------------------------------*/

- (BOOL)isFlipped
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)windowDidResignKey:(NSNotification *)notification
{
    OSXWindow *window = nil;
    cassert_no_null(notification);
    window = [notification object];
    cassert_no_null(window);
    if (window->role == ekGUI_ROLE_OVERLAY)
    {
        if (i_close(self, window, ekGUI_CLOSE_DEACT) == TRUE)
            window->role = ENUM_MAX(gui_role_t);
    }
}

/*---------------------------------------------------------------------------*/

- (id)windowWillReturnFieldEditor:(NSWindow *)sender toObject:(id)client
{
    /* https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/TextEditing/Tasks/FieldEditor.html */
    OSXWindow *window = cast(sender, OSXWindow);
    cassert([sender isKindOfClass:[OSXWindow class]]);
    if (window->text_editor == nil)
        window->text_editor = [[OSXText alloc] initWithFrame:NSMakeRect(0, 0, 10, 10)];

    [window->text_editor setDrawsBackground:YES];

    if ([client isKindOfClass:[NSTextField class]])
        [window->text_editor setFieldEditor:YES];
    else
        [window->text_editor setFieldEditor:NO];

    return window->text_editor;
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

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    unref(theEvent);
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)recalculateKeyViewLoop
{
    /* Prevents automatic key view loop re-compute.
       It seems that [NSWindow setAutorecalculatesKeyViewLoop:NO] doesn't work properly
       Tested in OSX El Capitan */
}

/*---------------------------------------------------------------------------*/

/* ESC key */
- (void)cancelOperation:(id)sender
{
    unref(sender);
    if (self->flags & ekWINDOW_ESC)
    {
        if (i_close([self delegate], self, ekGUI_CLOSE_ESC) == TRUE)
            [self orderOut:nil];
    }
}

/*---------------------------------------------------------------------------*/

- (BOOL)processKeyDown:(NSEvent *)theEvent
{
    /* '231' Comes from OSXEdit intro keyDown */
    unsigned short code = (theEvent == (NSEvent *)231) ? kVK_Return : [theEvent keyCode];

    if (code == kVK_Tab)
    {
        if (_ostabstop_capture_tab(&self->tabstop) == FALSE)
        {
            BOOL previous = NO;

#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
            NSEventModifierFlags flgs = [theEvent modifierFlags];
            previous = (flgs & NSEventModifierFlagShift) != 0;
#else
            NSUInteger flgs = [theEvent modifierFlags];
            previous = (flgs & NSShiftKeyMask) != 0;
#endif

            if (previous == YES)
                _ostabstop_prev(&self->tabstop, TRUE);
            else
                _ostabstop_next(&self->tabstop, TRUE);

            return YES;
        }
        else
        {
            return NO;
        }
    }

    else if (code == kVK_Return || code == kVK_ANSI_KeypadEnter)
    {
        if (_ostabstop_capture_return(&self->tabstop) == FALSE)
        {
            BOOL def = _osbutton_OnIntro(cast(self->tabstop.defbutton, NSResponder));

            if (self->flags & ekWINDOW_RETURN)
            {
                if (i_close([self delegate], self, ekGUI_CLOSE_INTRO) == TRUE)
                    [self orderOut:nil];

                return YES;
            }

            if (def == YES)
                return YES;
        }
        else
        {
            return NO;
        }
    }

    else if (code == kVK_Escape)
    {
        if (self->flags & ekWINDOW_ESC)
        {
            i_close([self delegate], self, ekGUI_CLOSE_ESC);
            return YES;
        }
    }

    if (self->hotkeys != NULL)
    {
        if (theEvent != (NSEvent *)231)
        {
            vkey_t vkey = _osgui_vkey([theEvent keyCode]);
            uint32_t modifiers = _osgui_modifiers((NSUInteger)[theEvent modifierFlags]);
            if (_oswindow_hotkey_process(cast(self, OSWindow), self->hotkeys, vkey, modifiers) == TRUE)
                return YES;
        }
    }

    return NO;
}

/*---------------------------------------------------------------------------*/

- (void)keyDown:(NSEvent *)theEvent
{
    if ([self processKeyDown:theEvent] == NO)
    {
        if (theEvent != cast(231, NSEvent))
            [super keyDown:theEvent];
    }
}

@end

/*---------------------------------------------------------------------------*/

static NSUInteger i_window_style_mask(const uint32_t flags)
{
    NSUInteger style_mask = 0;
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
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
    window->in_window_destroy = NO;
    window->destroy_main_view = YES;
    window->last_moved_by_interface = NO;
    window->launch_resize_event = YES;
    window->flags = flags;
    window->role = ENUM_MAX(gui_role_t);
    window->alpha = .5f;
    window->hotkeys = NULL;
    window->text_editor = nil;
    window->current_focus = nil;
    _ostabstop_init(&window->tabstop, cast(window, OSWindow));
    heap_auditor_add("OSXWindowDelegate");
    delegate = [OSXWindowDelegate alloc];
    delegate->OnMoved = NULL;
    delegate->OnResize = NULL;
    delegate->OnClose = NULL;
    [window setDelegate:delegate];
    [window setAcceptsMouseMovedEvents:YES];
    [window setIsVisible:NO];
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [window setMovable:YES];
#endif
    [window setWorksWhenModal:YES];
    [window setLevel:0];
    /* [window setFloatingPanel:NO]; */
    [window setHidesOnDeactivate:NO];
    [window setReleasedWhenClosed:NO];
    /* [window setBecomesKeyOnlyIfNeeded:YES]; */
    cassert([window contentView] != nil);
    return cast(window, OSWindow);
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_managed(void *native_ptr)
{
    unref(native_ptr);
    cassert(FALSE);
    return NULL;
}

/*---------------------------------------------------------------------------*/

void oswindow_destroy(OSWindow **window)
{
    OSXWindow *lwindow = nil;
    OSXWindowDelegate *delegate = nil;
    cassert_no_null(window);
    lwindow = *dcast(window, OSXWindow);
    cassert([cast(lwindow, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    cassert_no_null(lwindow);
    delegate = [lwindow delegate];
    cassert_no_null(delegate);

    if (lwindow->text_editor != nil)
        [lwindow->text_editor release];

    if (lwindow->destroy_main_view == YES)
    {
        OSPanel *panel = cast([lwindow contentView], OSPanel);
        if (panel != NULL)
        {
            oswindow_detach_panel(*window, panel);
            _ospanel_destroy(&panel);
        }
    }

    cassert([lwindow contentView] == nil);
    _ostabstop_remove(&lwindow->tabstop);
    _oswindow_hotkey_destroy(&lwindow->hotkeys);
    listener_destroy(&delegate->OnMoved);
    listener_destroy(&delegate->OnResize);
    listener_destroy(&delegate->OnClose);
    [lwindow setDelegate:nil];
    [delegate release];
    [lwindow close];
    [lwindow release];
    *window = NULL;
}

/*---------------------------------------------------------------------------*/

void oswindow_OnMoved(OSWindow *window, Listener *listener)
{
    OSXWindowDelegate *delegate = nil;
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    delegate = [cast(window, OSXWindow) delegate];
    cassert_no_null(delegate);
    listener_update(&delegate->OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnResize(OSWindow *window, Listener *listener)
{
    OSXWindowDelegate *delegate = nil;
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    delegate = [cast(window, OSXWindow) delegate];
    cassert_no_null(delegate);
    listener_update(&delegate->OnResize, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnClose(OSWindow *window, Listener *listener)
{
    OSXWindowDelegate *delegate = nil;
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    delegate = [cast(window, OSXWindow) delegate];
    cassert_no_null(delegate);
    listener_update(&delegate->OnClose, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_title(OSWindow *window, const char_t *text)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSString *str = nil;
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    cassert(([lwindow styleMask] & NSWindowStyleMaskTitled) == NSWindowStyleMaskTitled);
#else
    cassert(([lwindow styleMask] & NSTitledWindowMask) == NSTitledWindowMask);
#endif
    cassert_no_null(text);
    str = [[NSString alloc] initWithUTF8String:cast_const(text, char)];
    [lwindow setTitle:str];
    [str release];
}

/*---------------------------------------------------------------------------*/

void oswindow_edited(OSWindow *window, const bool_t is_edited)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    [lwindow setDocumentEdited:(BOOL)is_edited];
}

/*---------------------------------------------------------------------------*/

void oswindow_movable(OSWindow *window, const bool_t is_movable)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [lwindow setMovable:(BOOL)is_movable];
#endif
    [lwindow setMovableByWindowBackground:(BOOL)is_movable];
}

/*---------------------------------------------------------------------------*/

void oswindow_z_order(OSWindow *window, OSWindow *below_window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSInteger below_level = 0;
    cassert_no_null(lwindow);
    cassert_no_null(below_window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    below_level = [(OSXWindow *)below_window level];
    [lwindow setLevel:below_level + 10];
}

/*---------------------------------------------------------------------------*/

void oswindow_alpha(OSWindow *window, const real32_t alpha)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert(alpha >= 0.f && alpha <= 1.f);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    cassert(FALSE);
    lwindow->alpha = (CGFloat)alpha;
}

/*---------------------------------------------------------------------------*/

void oswindow_enable_mouse_events(OSWindow *window, const bool_t enabled)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    [lwindow setIgnoresMouseEvents:!(BOOL)enabled];
}

/*---------------------------------------------------------------------------*/

void oswindow_hotkey(OSWindow *window, const vkey_t key, const uint32_t modifiers, Listener *listener)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    _oswindow_hotkey_set(&lwindow->hotkeys, key, modifiers, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_taborder(OSWindow *window, OSControl *control)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    _ostabstop_list_add(&lwindow->tabstop, control);
    if (control == NULL)
    {
        /* The window main panel has changed. We ensure that default button is still valid */
        lwindow->tabstop.defbutton = _oswindow_apply_default_button(window, lwindow->tabstop.defbutton);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_tabcycle(OSWindow *window, const bool_t cycle)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    lwindow->tabstop.cycle = cycle;
}

/*---------------------------------------------------------------------------*/

gui_focus_t oswindow_tabstop(OSWindow *window, const bool_t next)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    if (next == TRUE)
        return _ostabstop_next(&lwindow->tabstop, FALSE);
    else
        return _ostabstop_prev(&lwindow->tabstop, FALSE);
}

/*---------------------------------------------------------------------------*/

gui_focus_t oswindow_focus(OSWindow *window, OSControl *control)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    return _ostabstop_move(&lwindow->tabstop, control);
}

/*---------------------------------------------------------------------------*/

OSControl *oswindow_get_focus(const OSWindow *window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    return lwindow->tabstop.current;
}

/*---------------------------------------------------------------------------*/

gui_tab_t oswindow_info_focus(const OSWindow *window, void **next_ctrl)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    return _ostabstop_info_focus(&lwindow->tabstop, next_ctrl);
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_panel(OSWindow *window, OSPanel *panel)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert_no_null(panel);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    [lwindow setContentView:cast(panel, NSView)];
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_panel(OSWindow *window, OSPanel *panel)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSView *lpanel = cast(panel, NSView);
    NSUInteger count = 0;
    cassert_no_null(lwindow);
    cassert_no_null(lpanel);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    cassert([lwindow contentView] == lpanel);
    count = [lpanel retainCount];
    cassert_unref(count > 0, count);
    lwindow->in_window_destroy = YES;
    [lwindow setContentView:nil /*[[NSView alloc] init]*/];
    /*   if (count == [(NSView*)main_view retainCount])
        [(NSView*)main_view release];
    cassert([(NSView*)main_view retainCount] == count - 1);*/
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_window(OSWindow *parent_window, OSWindow *child_window)
{
    cassert_no_null(parent_window);
    cassert_no_null(child_window);
    cassert([cast(parent_window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    cassert([cast(child_window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    [cast(parent_window, OSXWindow) addChildWindow:cast(child_window, OSXWindow) ordered:NSWindowAbove];
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_window(OSWindow *parent_window, OSWindow *child_window)
{
    cassert_no_null(parent_window);
    cassert_no_null(child_window);
    cassert([cast(parent_window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    cassert([cast(child_window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    [cast(parent_window, OSXWindow) removeChildWindow:cast(child_window, OSXWindow)];
}

/*---------------------------------------------------------------------------*/

void oswindow_launch(OSWindow *window, OSWindow *parent_window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    OSXWindow *lparent = cast(parent_window, OSXWindow);
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    if (lparent != nil)
    {
        cassert([cast(parent_window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
        lwindow->role = ekGUI_ROLE_OVERLAY;
    }
    else
    {
        lparent = nil; /*(OSXWindow*)window;*/
        lwindow->role = ekGUI_ROLE_MAIN;
    }

    _ostabstop_restore(&lwindow->tabstop);

    /* https://developer.apple.com/forums/thread/729496
     * I started seeing same warnings but they weren't there before.
     * Not sure if some behaviour on macOS changed or is just noise, but to on the safe side
     * I changed my implementation to open the first window on my app.
     * The key is to use orderFrontRegardless() instead of makeKeyAndOrderFront(nil)
     */
    if (lparent != nil)
        [lwindow makeKeyAndOrderFront:lparent];
    else
        [lwindow orderFrontRegardless];
}

/*---------------------------------------------------------------------------*/

void oswindow_hide(OSWindow *window, OSWindow *parent_window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    OSXWindow *lparent = nil;
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    if (parent_window != nil)
    {
        cassert([cast(parent_window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
        lparent = cast(parent_window, OSXWindow);
    }
    else
    {
        lparent = lwindow;
    }

    lwindow->role = ENUM_MAX(gui_role_t);
    [lwindow orderOut:lparent];
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
    OSXWindow *lwindow = cast(window, OSXWindow);
    OSXWindow *lparent = cast(parent_window, OSXWindow);
    OSXWindow *wfront = nil;
    NSInteger ret;
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);

    if (lparent != nil)
    {
        cassert([cast(parent_window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
        [lparent setWorksWhenModal:NO];
        wfront = lparent;
    }

    lwindow->role = ekGUI_ROLE_MODAL;

    /* https://developer.apple.com/forums/thread/729496
     * I started seeing same warnings but they weren't there before.
     * Not sure if some behaviour on macOS changed or is just noise, but to on the safe side
     * I changed my implementation to open the first window on my app.
     * The key is to use orderFrontRegardless() instead of makeKeyAndOrderFront(nil)
     */
    if (wfront != nil)
        [lwindow makeKeyAndOrderFront:nil];
    else
        [lwindow orderFrontRegardless];

    _ostabstop_restore(&lwindow->tabstop);
    ret = [NSApp runModalForWindow:lwindow];

    if (lparent != nil)
    {
        _ostabstop_restore(&lparent->tabstop);
        [lparent setWorksWhenModal:YES];
    }

    return (uint32_t)ret;
}

/*---------------------------------------------------------------------------*/

void oswindow_stop_modal(OSWindow *window, const uint32_t return_value)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    cassert([NSApp modalWindow] == lwindow);
    lwindow->role = ENUM_MAX(gui_role_t);
    [lwindow close];
    [NSApp stopModalWithCode:(NSInteger)return_value];
}

/*---------------------------------------------------------------------------*/

/*
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
*/

/*---------------------------------------------------------------------------*/

/*
//void oswindow_stop_sheet(OSWindow *window, OSWindow *parent)
//{
//    cassert_no_null(window);
//    [(OSXWindow*)window orderOut:(OSXWindow*)window];
//    [NSApp endSheet:(OSXWindow*)window];
//    [(OSXWindow*)parent makeKeyAndOrderFront:nil];
//}
*/

/*---------------------------------------------------------------------------*/

void oswindow_get_origin(const OSWindow *window, real32_t *x, real32_t *y)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert_no_null(x);
    cassert_no_null(y);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    if (*x == REAL32_MAX && *y == REAL32_MAX)
    {
        NSRect frame = [lwindow frame];
        CGFloat origin_x, origin_y;
        _oscontrol_origin_in_screen_coordinates(&frame, &origin_x, &origin_y);
        *x = (real32_t)origin_x;
        *y = (real32_t)origin_y;
    }
    else
    {
        NSSize size = [[lwindow contentView] frame].size;
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_6
        NSPoint pt = [lwindow convertRectToScreen:NSMakeRect((CGFloat)*x, size.height - (CGFloat)*y, 100, 100)].origin;
#else
        NSPoint pt = [lwindow convertBaseToScreen:NSMakePoint((CGFloat)*x, size.height - (CGFloat)*y)];
#endif

        NSSize ssize = [[NSScreen mainScreen] frame].size;
        *x = (real32_t)pt.x;
        *y = (real32_t)pt.y;
        *y = (real32_t)(ssize.height - pt.y);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_origin(OSWindow *window, const real32_t x, const real32_t y)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSRect window_frame;
    NSPoint origin;
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    window_frame = [lwindow frame];
    window_frame.origin.x = (CGFloat)x;
    window_frame.origin.y = (CGFloat)y;
    _oscontrol_origin_in_screen_coordinates(&window_frame, &origin.x, &origin.y);
    lwindow->last_moved_by_interface = YES;
    [(OSXWindow *)lwindow setFrameOrigin:origin];
}

/*---------------------------------------------------------------------------*/

void oswindow_get_size(const OSWindow *window, real32_t *width, real32_t *height)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSSize frame_size;
    cassert_no_null(lwindow);
    cassert_no_null(width);
    cassert_no_null(height);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    frame_size = [lwindow frame].size;
    *width = (real32_t)frame_size.width;
    *height = (real32_t)frame_size.height;
}

/*---------------------------------------------------------------------------*/

void oswindow_size(OSWindow *window, const real32_t content_width, const real32_t content_height)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSSize size;
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    size.width = (CGFloat)content_width;
    size.height = (CGFloat)content_height;
    lwindow->launch_resize_event = NO;
    [lwindow setContentSize:size];
    lwindow->launch_resize_event = YES;
}

/*---------------------------------------------------------------------------*/

void oswindow_set_default_pushbutton(OSWindow *window, OSButton *button)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    lwindow->tabstop.defbutton = _oswindow_apply_default_button(window, button);
}

/*---------------------------------------------------------------------------*/

void oswindow_set_cursor(OSWindow *window, Cursor *cursor)
{
    unref(window);
    if (cursor != nil)
        [cast(cursor, NSCursor) set];
    else
        [[NSCursor arrowCursor] set];
}

/*---------------------------------------------------------------------------*/

void oswindow_property(OSWindow *window, const gui_prop_t property, const void *value)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    unref(value);
    switch (property)
    {
    case ekGUI_PROP_CHILDREN:
        lwindow->destroy_main_view = NO;
        break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void _oswindow_widget_set_focus(OSWindow *window, OSWidget *widget)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSView *view = cast(widget, NSView);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    cassert([cast(widget, NSResponder) isKindOfClass:[NSView class]] == YES);
    if (lwindow->current_focus != view)
    {
        BOOL ok = [lwindow makeFirstResponder:view];
        cassert_unref(ok == YES, ok);

#if (defined MAC_OS_X_VERSION_10_6 && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6) || (defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
#else
        /* Clean rest of previous focus ring */
        if (lwindow->current_focus != nil)
        {
            if (_osbutton_is(lwindow->current_focus) || _osedit_is(lwindow->current_focus) || _ostext_is(lwindow->current_focus))
            {
                NSView *s = [lwindow->current_focus superview];
                NSView *ss = [s superview];
                [s setNeedsDisplay:YES];
                if (ss != nil)
                    [ss setNeedsDisplay:YES];
            }
        }
#endif

        lwindow->current_focus = view;
    }
}

/*---------------------------------------------------------------------------*/

static void i_get_controls(NSView *view, ArrPt(OSControl) *controls)
{
    OSControl *control = _oscontrol_from_nsview(view);
    if (control != NULL)
    {
        if (arrpt_find(controls, control, OSControl) == UINT32_MAX)
            arrpt_append(controls, control, OSControl);
    }

    {
        NSArray *children = [view subviews];
        NSUInteger i, count = [children count];
        for (i = 0; i < count; ++i)
        {
            NSView *child = cast([children objectAtIndex:i], NSView);
            i_get_controls(child, controls);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _oswindow_find_all_controls(OSWindow *window, ArrPt(OSControl) *controls)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    NSView *main_view = nil;
    cassert_no_null(lwindow);
    cassert(arrpt_size(controls, OSControl) == 0);
    main_view = [lwindow contentView];
    i_get_controls(main_view, controls);
}

/*---------------------------------------------------------------------------*/

const ArrPt(OSControl) *_oswindow_get_all_controls(const OSWindow *window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    return lwindow->tabstop.controls;
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_app(void *app, void *icon)
{
    cassert(FALSE);
    unref(app);
    unref(icon);
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_app_terminate(void)
{
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

BOOL _oswindow_in_destroy(NSWindow *window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    return lwindow->in_window_destroy;
}

/*---------------------------------------------------------------------------*/

NSView *_oswindow_main_view(OSWindow *window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(lwindow);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    return [lwindow contentView];
}

/*---------------------------------------------------------------------------*/

void _oswindow_next_tabstop(NSWindow *window, const bool_t keypress)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    _ostabstop_next(&lwindow->tabstop, keypress);
}

/*---------------------------------------------------------------------------*/

void _oswindow_prev_tabstop(NSWindow *window, const bool_t keypress)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    _ostabstop_prev(&lwindow->tabstop, keypress);
}

/*---------------------------------------------------------------------------*/

void _oswindow_restore_focus(NSWindow *window)
{
    OSXWindow *lwindow = cast(window, OSXWindow);
    cassert_no_null(window);
    cassert([cast(window, NSResponder) isKindOfClass:[OSXWindow class]] == YES);
    _ostabstop_restore(&lwindow->tabstop);
}

/*---------------------------------------------------------------------------*/

bool_t _oswindow_key_down(OSControl *control, NSEvent *theEvent)
{
    OSXWindow *window = nil;
    cassert_no_null(control);
    cassert([cast(control, NSResponder) isKindOfClass:[NSView class]] == YES);
    window = cast([cast(control, NSView) window], OSXWindow);
    cassert_no_null(window);
    return (bool_t)[window processKeyDown:theEvent];
}

/*---------------------------------------------------------------------------*/

bool_t _oswindow_mouse_down(OSControl *control)
{
    OSXWindow *window = nil;
    cassert_no_null(control);
    cassert([cast(control, NSResponder) isKindOfClass:[NSView class]] == YES);
    window = cast([cast(control, NSView) window], OSXWindow);
    cassert_no_null(window);
    return _ostabstop_mouse_down(&window->tabstop, control);
}

/*---------------------------------------------------------------------------*/

void _oswindow_release_transient_focus(OSControl *control)
{
    OSXWindow *window = nil;
    cassert_no_null(control);
    cassert([cast(control, NSResponder) isKindOfClass:[NSView class]] == YES);
    window = cast([cast(control, NSView) window], OSXWindow);
    cassert_no_null(window);
    _ostabstop_release_transient(&window->tabstop, control);
}
