/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.m
 *
 */

/* Cocoa CustomView */

#include "osgui_osx.inl"
#include "osview.h"
#include "osview.inl"
#include "osgui.inl"
#include "oscontrol.inl"
#include "oslistener.inl"
#include "ospanel.inl"
#include "cassert.h"
#include "dctxh.h"
#include "event.h"
#include "heap.h"
#include "ptr.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXView : NSView
{
@public
    NSScrollView *scroll;
    DCtx *ctx;
    uint32_t flags;
    NSTrackingArea *tracking_area;
    ViewListeners listeners;
    Listener *OnFocus;
    Listener *OnNotify;
    Listener *OnOverlay;
    BOOL mouse_inside;
}
@end

/*---------------------------------------------------------------------------*/

static CGContextRef i_CGContext(NSGraphicsContext *nscontext)
{
#if defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
    return [nscontext CGContext];
#else
    return (CGContextRef)[nscontext graphicsPort];
#endif
}

/*---------------------------------------------------------------------------*/

@implementation OSXView

/*---------------------------------------------------------------------------*/

static void i_area_expand(EvDraw *params, const real32_t area_width, const real32_t area_height)
{
    static const real32_t i_EXPAND_MARGIN = 300;

    params->x -= i_EXPAND_MARGIN;
    params->y -= i_EXPAND_MARGIN;
    params->width += 2 * i_EXPAND_MARGIN;
    params->height += 2 * i_EXPAND_MARGIN;

    if (params->x < 0)
        params->x = 0;

    if (params->y < 0)
        params->y = 0;

    if (params->x + params->width > area_width)
        params->width = area_width - params->x;

    if (params->y + params->height > area_height)
        params->height = area_height - params->y;
}

/*---------------------------------------------------------------------------*/

- (void) drawRect:(NSRect)rect
{
    if (self->listeners.OnDraw != NULL)
    {
        EvDraw params;
        params.ctx = NULL;

        if (self->scroll != nil)
        {
            NSRect vrect = [self->scroll documentVisibleRect];
            rect = [self frame];
            params.x = (real32_t)vrect.origin.x;
            params.y = (real32_t)vrect.origin.y;
            params.width = (real32_t)vrect.size.width;
            params.height = (real32_t)vrect.size.height;
            i_area_expand(&params, (real32_t)rect.size.width, (real32_t)rect.size.height);
        }
        else
        {
            rect = [self frame];
            params.x = 0;
            params.y = 0;
            params.width = (real32_t)rect.size.width;
            params.height = (real32_t)rect.size.height;
        }

        if ((self->flags & ekVIEW_OPENGL) == 0)
        {
            NSGraphicsContext *nscontext;
            CGContextRef cgcontext;

            if (self->ctx == NULL)
            {
                self->ctx = dctx_create();
                dctx_set_flipped(self->ctx, (bool_t)[self isFlipped]);
            }

            params.ctx = self->ctx;
            nscontext = [NSGraphicsContext currentContext];
            cgcontext = i_CGContext(nscontext);
            dctx_set_gcontext(self->ctx, nscontext, (uint32_t)rect.size.width, (uint32_t)rect.size.height, 0, 0, 0, TRUE);
            listener_event(self->listeners.OnDraw, ekGUI_EVENT_DRAW, (OSView*)self, &params, NULL, OSView, EvDraw, void);
            dctx_unset_gcontext(self->ctx);
        }
        else
        {
            listener_event(self->listeners.OnDraw, ekGUI_EVENT_DRAW, (OSView*)self, &params, NULL, OSView, EvDraw, void);
        }
    }
}

/*---------------------------------------------------------------------------*/

-(BOOL)acceptsFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

-(BOOL)becomeFirstResponder
{
    if (self->listeners.is_enabled == YES && self->OnFocus != NULL)
    {
        bool_t params = TRUE;
        listener_event(self->OnFocus, ekGUI_EVENT_FOCUS, (OSView*)self, &params, NULL, OSView, bool_t, void);
        return YES;
    }

    return NO;
}

/*---------------------------------------------------------------------------*/

- (BOOL) resignFirstResponder
{
    if (self->listeners.is_enabled == YES && self->OnFocus != NULL)
    {
        bool_t params = FALSE;
        listener_event(self->OnFocus, ekGUI_EVENT_FOCUS, (OSView*)self, &params, NULL, OSView, bool_t, void);
    }

    return YES;
}

/*---------------------------------------------------------------------------*/

- (BOOL) mouseDownCanMoveWindow
{
    return NO;
}

/*---------------------------------------------------------------------------*/

//- (BOOL) acceptsFirstMouse:(NSEvent*)theEvent
//{
//    unref(theEvent);
//    return NO;
//}

/*---------------------------------------------------------------------------*/

- (void) mouseEntered:(NSEvent*)theEvent
{
    self->mouse_inside = YES;
    _oslistener_mouse_enter(self, theEvent, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) mouseExited:(NSEvent*)theEvent
{
    unref(theEvent);
    self->mouse_inside = NO;
    _oslistener_mouse_exit(self, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) mouseMoved:(NSEvent*)theEvent
{
    if (self->mouse_inside == YES)
        _oslistener_mouse_moved(self, theEvent, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) mouseDown:(NSEvent*)theEvent
{
    _oslistener_mouse_down(self, theEvent, ekGUI_MOUSE_LEFT, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) rightMouseDown:(NSEvent*)theEvent
{
    _oslistener_mouse_down(self, theEvent, ekGUI_MOUSE_RIGHT, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) otherMouseDown:(NSEvent*)theEvent
{
    _oslistener_mouse_down(self, theEvent, ekGUI_MOUSE_MIDDLE, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) mouseUp:(NSEvent*)theEvent
{
    _oslistener_mouse_up(self, theEvent, ekGUI_MOUSE_LEFT, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) rightMouseUp:(NSEvent*)theEvent
{
    _oslistener_mouse_up(self, theEvent, ekGUI_MOUSE_RIGHT, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) otherMouseUp:(NSEvent*)theEvent
{
    _oslistener_mouse_up(self, theEvent, ekGUI_MOUSE_MIDDLE, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) mouseDragged:(NSEvent*)theEvent
{
    _oslistener_mouse_dragged(self, theEvent, ekGUI_MOUSE_LEFT, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) rightMouseDragged:(NSEvent*)theEvent
{
    _oslistener_mouse_dragged(self, theEvent, ekGUI_MOUSE_RIGHT, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) otherMouseDragged:(NSEvent*)theEvent
{
    _oslistener_mouse_dragged(self, theEvent, ekGUI_MOUSE_MIDDLE, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) scrollWheel:(NSEvent*)theEvent
{
    if (self->scroll != nil)
        [self->scroll scrollWheel:theEvent];
    _oslistener_scroll_whell(self, theEvent, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) keyDown:(NSEvent*)theEvent
{
    // kVK_Tab
    if ([theEvent keyCode] == 0x30)
    {
        [super keyDown:theEvent];
        return;
    }

    _oslistener_key_down(self, theEvent, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) keyUp:(NSEvent*)theEvent
{
    _oslistener_key_up(self, theEvent, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (void) flagsChanged:(NSEvent*)theEvent
{
    _oslistener_key_flags_changed(self, theEvent, &self->listeners);
}

/*---------------------------------------------------------------------------*/

- (BOOL)isFlipped
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)NAppGUIOSX_setOpenGL
{
    self->flags |= ekVIEW_OPENGL;
}

/*---------------------------------------------------------------------------*/

- (void)NAppGUIOSX_unsetOpenGL
{
    self->flags &= (uint32_t)~ekVIEW_OPENGL;
}

@end

/*---------------------------------------------------------------------------*/

OSView *osview_create(const uint32_t flags)
{
    OSXView *view = nil;
    heap_auditor_add("OSXView");
    view = [[OSXView alloc] initWithFrame:NSZeroRect];
    view->flags = flags;
    view->ctx = NULL;
    view->tracking_area = NULL;
    view->OnNotify = NULL;
    view->OnFocus = NULL;
    view->OnOverlay = NULL;
    view->mouse_inside = NO;
    _oslistener_init(&view->listeners);

    if (flags & ekVIEW_HSCROLL || flags & ekVIEW_VSCROLL)
    {
        NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSZeroRect];
        [scroll setDrawsBackground:NO];
        [scroll setDocumentView:view];
        [scroll setHasHorizontalScroller:(flags & ekVIEW_HSCROLL) ? YES : NO];
        [scroll setHasVerticalScroller:(flags & ekVIEW_VSCROLL) ? YES : NO];
        [scroll setAutohidesScrollers:YES];
        [scroll setBorderType:(flags & ekVIEW_BORDER) ? NSGrooveBorder : NSNoBorder];
        view->scroll = scroll;
        return (OSView*)scroll;
    }
    else
    {
        view->scroll = nil;
        return (OSView*)view;
    }
}

/*---------------------------------------------------------------------------*/

static OSXView *i_get_view(const OSView *view)
{
    cassert_no_null(view);
    if([(NSView*)view isKindOfClass:[OSXView class]])
    {
        return (OSXView*)view;
    }
    else
    {
        NSScrollView *scroll = (NSScrollView*)view;
        return [scroll documentView];
    }
}

/*---------------------------------------------------------------------------*/

void osview_destroy(OSView **view)
{
    OSXView *cview;
    NSScrollView *scroll = nil;
    cassert_no_null(view);
    cview = i_get_view(*view);

    _oslistener_release(&cview->listeners);
    listener_destroy(&cview->OnNotify);
    listener_destroy(&cview->OnFocus);
    listener_destroy(&cview->OnOverlay);

    if (cview->tracking_area != nil)
    {
        [cview removeTrackingArea:cview->tracking_area];
        [cview->tracking_area release];
    }

    if (cview->ctx != NULL)
        dctx_destroy(&cview->ctx);

    scroll = cview->scroll;
    [cview release];

    if (scroll != nil)
        [scroll release];

    *view = NULL;
    heap_auditor_delete("OSXView");
}

/*---------------------------------------------------------------------------*/

void osview_OnDraw(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnDraw, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnOverlay(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->OnOverlay, listener);
}

/*---------------------------------------------------------------------------*/

static bool_t i_needs_tracking_area(OSXView *view)
{
    cassert_no_null(view);
    if (view->listeners.OnClick != NULL)
        return TRUE;
    if (view->listeners.OnMoved != NULL)
        return TRUE;
    if (view->listeners.OnEnter != NULL)
        return TRUE;
    if (view->listeners.OnExit != NULL)
        return TRUE;
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_update_tracking_area(OSXView *view)
{
    bool_t with_area = i_needs_tracking_area(view);
    NSUInteger options = NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways | NSTrackingInVisibleRect;
    if (view->tracking_area != nil && with_area == TRUE)
    {
        NSSize required_size = [view frame].size;
        NSSize current_size = [view->tracking_area rect].size;
        if (NSEqualSizes(required_size, current_size) == NO)
        {
            [view removeTrackingArea:view->tracking_area];
            [view->tracking_area release];
            view->tracking_area = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0.f, 0.f, required_size.width, required_size.height) options:(NSTrackingAreaOptions)options owner:view userInfo:nil];
            [view addTrackingArea:view->tracking_area];
            cassert([[view trackingAreas] count] == 1);
        }

        return;
    }

    if (view->tracking_area != nil)
    {
        [view removeTrackingArea:view->tracking_area];
        [view->tracking_area release];
        view->tracking_area = nil;
    }

    if (with_area == TRUE)
    {
        NSSize size = [view frame].size;
        view->tracking_area = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0.f, 0.f, size.width, size.height) options:(NSTrackingAreaOptions)options owner:view userInfo:nil];
        [view addTrackingArea:view->tracking_area];
        cassert([[view trackingAreas] count] == 1);
    }
}

/*---------------------------------------------------------------------------*/

void osview_OnEnter(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnEnter, listener);
    i_update_tracking_area(lview);
}

/*---------------------------------------------------------------------------*/

void osview_OnExit(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnExit, listener);
    i_update_tracking_area(lview);
}

/*---------------------------------------------------------------------------*/

void osview_OnMoved(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnMoved, listener);
    i_update_tracking_area(lview);
}

/*---------------------------------------------------------------------------*/

void osview_OnDown(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnDown, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnUp(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnUp, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnClick(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnClick, listener);
    i_update_tracking_area(lview);
}

/*---------------------------------------------------------------------------*/

void osview_OnDrag(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnDrag, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnWheel(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnWheel, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyDown(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnKeyDown, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyUp(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->listeners.OnKeyUp, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnFocus(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnNotify(OSView *view, Listener *listener)
{
    OSXView *lview = i_get_view(view);
    listener_update(&lview->OnNotify, listener);
}

/*---------------------------------------------------------------------------*/

//void osview_launch_OnMouseMovedEvent(OSView *view)
//{
//    NSPoint mouse_position;
//    cassert_no_null(view);
//#if defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
//    {
//        NSRect rect;
//        rect.origin = [NSEvent mouseLocation];
//        rect.size = NSMakeSize(0.f, 0.f);
//        rect = [[(OSXView*)view window] convertRectFromScreen:rect];
//        mouse_position = rect.origin;
//    }
//#else
//    mouse_position = [[(OSXView*)view window] convertScreenToBase:[NSEvent mouseLocation]];
//#endif
//    _oslistener_mouse_moved((OSXView*)view, &mouse_position, /*ekEVENT_SOURCE_SOFTWARE,*/0, &((OSXView*)view)->listeners);
//}

/*---------------------------------------------------------------------------*/

//void osview_set_event_source(OSView *view, const enum gui_event_t event_type, const /*enum event_source_t*/uint32_t event_source)
//{
//    cassert_no_null(view);
//    cassert([(OSXView*)view isKindOfClass:[OSXView class]] == YES);
//    switch(event_type)
//    {
//        case ekGUI_EVENT_MOUSE_MOVED:
//            ((OSXView*)view)->listeners.OnMouseMoved_event_source = event_source;
//            break;
//            cassert_default();
//    }
//}

/*---------------------------------------------------------------------------*/

void osview_scroll(OSView *view, const real32_t x, const real32_t y)
{
    // https://stackoverflow.com/questions/5834056/how-i-set-the-default-position-of-a-nsscroll-view
    NSScrollView *scroll = (NSScrollView*)view;
    NSPoint pointToScrollTo = NSMakePoint((CGFloat)x, (CGFloat)y);
    cassert([(NSView*)view isKindOfClass:[NSScrollView class]]);
    [[scroll contentView] scrollToPoint: pointToScrollTo];
    [scroll reflectScrolledClipView: [scroll contentView]];
}

/*---------------------------------------------------------------------------*/

void osview_scroll_get(const OSView *view, real32_t *x, real32_t *y)
{
    NSScrollView *scroll = (NSScrollView*)view;
    NSRect rect;
    cassert([(NSView*)view isKindOfClass:[NSScrollView class]]);
    cassert_no_null(x);
    cassert_no_null(y);
    rect = [scroll documentVisibleRect];
    *x = (real32_t)rect.origin.x;
    *y = (real32_t)rect.origin.y;
}

/*---------------------------------------------------------------------------*/

void osview_scroller_size(const OSView *view, real32_t *width, real32_t *height)
{
    NSScrollView *scroll = (NSScrollView*)view;
    cassert([(NSView*)view isKindOfClass:[NSScrollView class]]);

    if (width)
    {
        NSScroller *scroller = [scroll verticalScroller];
        *width = 0;
        if (scroller != nil)
		{
	#if defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
			if ([scroller scrollerStyle] == NSScrollerStyleLegacy)
	#endif
				*width = (real32_t)[scroller frame].size.width;
		}
    }

    if (height)
    {
        NSScroller *scroller = [scroll horizontalScroller];
        *height = 0;
        if (scroller != nil)
		{
	#if defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
			if ([scroller scrollerStyle] == NSScrollerStyleLegacy)
	#endif
				*height = (real32_t)[scroller frame].size.height;
		}
    }
}

/*---------------------------------------------------------------------------*/

void osview_content_size(OSView *view, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    OSXView *lview = i_get_view(view);
    cassert([(NSView*)view isKindOfClass:[NSScrollView class]]);
    cassert(lview == [(NSScrollView*)view documentView]);
    unref(line_width);
    unref(line_height);
    if (lview->tracking_area != nil)
    {
        [lview removeTrackingArea:lview->tracking_area];
        [lview->tracking_area release];
        lview->tracking_area = nil;
    }

    [lview setFrame:NSMakeRect(0, 0, (CGFloat)width, (CGFloat)height)];
    i_update_tracking_area(lview);
}

/*---------------------------------------------------------------------------*/

real32_t osview_scale_factor(const OSView *view)
{
#if defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
    OSXView *lview = i_get_view(view);
    NSWindow *window = [lview window];
    if (window != nil)
        return (real32_t)[window backingScaleFactor];

    return (real32_t)[[NSScreen mainScreen] backingScaleFactor];
#else
    unref(view);
    return 1;
#endif
}

/*---------------------------------------------------------------------------*/

void osview_set_need_display(OSView *view)
{
    OSXView *lview = i_get_view(view);
    [lview setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

void *osview_get_native_view(const OSView *view)
{
    return (void*)view;
}

/*---------------------------------------------------------------------------*/

void osview_attach(OSView *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, (NSView*)view);
}

/*---------------------------------------------------------------------------*/

void osview_detach(OSView *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (NSView*)view);
}

/*---------------------------------------------------------------------------*/

void osview_visible(OSView *view, const bool_t is_visible)
{
    _oscontrol_set_visible((NSView*)view, is_visible);
}

/*---------------------------------------------------------------------------*/

void osview_enabled(OSView *view, const bool_t is_enabled)
{
    OSXView *lview = i_get_view(view);
    _oslistener_set_enabled(&lview->listeners, is_enabled);
}

/*---------------------------------------------------------------------------*/

void osview_size(const OSView *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((NSView*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void osview_origin(const OSView *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((NSView*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void osview_frame(OSView *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXView *lview = i_get_view(view);
    _oscontrol_set_frame((NSView*)view, x, y, width, height);
    i_update_tracking_area(lview);
    [lview setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

BOOL _osview_is(NSView *view)
{
    return [view isKindOfClass:[OSXView class]] || [view isKindOfClass:[NSScrollView class]];
}

/*---------------------------------------------------------------------------*/

void _osview_OnFocus(NSView *view, const bool_t focus)
{
    OSXView *lview = (OSXView*)view;
    if (lview->listeners.is_enabled == YES && lview->OnFocus != NULL)
    {
        bool_t params = focus;
        listener_event(lview->OnFocus, ekGUI_EVENT_FOCUS, (OSView*)lview, &params, NULL, OSView, bool_t, void);
    }
}

/*---------------------------------------------------------------------------*/

void _osview_detach_and_destroy(OSView **view, OSPanel *panel)
{
    cassert_no_null(view);
    osview_detach(*view, panel);
    osview_destroy(view);
}




/*
// DRAG SUPPORT

- (IBAction)onImageDrag:(id)sender
{
    cassert(sender == self);
    unreferenced_release(sender);

    if ([self isEnabled] == YES && self->OnImageDragged.object != NULL)
    {
        cassert(FALSE);
                Event event;
         event.sender = self;
         event.type = ekEVENT_TYPE_CONTROL_INCOMMING_DRAG;
         event.data.cdrag.obj = (const void*)[self image];
         listener_event(&self->OnImageDragged, &event);
    }
}


- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    if ([self isEnabled] == YES && self->accepts_drag == YES && self->OnAccepsDraggedImage.object != NULL)
    {
        NSPasteboard *paste;
        NSString *desired_type;
        NSData *carried_data;

        paste = [sender draggingPasteboard];

        {
            NSArray *types;

            types = [NSArray arrayWithObjects:NSTIFFPboardType, NSFilenamesPboardType, nil];
            desired_type = [paste availableTypeFromArray:types];
        }

        carried_data = [paste dataForType:desired_type];

        if (carried_data != nil)
        {
            NSImage *image;

            if ([desired_type isEqualToString:NSTIFFPboardType])
            {
                image = [[NSImage alloc] initWithData:carried_data];
            }
            else if ([desired_type isEqualToString:NSFilenamesPboardType])
            {
                NSArray *file_array;
                NSString *path;

                file_array = [paste propertyListForType:@"NSFilenamesPboardType"];
                path = [file_array objectAtIndex:0];
                image = [[NSImage alloc] initWithContentsOfFile:path];
            }
            else
            {
                image = nil;
            }

            if (image != nil)
            {
                //                Event event;
                BOOL accepts;

                cassert(FALSE);

                 event.sender = self;
                 event.type = ekEVENT_TYPE_CONTROL_ACCEPTS_DRAG;
                 event.data.cdrag.obj = (const void*)image;
                 listener_event(&self->OnAccepsDraggedImage, &event);

                 if (event.ret.rbool == TRUE)
                 {
                 [self setImage:image];
                 [self setNeedsDisplay:YES];
                 accepts = YES;
                 }
                 else
                 {
                 accepts = NO;
                 }

                [image release];
                return accepts;
            }
            else
            {
                return NO;
            }
        }
        else
        {
            return NO;
        }
    }
    else
    {
        return NO;
    }
}


OSImageView *osimageview_create(void)
{
    OSXImageView *view = NULL;
    _osgui_imageview_alloc(0);
    view = [[OSXImageView alloc] initWithFrame:NSZeroRect];
    [view setHidden:YES];
    view->accepts_drag = NO;
    listener_init_empty(&view->OnAccepsDraggedImage);
    listener_init_empty(&view->OnImageDragged);
    [view setImageFrameStyle:NSImageFrameNone];
    [view setImageAlignment:NSImageAlignCenter];
    [view setImageScaling:NSImageScaleProportionallyUpOrDown];
    [view setEditable:YES];
    [view setTarget:view];
    [view setAction:@selector(onImageDrag:)];
    [[view cell] setFocusRingType:NSFocusRingTypeNone];
    return (OSImageView*)view;
}

*/

