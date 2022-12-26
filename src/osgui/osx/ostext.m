/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
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
#include "oscolor.inl"
#include "ospanel.inl"
#include "cassert.h"
#include "color.h"
#include "event.h"
#include "heap.h"
#include "strings.h"
#include "ptr.h"

#if !defined (__MACOS__)
#error This file is only for OSX
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
    NSMutableDictionary *dict;
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
        listener_event(self->OnTextChange_listener, ekGUI_EVENT_TXTCHANGE, (OSText*)self, &params, NULL, OSText, EvText, void);
    }
}

@end

/*---------------------------------------------------------------------------*/

OSText *ostext_create(const uint32_t flags)
{
    OSXTextView *view = [[OSXTextView alloc] initWithFrame:NSZeroRect];
    unref(flags);
    heap_auditor_add("OSXTextView");
    view->is_editable = NO;
    view->is_opaque = YES;
    view->scroll = [[NSScrollView alloc] initWithFrame:NSZeroRect];
    view->dict = [[NSMutableDictionary alloc] init];
    view->ffamily[0] = '\0';
    view->fsize = REAL32_MAX;
    view->fstyle = UINT32_MAX;
    view->palign = ENUM_MAX(align_t);
    view->pspacing = REAL32_MAX;
    view->pafter = REAL32_MAX;
    view->pbefore = REAL32_MAX;
    [view->scroll setDocumentView:view];
    [view->scroll setHasVerticalScroller:YES];
    [view->scroll setHasHorizontalScroller:YES];
    [view->scroll setAutohidesScrollers:YES];
    [view->scroll setBorderType:NSBezelBorder];
    [view->scroll setHidden:YES];
    [view setEditable:view->is_editable];
    [view setRichText:YES];

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

    {
        NSColor *color = oscolor_NSColor(1); /* ekSYS_LABEL */
        [view->dict setValue:color forKey:NSForegroundColorAttributeName];
    }

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
    [lview->dict release];
    listener_destroy(&delegate->OnTextChange_listener);
    [lview setDelegate:nil];
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

static NSAttributedString *i_attr_str(OSXTextView *lview, const char_t *text)
{
    NSString *str = nil;
    NSAttributedString *astr = nil;
    cassert_no_null(lview);
    str = [NSString stringWithUTF8String:(const char*)text];
    astr = [[NSAttributedString alloc] initWithString:str attributes:lview->dict];
    return astr;
}

/*---------------------------------------------------------------------------*/

void ostext_insert_text(OSText *view, const char_t *text)
{
    OSXTextView *lview = nil;
    NSAttributedString *astr = nil;
    cassert_no_null(view);
    cassert_no_null(text);
    lview = [(NSScrollView*)view documentView];
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
}

/*---------------------------------------------------------------------------*/

void ostext_set_text(OSText *view, const char_t *text)
{
    OSXTextView *lview = nil;
    NSAttributedString *astr = nil;
    cassert_no_null(view);
    lview = [(NSScrollView*)view documentView];
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
}

/*---------------------------------------------------------------------------*/

void ostext_set_rtf(OSText *view, Stream *rtf_in)
{
    unref(view);
    unref(rtf_in);
}

/*---------------------------------------------------------------------------*/

static NSFont *i_convent_to_italic(NSFont *font, const CGFloat height, NSFontManager *font_manager)
{
    NSFont *italic_font = nil;
    NSFontTraitMask fontTraits = (NSFontTraitMask)0;
    cassert_no_null(font);
    
    italic_font = [font_manager convertFont:font toHaveTrait:NSItalicFontMask];
    fontTraits = [font_manager traitsOfFont:italic_font];
    
    if ((fontTraits & NSItalicFontMask) == 0)
    {
        NSAffineTransform *font_transform = [NSAffineTransform transform];
        [font_transform scaleBy:height];
        
        {
            NSAffineTransformStruct data;
            NSAffineTransform *italic_transform = nil;
            data.m11 = 1.f;
            data.m12 = 0.f;
            data.m21 = - tanf(/*italic_angle*/-10.f * 0.017453292519943f);
            data.m22 = 1.f;
            data.tX  = 0.f;
            data.tY  = 0.f;
            italic_transform = [NSAffineTransform transform];
            [italic_transform setTransformStruct:data];
            [font_transform appendTransform:italic_transform];
        }
        
        italic_font = [NSFont fontWithDescriptor:[italic_font fontDescriptor] textTransform:font_transform];
    }
    
    return italic_font;
}

/*---------------------------------------------------------------------------*/

static NSFont *i_font_create(const char_t *family, const real32_t size, const uint32_t style)
{
    NSFont *nsfont = nil;
    cassert(size > 0.f);

    // Unitialized font attribs
    if (str_empty_c(family) == TRUE)
        return nil;

    if (size >= REAL32_MAX + 1e3f)
        return nil;

    if (style == UINT32_MAX)
        return nil;
    
    {
        NSFontManager *fontManager = [NSFontManager sharedFontManager];
        NSString *ffamily = [NSString stringWithUTF8String:family];
        NSUInteger mask = (style & ekFBOLD) ? NSBoldFontMask : 0;
        nsfont = [fontManager fontWithFamily:ffamily traits:(NSFontTraitMask)mask weight:5 size:(CGFloat)size];
        cassert_fatal_msg(nsfont != nil, "Font is not available on this computer.");
    }

    if (nsfont != nil)
    {
        if (style & ekFITALIC)
        {
            NSFontManager *fontManager = [NSFontManager sharedFontManager];
            nsfont = i_convent_to_italic(nsfont, (CGFloat)size, fontManager);
        }
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
        [lview->dict setValue:font forKey:NSFontAttributeName];
        [lview->dict setValue:under forKey:NSUnderlineStyleAttributeName];
        [lview->dict setValue:strike forKey:NSStrikethroughStyleAttributeName];
    }
}

/*---------------------------------------------------------------------------*/

static void i_change_paragraph(OSXTextView *lview)
{
    cassert_no_null(lview);

    // Unitialized paragraph attribs
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
        [lview->dict setValue:par forKey:NSParagraphStyleAttributeName];
    }
}

/*---------------------------------------------------------------------------*/

void ostext_property(OSText *view, const gui_prop_t param, const void *value)
{
    OSXTextView *lview = nil;
    cassert_no_null(view);
    lview = [(NSScrollView*)view documentView];
    cassert_no_null(lview);
            
    switch (param) {
    case ekGUI_PROP_FAMILY:
        if (str_equ_c(lview->ffamily, (const char_t*)value) == FALSE)
        {
            str_copy_c(lview->ffamily, sizeof(lview->ffamily), (const char_t*)value);
            i_change_font(lview);
        }
        break;

    case ekGUI_PROP_UNITS:
        break;

    case ekGUI_PROP_SIZE:
        if (lview->fsize != *((real32_t*)value))
        {
            lview->fsize = *((real32_t*)value);
            i_change_font(lview);
        }
        break;

    case ekGUI_PROP_STYLE:
        if (lview->fstyle != *((uint32_t*)value))
        {
            lview->fstyle = *((uint32_t*)value);
            i_change_font(lview);
        }
        break;

    case ekGUI_PROP_COLOR:
    {
        NSColor *color = nil;
        if (*(color_t*)value == kCOLOR_TRANSPARENT)
            color = oscolor_NSColor(1); // ekSYS_LABEL
        else
            color = oscolor_NSColor(*(color_t*)value);
        [lview->dict setValue:color forKey:NSForegroundColorAttributeName];
        break;
    }

    case ekGUI_PROP_BGCOLOR:
    {
        NSColor *color = oscolor_NSColor(*(color_t*)value);
        [lview->dict setValue:color forKey:NSBackgroundColorAttributeName];
        break;
    }

    case ekGUI_PROP_PGCOLOR:
        if (*(color_t*)value != kCOLOR_TRANSPARENT)
        {
            NSColor *color = oscolor_NSColor(*(color_t*)value);
            [lview setBackgroundColor:color];
            [lview setDrawsBackground:YES];
        }
        else
        {
            [lview setDrawsBackground:NO];
        }
        break;

    case ekGUI_PROP_PARALIGN:
        if (lview->palign != *((align_t*)value))
        {
            lview->palign = *((align_t*)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_PROP_LSPACING:
        if (lview->pspacing != *((real32_t*)value))
        {
            lview->pspacing = *((real32_t*)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_PROP_AFPARSPACE:
        if (lview->pafter != *((real32_t*)value))
        {
            lview->pafter = *((real32_t*)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_PROP_BFPARSPACE:
        if (lview->pbefore != *((real32_t*)value))
        {
            lview->pbefore = *((real32_t*)value);
            i_change_paragraph(lview);
        }
        break;

    case ekGUI_PROP_VSCROLL:
    {
        NSRange edrange = NSMakeRange([[lview string] length], 0);
        [lview scrollRangeToVisible:edrange];
        break;
    }

    cassert_default();
    }
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


