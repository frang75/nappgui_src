/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.m
 *
 */

/* Cocoa NSTextView wrapper */

#include "osgui_osx.inl"
#include "ostext.h"
#include "ostext.inl"
#include "osgui.inl"
#include "oscontrol.inl"
#include "ospanel.inl"
#include "cassert.h"
#include "event.h"
#include "font.h"
#include "font.inl"
#include "heap.h"
#include "ptr.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXTextView : NSTextView 
{
    @public
    Font *font;
    NSScrollView *scroll;
    BOOL is_editable;
    BOOL is_opaque;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXTextView

- (void) dealloc
{
    [super dealloc];
}

/*---------------------------------------------------------------------------*/

- (void)drawRect:(NSRect)rect
{
    [super drawRect:rect];

    /*
    Feature #102: Retomar cuando tengamos las NSTableView.
    if ([[self window] firstResponder] == self->scroll)
    {
        NSRect scroll_rect = NSInsetRect([self->scroll documentVisibleRect], 4, 4);
        scroll_rect.origin.x -= 1.f;
        scroll_rect.origin.y -= 1.f;
        scroll_rect.size.width += 0.f;

        [NSGraphicsContext saveGraphicsState];
        NSSetFocusRingStyle(NSFocusRingOnly);
        [[NSBezierPath bezierPathWithRect:rect] fill];
        [NSGraphicsContext restoreGraphicsState];
    }
    */
}

@end

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    @interface OSXTextViewDelegate : NSObject <NSTextViewDelegate>
#else
    @interface OSXTextViewDelegate : NSObject
#endif
{
@public
    Listener *OnTextChange_listener;
}

@end

@implementation OSXTextViewDelegate

/*---------------------------------------------------------------------------*/

- (void) dealloc
{
    [super dealloc];
    heap_auditor_delete("OSXTextView");
}

/*---------------------------------------------------------------------------*/

- (void) textDidChange:(NSNotification*)notification
{
    OSXTextView *view = nil;
    cassert_no_null(notification);
    view = [notification object];
    cassert_no_null(view);
    if (self->OnTextChange_listener != NULL)
    {
        EvText params;
        params.text = NULL;
        params.cpos = 0;
        listener_event(self->OnTextChange_listener, ekEVTXTCHANGE, (OSText*)self, &params, NULL, OSText, EvText, void);
    }
}

@end

/*---------------------------------------------------------------------------*/

OSText *ostext_create(const tview_flag_t flags)
{
    OSXTextView *view = [[OSXTextView alloc] initWithFrame:NSZeroRect];
    unref(flags);
    heap_auditor_add("OSXTextView");
    view->is_editable = NO;
    view->is_opaque = YES;
    view->font = font_monospace(13.f, 0);
    view->scroll = [[NSScrollView alloc] initWithFrame:NSZeroRect];
    [view->scroll setDocumentView:view];
    [view->scroll setHasVerticalScroller:YES];
    [view->scroll setHasHorizontalScroller:YES];
    [view->scroll setAutohidesScrollers:YES];
    [view->scroll setBorderType:NSBezelBorder];
    [view->scroll setHidden:YES];
    [view setEditable:view->is_editable];
    [view setFont:font_native(view->font)];
    [view setRichText:NO];

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
    [view setAutomaticTextReplacementEnabled:NO];
    [view setAutomaticDashSubstitutionEnabled:NO];
    [view setAutomaticQuoteSubstitutionEnabled:NO];
#endif

    {
        OSXTextViewDelegate *delegate = [[OSXTextViewDelegate alloc] init];
        delegate->OnTextChange_listener = NULL;
        [view setDelegate:delegate];
    }
    
    /*    [view->scroll setBorderType:NSLineBorder];*/
    return (OSText*)view->scroll;
}

/*---------------------------------------------------------------------------*/

void ostext_destroy(OSText **view)
{
    OSXTextView *lview = nil;
    OSXTextViewDelegate *delegate = nil;
    cassert_no_null(view);
    lview = [(NSScrollView*)*view documentView];
    cassert_no_null(lview);
    delegate = [lview delegate];
    cassert_no_null(delegate);
    listener_destroy(&delegate->OnTextChange_listener);
    [lview setDelegate:nil];
    font_destroy(&lview->font);
    [delegate release];
    [lview release];    
    [(*(NSScrollView**)view) release];
}

/*---------------------------------------------------------------------------*/

static __INLINE OSXTextViewDelegate *i_get_delegate(OSText *view)
{
    cassert_no_null(view);
    return [(OSXTextView*)[(NSScrollView*)view documentView] delegate];
}

/*---------------------------------------------------------------------------*/

void ostext_OnTextChange(OSText *view, Listener *listener)
{
    OSXTextViewDelegate *delegate = i_get_delegate(view);
    cassert_no_null(delegate);
    listener_update(&delegate->OnTextChange_listener, listener);    
}

/*---------------------------------------------------------------------------*/
/* #pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Weverything"
 */
void ostext_insert_text(OSText *view, const char_t *text)
{
    OSXTextView *lview = nil;
    //NSDictionary *dict = nil;
    NSString *str = nil;
    NSRange end_range;

    //NSAttributedString *astr = nil;
    cassert_no_null(view);
    cassert_no_null(text);
    lview = [(NSScrollView*)view documentView];
    cassert_no_null(lview);
    end_range = NSMakeRange([[lview string] length], 0);
    //dict = _oscontrol_text_attribs(ekLEFT, UINT32_MAX, lview->font);
    str = [NSString stringWithUTF8String:(const char*)text];
    //astr = [[NSAttributedString alloc] initWithString:str attributes:dict];
    //[dict release]; // Autorelease
//    [[lview textStorage] appendAttributedString:astr];
//    [[lview textStorage] appendAttributedString:astr];
//    [[lview textStorage] appendString:str];
    //[lview insertText:str];

#if defined(MAC_OS_X_VERSION_10_11) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_11
    if (lview->is_editable == YES)
    {
        [lview insertText:str replacementRange:end_range];
    }
    else
    {
        [lview setEditable:YES];
        [lview insertText:str replacementRange:end_range];
        [lview setEditable:NO];
    }
#else
    if (lview->is_editable == YES)
    {
        [lview insertText:str];
    }
    else
    {
        [lview setEditable:YES];
        [lview insertText:str];
        [lview setEditable:NO];
    }
#endif
    
    [lview scrollRangeToVisible:end_range];
    //[astr release];
}

//#pragma GCC diagnostic pop

/*---------------------------------------------------------------------------*/

void ostext_set_text(OSText *view, const char_t *text)
{
    OSXTextView *lview;
    NSString *str;
    cassert_no_null(view);
    lview = [(NSScrollView*)view documentView];
    cassert_no_null(lview);
    str = [[NSString alloc] initWithUTF8String:(const char*)text];
    if (lview->is_editable == YES)
    {
        [lview setString:str];
    }
    else
    {
        [lview setEditable:YES];
        [lview setString:str];
        [lview setEditable:NO];
    }

    [str release];
}

/*---------------------------------------------------------------------------*/

void ostext_set_rtf(OSText *view, Stream *rtf_in)
{
    unref(view);
    unref(rtf_in);
}

/*---------------------------------------------------------------------------*/

void ostext_param(OSText *view, const guiprop_t param, const void *value)
{
    unref(view);
    unref(param);
    unref(value);
}

/*---------------------------------------------------------------------------*/

const char_t *ostext_get_text(const OSText *view)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView*)view documentView];
    cassert_no_null(lview);
    return (const char_t*)[[[lview textStorage] string] UTF8String];
}

/*---------------------------------------------------------------------------*/

void ostext_editable(OSText *view, const bool_t is_editable)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView*)view documentView];
    cassert_no_null(lview);
    lview->is_editable = (BOOL)is_editable;
    [lview setEditable:lview->is_editable];
}

/*---------------------------------------------------------------------------*/

//void ostext_set_font_size(OSText *view, const real32_t size)
//{
//    OSXTextView *lview;
//    cassert_no_null(view);
//    lview = [(NSScrollView*)view documentView];
//    cassert_no_null(lview);
//    [lview setFont:[NSFont fontWithName:@"Courier New" size:(CGFloat)size]];
//}

/*---------------------------------------------------------------------------*/

void ostext_set_need_display(OSText *view)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView*)view documentView];
    cassert_no_null(lview);
    [lview setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

void ostext_attach(OSText *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, (NSView*)view);
}

/*---------------------------------------------------------------------------*/

void ostext_detach(OSText *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (NSView*)view);
}

/*---------------------------------------------------------------------------*/

void ostext_visible(OSText *view, const bool_t is_visible)
{
    _oscontrol_set_visible((NSView*)view, is_visible);
}

/*---------------------------------------------------------------------------*/

void ostext_enabled(OSText *view, const bool_t is_enabled)
{
    unref(view);
    unref(is_enabled);
}

/*---------------------------------------------------------------------------*/

void ostext_size(const OSText *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((NSView*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void ostext_origin(const OSText *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((NSView*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void ostext_frame(OSText *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXTextView *lview;
    NSRect view_rect;
    lview = [(NSScrollView*)view documentView];
    _oscontrol_set_frame((NSView*)view, x, y, width, height);
    view_rect = [(NSScrollView*)view documentVisibleRect];
    /* view_rect.origin.y += 2.f;
       view_rect.size.width -= 5.f; */
    [lview setFrame:view_rect];    
}

/*---------------------------------------------------------------------------*/

BOOL _ostext_is(NSView *view)
{
    if ([view isKindOfClass:[NSScrollView class]])
        return [[(NSScrollView*)view documentView] isKindOfClass:[OSXTextView class]];
    return FALSE;
}

/*---------------------------------------------------------------------------*/

void _ostext_detach_and_destroy(OSText **view, OSPanel *panel)
{
    cassert_no_null(view);
    ostext_detach(*view, panel);
    ostext_destroy(view);
}


