/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.m
 *
 */

/* Cocoa NSTextView wrapper */

#include "osgui_osx.inl"
#include "ostext.h"
#include "ostext_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "oswindow_osx.inl"
#include "osgui.inl"
#include "oscolor.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
#else
@interface OSXScrollView : NSScrollView
{
    uint32_t dummy;
}
@end
#endif

/*---------------------------------------------------------------------------*/

@interface OSXTextView : NSTextView
{
  @public
    char_t ffamily[64];
    real32_t fsize;
    uint32_t fstyle;
    align_t palign;
    real32_t pspacing;
    real32_t pafter;
    real32_t pbefore;
    NSScrollView *scroll;
    NSMutableDictionary *attribs;
    NSRange select;
    uint32_t num_chars;
    BOOL focused;
    BOOL focus_draw;
    BOOL is_editable;
    BOOL is_opaque;
    BOOL is_wrap_mode;
    BOOL show_select;
    Listener *OnFilter;
    Listener *OnFocus;
}
@end

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
#else
@implementation OSXScrollView

/* Draw focus ring in older mac OSX */
- (void)drawRect:(NSRect)rect
{
    OSXTextView *view = [self documentView];
    [super drawRect:rect];

    if (view->focused == YES)
    {
        /* Avoid draw focus ring over scrollbars */
        if (view->focus_draw == YES)
        {
            NSRect r = [self bounds];
            r.origin.x += 1;
            r.origin.y += 1;
            r.size.width -= 2;
            r.size.height -= 2;
            NSSetFocusRingStyle(NSFocusRingOnly);
            NSRectFill(r);
            view->focus_draw = NO;
        }
    }
}

@end
#endif

/*---------------------------------------------------------------------------*/

@implementation OSXTextView

- (void)dealloc
{
    [super dealloc];
}

/*---------------------------------------------------------------------------*/

- (void)keyDown:(NSEvent *)theEvent
{
    if (_oswindow_key_down(cast(self, OSControl), theEvent) == FALSE)
        [super keyDown:theEvent];
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    if (_oswindow_mouse_down((OSControl *)self->scroll) == TRUE)
        [super mouseDown:theEvent];
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

static uint32_t i_text_num_chars(OSXTextView *lview)
{
    const char_t *utf8 = NULL;
    cassert_no_null(lview);
    utf8 = cast_const([[[lview textStorage] string] UTF8String], char_t);
    return unicode_nchars(utf8, ekUTF8);
}

/*---------------------------------------------------------------------------*/

static char_t *i_get_seltext(OSXTextView *lview, NSRange range, uint32_t *size)
{
    const char_t *utf8 = NULL;
    const char_t *from = NULL, *to = NULL;
    char_t *text = NULL;
    cassert_no_null(lview);
    cassert_no_null(size);
    utf8 = cast_const([[[lview textStorage] string] UTF8String], char_t);
    from = unicode_move(utf8, (uint32_t)range.location, ekUTF8);
    to = unicode_move(from, (uint32_t)range.length, ekUTF8);
    cassert(to >= from);
    *size = (uint32_t)(to - from) + 1;
    text = cast(heap_malloc(*size, "OSTextSelText"), char_t);
    str_copy_cn(text, *size, from, *size - 1);
    return text;
}

/*---------------------------------------------------------------------------*/

static void i_replace_seltext(OSXTextView *lview, NSRange range, const char_t *text)
{
    NSString *str = [NSString stringWithUTF8String:(const char *)text];
    [[lview textStorage] replaceCharactersInRange:range withString:str];
}

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
@interface OSXTextViewDelegate : NSObject <NSTextViewDelegate>
#else
@interface OSXTextViewDelegate : NSObject
#endif
{
    uint32_t dummy;
}

@end

@implementation OSXTextViewDelegate

/*---------------------------------------------------------------------------*/

- (void)dealloc
{
    [super dealloc];
    heap_auditor_delete("OSXTextView");
}

/*---------------------------------------------------------------------------*/

- (void)textDidChange:(NSNotification *)notification
{
    OSXTextView *lview = nil;
    cassert_no_null(notification);
    lview = [notification object];
    cassert_no_null(lview);
    if (lview->OnFilter != NULL)
    {
        uint32_t num_chars = i_text_num_chars(lview);

        /* Event only if inserted text */
        if (num_chars > lview->num_chars)
        {
            char_t *edit_text = NULL;
            uint32_t tsize;
            uint32_t inschars = num_chars - lview->num_chars;
            EvText params;
            EvTextFilter result;
            NSUInteger location = [lview selectedRange].location;
            NSRange range = NSMakeRange(location - inschars, inschars);

            /* Select the inserted text */
            [lview setSelectedRange:range];

            edit_text = i_get_seltext(lview, range, &tsize);
            params.text = (const char_t *)edit_text;
            params.cpos = (uint32_t)range.location;
            params.len = (int32_t)inschars;
            result.apply = FALSE;
            result.text[0] = '\0';
            result.cpos = UINT32_MAX;
            listener_event(lview->OnFilter, ekGUI_EVENT_TXTFILTER, cast(lview, OSText), &params, &result, OSText, EvText, EvTextFilter);
            heap_free((byte_t **)&edit_text, tsize, "OSTextSelText");

            lview->num_chars = num_chars;

            if (result.apply == TRUE)
            {
                uint32_t replnchars = unicode_nchars(result.text, ekUTF8);

                /* Replace the previously selected (inserted) text */
                i_replace_seltext(lview, range, result.text);

                if (replnchars >= inschars)
                    lview->num_chars += replnchars - inschars;
                else
                    lview->num_chars -= inschars - replnchars;
            }
            else
            {
                /* Just unselect the previous selected text, remains the caret in its position */
                NSRange desel = NSMakeRange(location, 0);
                [lview setSelectedRange:desel];
            }
        }
        else
        {
            lview->num_chars = num_chars;
        }
    }
}

@end

/*---------------------------------------------------------------------------*/
/* https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/TextUILayer/Tasks/TextInScrollView.html#//apple_ref/doc/uid/20000938-CJBBIAAF */
static void i_set_wrap_mode(OSXTextView *view, const BOOL wrap)
{
    NSSize csize;
    cassert_no_null(view);
    cassert([view enclosingScrollView] == view->scroll);
    [view->scroll setHasVerticalScroller:YES];
    [view->scroll setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [view->scroll setAutohidesScrollers:YES];
    csize = [view->scroll contentSize];
    [view setFrameSize:csize];
    [view setVerticallyResizable:YES];
    [view setMinSize:NSMakeSize(0.0, csize.height)];
    [view setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];

    if (wrap == YES)
    {
        [view->scroll setHasHorizontalScroller:NO];
        [view setHorizontallyResizable:NO];
        [view setAutoresizingMask:NSViewWidthSizable];
        [[view textContainer] setContainerSize:NSMakeSize(csize.width, FLT_MAX)];
        [[view textContainer] setWidthTracksTextView:YES];
    }
    else
    {
        [view->scroll setHasHorizontalScroller:YES];
        [view setHorizontallyResizable:YES];
        [view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
        [[view textContainer] setContainerSize:NSMakeSize(FLT_MAX, FLT_MAX)];
        [[view textContainer] setWidthTracksTextView:NO];
    }
}

/*---------------------------------------------------------------------------*/

OSText *ostext_create(const uint32_t flags)
{
    OSXTextView *view = [[OSXTextView alloc] initWithFrame:NSZeroRect];
    unref(flags);
    heap_auditor_add("OSXTextView");
    view->is_editable = NO;
    view->is_opaque = YES;
    view->focused = NO;
    view->focus_draw = NO;
    view->is_wrap_mode = YES;
    view->show_select = NO;

#if defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
    view->scroll = [[NSScrollView alloc] initWithFrame:NSZeroRect];
#else
    view->scroll = [[OSXScrollView alloc] initWithFrame:NSZeroRect];
#endif
    view->attribs = [[NSMutableDictionary alloc] init];
    view->ffamily[0] = '\0';
    view->fsize = REAL32_MAX;
    view->fstyle = UINT32_MAX;
    view->palign = ENUM_MAX(align_t);
    view->pspacing = REAL32_MAX;
    view->pafter = REAL32_MAX;
    view->pbefore = REAL32_MAX;
    view->select = NSMakeRange(0, 0);
    view->num_chars = 0;
    view->OnFilter = NULL;
    view->OnFocus = NULL;
    [view->scroll setDocumentView:view];
    [view->scroll setBorderType:NSBezelBorder];
    [view->scroll setHidden:YES];
    [view setEditable:view->is_editable];
    [view setRichText:YES];
    i_set_wrap_mode(view, view->is_wrap_mode);

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
    [view setAutomaticTextReplacementEnabled:NO];
    [view setAutomaticDashSubstitutionEnabled:NO];
    [view setAutomaticQuoteSubstitutionEnabled:NO];
#endif

    {
        OSXTextViewDelegate *delegate = [[OSXTextViewDelegate alloc] init];
        [view setDelegate:delegate];
    }

    {
        NSColor *color = oscolor_NSColor(1); /* ekSYS_LABEL */
        [view->attribs setValue:color forKey:NSForegroundColorAttributeName];
    }

    /* OSText allways have border */
    {
        [view->scroll setFocusRingType:NSFocusRingTypeExterior];
    }

    return (OSText *)view->scroll;
}

/*---------------------------------------------------------------------------*/

void ostext_destroy(OSText **view)
{
    OSXTextView *lview = nil;
    OSXTextViewDelegate *delegate = nil;
    cassert_no_null(view);
    lview = [(NSScrollView *)*view documentView];
    cassert_no_null(lview);
    delegate = [lview delegate];
    cassert_no_null(delegate);
    [lview->attribs release];
    listener_destroy(&lview->OnFocus);
    listener_destroy(&lview->OnFilter);
    [lview setDelegate:nil];
    [delegate release];
    [lview release];
    [(*(NSScrollView **)view) release];
}

/*---------------------------------------------------------------------------*/

void ostext_OnFilter(OSText *view, Listener *listener)
{
    OSXTextView *lview = [(NSScrollView *)view documentView];
    cassert_no_null(view);
    listener_update(&lview->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void ostext_OnFocus(OSText *view, Listener *listener)
{
    OSXTextView *lview = [(NSScrollView *)view documentView];
    cassert_no_null(view);
    listener_update(&lview->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

static NSAttributedString *i_attr_str(OSXTextView *lview, const char_t *text)
{
    NSString *str = nil;
    NSAttributedString *astr = nil;
    cassert_no_null(lview);
    str = [NSString stringWithUTF8String:(const char *)text];
    astr = [[NSAttributedString alloc] initWithString:str attributes:lview->attribs];
    return astr;
}

/*---------------------------------------------------------------------------*/

void ostext_insert_text(OSText *view, const char_t *text)
{
    OSXTextView *lview = nil;
    NSAttributedString *astr = nil;
    cassert_no_null(view);
    cassert_no_null(text);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);
    astr = i_attr_str(lview, text);

    if (lview->is_editable == YES)
    {
        [[lview textStorage] appendAttributedString:astr];
    }
    else
    {
        [lview setEditable:YES];
        [[lview textStorage] appendAttributedString:astr];
        [lview setEditable:NO];
    }

    [astr release];
    lview->num_chars = i_text_num_chars(lview);
}

/*---------------------------------------------------------------------------*/

void ostext_set_text(OSText *view, const char_t *text)
{
    OSXTextView *lview = nil;
    NSAttributedString *astr = nil;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);
    astr = i_attr_str(lview, text);

    if (lview->is_editable == YES)
    {
        [[lview textStorage] setAttributedString:astr];
    }
    else
    {
        [lview setEditable:YES];
        [[lview textStorage] setAttributedString:astr];
        [lview setEditable:NO];
    }

    [astr release];
    lview->num_chars = i_text_num_chars(lview);
}

/*---------------------------------------------------------------------------*/

void ostext_set_rtf(OSText *view, Stream *rtf_in)
{
    unref(view);
    unref(rtf_in);
}

/*---------------------------------------------------------------------------*/

static NSFont *i_font_create(const char_t *family, const real32_t size, const uint32_t style)
{
    NSFont *nsfont = nil;
    cassert(size > 0.f);

    /* Unitialized font attribs */
    if (str_empty_c(family) == TRUE)
        return nil;

    if (size >= REAL32_MAX + 1e3f)
        return nil;

    if (style == UINT32_MAX)
        return nil;

    {
        Font *font = font_create(family, size, style);
        nsfont = (NSFont *)font_native(font);
        font_destroy(&font);
    }

    return nsfont;
}

/*---------------------------------------------------------------------------*/

static void i_change_font(OSXTextView *lview)
{
    NSFont *font = nil;
    cassert_no_null(lview);
    font = i_font_create(lview->ffamily, lview->fsize, lview->fstyle);
    if (font != nil)
    {
        NSNumber *under = (lview->fstyle & ekFUNDERLINE) ? [NSNumber numberWithInt:NSUnderlineStyleSingle] : [NSNumber numberWithInt:NSUnderlineStyleNone];
        NSNumber *strike = (lview->fstyle & ekFSTRIKEOUT) ? [NSNumber numberWithInt:NSUnderlineStyleSingle] : [NSNumber numberWithInt:NSUnderlineStyleNone];
        [lview->attribs setValue:font forKey:NSFontAttributeName];
        [lview->attribs setValue:under forKey:NSUnderlineStyleAttributeName];
        [lview->attribs setValue:strike forKey:NSStrikethroughStyleAttributeName];
    }
}

/*---------------------------------------------------------------------------*/

static void i_change_paragraph(OSXTextView *lview)
{
    cassert_no_null(lview);

    /* Unitialized paragraph attribs */
    if (lview->palign == ENUM_MAX(align_t))
        return;

    if (lview->pspacing >= REAL32_MAX + 1e3f)
        return;

    if (lview->pafter >= REAL32_MAX + 1e3f)
        return;

    if (lview->pbefore >= REAL32_MAX + 1e3f)
        return;

    {
        NSMutableParagraphStyle *par = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [par setAlignment:_oscontrol_text_alignment(lview->palign)];
        [par setLineSpacing:(CGFloat)lview->pspacing];
        [par setParagraphSpacing:(CGFloat)lview->pafter];
        [par setParagraphSpacingBefore:(CGFloat)lview->pbefore];
        [lview->attribs setValue:par forKey:NSParagraphStyleAttributeName];
    }
}

/*---------------------------------------------------------------------------*/

static void i_apply_all(OSXTextView *lview)
{
    NSTextStorage *storage = nil;
    cassert_no_null(lview);
    storage = [lview textStorage];
    if ([storage length] == 0)
    {
        NSAttributedString *astr = i_attr_str(lview, "FFF");
        [storage setAttributedString:astr];
        [astr release];
        astr = i_attr_str(lview, "");
        [storage setAttributedString:astr];
        [astr release];
    }
    else
    {
        NSRange all = NSMakeRange(0, [storage length]);
        [storage setAttributes:@{} range:all];
        [storage setAttributes:lview->attribs range:all];
    }
}

/*---------------------------------------------------------------------------*/

static void i_apply_sel(OSXTextView *lview)
{
    NSTextStorage *storage = nil;
    NSRange sel;
    cassert_no_null(lview);
    storage = [lview textStorage];
    sel = [lview selectedRange];
    [storage setAttributes:@{} range:sel];
    [storage setAttributes:lview->attribs range:sel];
}

/*---------------------------------------------------------------------------*/

void ostext_property(OSText *view, const gui_text_t param, const void *value)
{
    OSXTextView *lview = nil;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);

    switch (param)
    {
    case ekGUI_TEXT_FAMILY:
        if (str_equ_c(lview->ffamily, (const char_t *)value) == FALSE)
        {
            str_copy_c(lview->ffamily, sizeof(lview->ffamily), (const char_t *)value);
            i_change_font(lview);
        }
        break;

    case ekGUI_TEXT_UNITS:
        break;

    case ekGUI_TEXT_SIZE:
        if (lview->fsize != *((real32_t *)value))
        {
            lview->fsize = *((real32_t *)value);
            i_change_font(lview);
        }
        break;

    case ekGUI_TEXT_STYLE:
        if (lview->fstyle != *((uint32_t *)value))
        {
            lview->fstyle = *((uint32_t *)value);
            i_change_font(lview);
        }
        break;

    case ekGUI_TEXT_COLOR:
    {
        NSColor *color = nil;
        if (*(color_t *)value == kCOLOR_DEFAULT)
            color = oscolor_NSColor(1); /* ekSYS_LABEL */
        else
            color = oscolor_NSColor(*(color_t *)value);
        [lview->attribs setValue:color forKey:NSForegroundColorAttributeName];
        break;
    }

    case ekGUI_TEXT_BGCOLOR:
    {
        NSColor *color = oscolor_NSColor(*(color_t *)value);
        [lview->attribs setValue:color forKey:NSBackgroundColorAttributeName];
        break;
    }

    case ekGUI_TEXT_PGCOLOR:
        if (*(color_t *)value != kCOLOR_DEFAULT)
        {
            NSColor *color = oscolor_NSColor(*(color_t *)value);
            [lview setBackgroundColor:color];
            [lview setDrawsBackground:YES];
        }
        else
        {
            [lview setDrawsBackground:NO];
        }
        break;

    case ekGUI_TEXT_PARALIGN:
        if (lview->palign != *((align_t *)value))
        {
            lview->palign = *((align_t *)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_TEXT_LSPACING:
        if (lview->pspacing != *((real32_t *)value))
        {
            lview->pspacing = *((real32_t *)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_TEXT_AFPARSPACE:
        if (lview->pafter != *((real32_t *)value))
        {
            lview->pafter = *((real32_t *)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_TEXT_BFPARSPACE:
        if (lview->pbefore != *((real32_t *)value))
        {
            lview->pbefore = *((real32_t *)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_TEXT_SELECT:
    {
        int32_t start = cast_const(value, int32_t)[0];
        int32_t end = cast_const(value, int32_t)[1];
        /* Deselect all text */
        if (start == -1 && end == 0)
        {
            lview->select = NSMakeRange(0, 0);
        }
        /* Deselect all text and caret to the end */
        else if (start == -1 && end == -1)
        {
            lview->select = NSMakeRange(NSUIntegerMax, 0);
        }
        /* Select all text and caret to the end */
        else if (start == 0 && end == -1)
        {
            lview->select = NSMakeRange(0, NSUIntegerMax);
        }
        /* Select from position to the end */
        else if (start > 0 && end == -1)
        {
            lview->select = NSMakeRange(start, NSIntegerMax);
        }
        /* Deselect all and move the caret */
        else if (start == end)
        {
            lview->select = NSMakeRange(start, 0);
        }
        /* Select from start to end */
        else
        {
            lview->select = NSMakeRange(start, end - start);
        }

        if (lview->focused)
        {
            [lview setSelectedRange:lview->select];
        }

        break;
    }

    case ekGUI_TEXT_SHOW_SELECT:
    {
        bool_t show_sel = *((bool_t *)value);
        lview->show_select = (BOOL)show_sel;
        break;
    }

    case ekGUI_TEXT_SCROLL:
        [lview scrollRangeToVisible:[lview selectedRange]];
        break;

    case ekGUI_TEXT_WRAP_MODE:
    {
        bool_t wrap = *cast(value, bool_t);
        if ((BOOL)wrap != lview->is_wrap_mode)
        {
            lview->is_wrap_mode = (BOOL)wrap;
            i_set_wrap_mode(lview, lview->is_wrap_mode);
        }
        break;
    }

    case ekGUI_TEXT_APPLY_ALL:
        i_apply_all(lview);
        break;

    case ekGUI_TEXT_APPLY_SEL:
        i_apply_sel(lview);
        break;

        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void ostext_editable(OSText *view, const bool_t is_editable)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);
    lview->is_editable = (BOOL)is_editable;
    [lview setEditable:lview->is_editable];
}

/*---------------------------------------------------------------------------*/

const char_t *ostext_get_text(const OSText *view)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);
    return (const char_t *)[[[lview textStorage] string] UTF8String];
}

/*---------------------------------------------------------------------------*/

void ostext_scroller_visible(OSText *view, const bool_t horizontal, const bool_t vertical)
{
    NSScrollView *sview = (NSScrollView *)view;
    cassert_no_null(sview);
    [sview setHasHorizontalScroller:(BOOL)horizontal];
    [sview setHasVerticalScroller:(BOOL)vertical];
}

/*---------------------------------------------------------------------------*/

void ostext_set_need_display(OSText *view)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);
    [lview setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

void ostext_clipboard(OSText *view, const clipboard_t clipboard)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);

    switch (clipboard)
    {
    case ekCLIPBOARD_COPY:
        [lview copy:lview];
        break;
    case ekCLIPBOARD_CUT:
        [lview cut:lview];
        break;
    case ekCLIPBOARD_PASTE:
        [lview paste:lview];
        break;
    }
}

/*---------------------------------------------------------------------------*/

void ostext_attach(OSText *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, (NSView *)view);
}

/*---------------------------------------------------------------------------*/

void ostext_detach(OSText *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (NSView *)view);
}

/*---------------------------------------------------------------------------*/

void ostext_visible(OSText *view, const bool_t visible)
{
    _oscontrol_set_visible((NSView *)view, visible);
}

/*---------------------------------------------------------------------------*/

void ostext_enabled(OSText *view, const bool_t enabled)
{
    unref(view);
    unref(enabled);
}

/*---------------------------------------------------------------------------*/

void ostext_size(const OSText *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((NSView *)view, width, height);
}

/*---------------------------------------------------------------------------*/

void ostext_origin(const OSText *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((NSView *)view, x, y);
}

/*---------------------------------------------------------------------------*/

void ostext_frame(OSText *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXTextView *lview;
    lview = [(NSScrollView *)view documentView];
    _oscontrol_set_frame((NSView *)view, x, y, width, height);
    i_set_wrap_mode(lview, lview->is_wrap_mode);
}

/*---------------------------------------------------------------------------*/

void ostext_focus(OSText *view, const bool_t focus)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);
    lview->focused = (BOOL)focus;
    lview->focus_draw = YES;

    if (lview->OnFocus != NULL)
    {
        bool_t params = focus;
        listener_event(lview->OnFocus, ekGUI_EVENT_FOCUS, view, &params, NULL, OSText, bool_t, void);
    }

    if (focus == TRUE)
    {
        [lview setSelectedRange:lview->select];
    }
    else
    {
        lview->select = [lview selectedRange];
        if (lview->show_select == NO)
            [lview setSelectedRange:NSMakeRange(0, 0)];
    }

#if defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
#else
    [lview->scroll setNeedsDisplay:YES];
#endif
}

/*---------------------------------------------------------------------------*/

bool_t ostext_capture_return(const OSText *view)
{
    OSXTextView *lview;
    cassert_no_null(view);
    lview = [(NSScrollView *)view documentView];
    cassert_no_null(lview);
    return lview->is_editable;
}

/*---------------------------------------------------------------------------*/

BOOL _ostext_is(NSView *view)
{
    if ([view isKindOfClass:[NSScrollView class]])
        return [[(NSScrollView *)view documentView] isKindOfClass:[OSXTextView class]];
    return FALSE;
}
