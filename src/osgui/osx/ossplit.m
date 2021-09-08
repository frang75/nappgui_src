/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.m
 *
 */

/* Operating System split view */

#include "osgui_osx.inl"
#include "ossplit.h"
#include "osgui.inl"
#include "oscontrol.inl"
#include "oslistener.inl"
#include "ospanel.inl"
#include "cassert.h"
#include "event.h"
#include "event.inl"
#include "heap.h"
#include "ptr.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

@interface OSXSplitView : NSView 
{
    @public
    split_flag_t flags;
    NSTrackingArea *track_area;
    NSRect div_rect;
    ViewListeners listeners;
}

- (void) mouseDragged:(NSEvent*)theEvent;
- (void) mouseUp:(NSEvent*)theEvent;

@end

/*---------------------------------------------------------------------------*/

@implementation OSXSplitView

/*---------------------------------------------------------------------------*/

- (void) mouseMoved:(NSEvent*)event
{
    unref(event);
    cassert(FALSE);
    //if (self->listeners.is_dragging == NO)
    {
        NSPoint local_point;
        cassert_no_null(event);
        local_point = [self convertPoint:[event locationInWindow] fromView:nil];
        if (NSPointInRect(local_point, self->div_rect) == YES)
        {
            if (split_type(self->flags) == ekSPHORZ)
                [[NSCursor resizeUpDownCursor] set];
            else
                [[NSCursor resizeLeftRightCursor] set];
        }
        else
        {
            [[NSCursor arrowCursor] set];
        }
    }
}

/*---------------------------------------------------------------------------*/

- (NSView*)hitTest:(NSPoint)aPoint
{
    cassert(FALSE);
    //if (self->listeners.is_dragging == NO)
    {
        NSPoint local_point = [self convertPoint:aPoint fromView:[self superview]];
        if (NSPointInRect(local_point, self->div_rect) == YES)
        {
            return self;
        }
        else
        {
            NSArray *children = [self subviews];
            NSUInteger count = [children count];
            cassert(count == 1 || count == 2);
            NSView *child1 = (NSView*)[children objectAtIndex:0];
            NSView *hit1 = [child1 hitTest:aPoint];
            if (hit1 != nil)
            {
                return hit1;
            }
            else if (count == 2)
            {
                NSView *child2 = (NSView*)[children objectAtIndex:1];
                return [child2 hitTest:aPoint];
            }
            else
            {
                return nil;
            }
        }
    }
//    else
//    {
//        return nil;
//    }
}

/*---------------------------------------------------------------------------*/

- (void)mouseDragged:(NSEvent*)theEvent
{
    // Called whenever graphics state updated (such as window resize)	
    // OpenGL rendering is not synchronous with other rendering on the OSX.
    // Therefore, call disableScreenUpdatesUntilFlush so the window server
    // doesn't render non-OpenGL content in the window asynchronously from
    // OpenGL content, which could cause flickering.  (non-OpenGL content
    // includes the title bar and drawing done by the app with other APIs)
    NSWindow *window = [self window];
    if (window != nil)
        [window disableScreenUpdatesUntilFlush];
    
    //BOOL is_dragging = self->listeners.is_dragging;
    cassert(FALSE);
    BOOL is_dragging = NO;//self->listeners.is_dragging;
    _oslistener_mouse_dragged(self, theEvent, ekMLEFT, &self->listeners);
    
    if (is_dragging == NO)
    {
        [window disableCursorRects];
        if (split_type(self->flags) == ekSPHORZ)
            [[NSCursor resizeUpDownCursor] set];
        else
            [[NSCursor resizeLeftRightCursor] set];
    }
}

/*---------------------------------------------------------------------------*/

- (void)mouseUp:(NSEvent*)theEvent
{
    BOOL is_dragging = NO;// self->listeners.is_dragging;
    cassert(FALSE);
    _oslistener_mouse_up(self, theEvent, ekMLEFT, &self->listeners);
    if (is_dragging == YES)
    {
        NSWindow *window = [self window];
        [window enableCursorRects];
        [window resetCursorRects];
    }
}

/*---------------------------------------------------------------------------*/

- (BOOL)isFlipped
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)dealloc
{
    [super dealloc];
    heap_auditor_delete("OSXSplit");
}

@end

/*---------------------------------------------------------------------------*/

OSSplit *ossplit_create(const split_flag_t flags)
{
    OSXSplitView *view;
    heap_auditor_add("OSXSplit");
    view = [[OSXSplitView alloc] initWithFrame:NSZeroRect];
    view->flags = flags;
    view->track_area = [[NSTrackingArea alloc] initWithRect:NSZeroRect options:(NSTrackingAreaOptions)(NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways) owner:view userInfo:nil];
    view->div_rect = NSZeroRect;
    [view addTrackingArea:view->track_area];
    _oslistener_init(&view->listeners);
    [view setAutoresizesSubviews:NO];
    return (OSSplit*)view;
}

/*---------------------------------------------------------------------------*/

void ossplit_destroy(OSSplit **view)
{
    OSXSplitView *split;
    cassert_no_null(view);
    split = *(OSXSplitView**)view;
    cassert_no_null(split);
    cassert([[split subviews] count] == 0);
    _oslistener_release(&split->listeners);
    [split removeTrackingArea:split->track_area];
    [split->track_area release];
    [split release];
    *view = NULL;
}

/*---------------------------------------------------------------------------*/

#if defined (__ASSERTS__)
static BOOL i_exists_subview(OSXSplitView *view, NSView *subview)
{
    NSArray *subviews = [view subviews];
    NSUInteger count = [subviews count];
    cassert(count <= 2);
    for (NSUInteger i = 0; i < count; ++i)
    {
        if ([subviews objectAtIndex:i] == subview)
            return YES;
    }

    return NO;
}
#endif

/*---------------------------------------------------------------------------*/

void ossplit_attach_control(OSSplit *view, OSControl *control)
{
    cassert_no_null(view);
    cassert_no_null(control);
    cassert(i_exists_subview((OSXSplitView*)view, (NSView*)control) == NO);
    [(OSXSplitView*)view addSubview:(NSView*)control];
    cassert(i_exists_subview((OSXSplitView*)view, (NSView*)control) == YES);
}

/*---------------------------------------------------------------------------*/

void ossplit_detach_control(OSSplit *view, OSControl *control)
{
    cassert_no_null(view);
    cassert_no_null(control);
    cassert([(NSView*)control superview] == (OSXSplitView*)view);
    cassert(i_exists_subview((OSXSplitView*)view, (NSView*)control) == YES);
    [(NSView*)control removeFromSuperviewWithoutNeedingDisplay];
    cassert(i_exists_subview((OSXSplitView*)view, (NSView*)control) == NO);
}

/*---------------------------------------------------------------------------*/

void ossplit_OnMoved(OSSplit *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(listener);
    cassert(FALSE);
//    listener_update(&((OSXSplitView*)view)->listeners.OnMouseStartDrag, listener_copy(listener));
//    listener_update(&((OSXSplitView*)view)->listeners.OnMouseDragging, listener_copy(listener));
//    listener_update(&((OSXSplitView*)view)->listeners.OnMouseEndDrag, listener);
}

/*---------------------------------------------------------------------------*/

void ossplit_track_area(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXSplitView *split = (OSXSplitView*)view;
    cassert_no_null(split);
    split->div_rect = NSMakeRect((CGFloat)x, (CGFloat)y, (CGFloat)width, (CGFloat)height);
}

/*---------------------------------------------------------------------------*/

void ossplit_attach(OSSplit *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, (NSView*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_detach(OSSplit *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (NSView*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_visible(OSSplit *view, const bool_t is_visible)
{
    _oscontrol_set_visible((NSView*)view, is_visible);
}

/*---------------------------------------------------------------------------*/

void ossplit_enabled(OSSplit *view, const bool_t is_enabled)
{
    unref(view);
    unref(is_enabled);
}

/*---------------------------------------------------------------------------*/

void ossplit_size(const OSSplit *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((NSView*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void ossplit_origin(const OSSplit *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((NSView*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void ossplit_frame(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXSplitView *split = (OSXSplitView*)view;
    cassert_no_null(split);
    _oscontrol_set_frame(split, x, y, width, height);
    [split removeTrackingArea:split->track_area];
    [split->track_area release];
    split->track_area = [[NSTrackingArea alloc] initWithRect:NSMakeRect((CGFloat)x, (CGFloat)y, (CGFloat)width, (CGFloat)height) options:(NSTrackingAreaOptions)(NSTrackingMouseMoved | NSTrackingActiveInActiveApp) owner:split userInfo:nil];
    [split addTrackingArea:split->track_area];
}

