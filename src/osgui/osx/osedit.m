/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.m
 *
 */

/* Operating System edit box */

#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "oswindow_osx.inl"
#include "oscolor.inl"
#include "../osedit.h"
#include "../osedit.inl"
#include "../osgui.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXTextFieldCell : NSTextFieldCell
{
  @public
    NSView *parent;
}

@end

/*---------------------------------------------------------------------------*/

@interface OSXSecureTextFieldCell : NSSecureTextFieldCell
{
  @public
    NSView *parent;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXTextField : NSTextField
{
  @public
    NSView *parent;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXSecureTextField : NSSecureTextField
{
  @public
    NSView *parent;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXEdit : NSView
{
  @public
    NSTextField *field;
    NSText *editor;
    uint32_t flags;
    uint32_t vpadding;
    real32_t rpadding;
    color_t bgcolor;
    NSRange select;
    CGFloat wpadding;
    OSTextAttr attrs;
    bool_t focused;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXEdit

- (BOOL)acceptsFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (BOOL)makeFirstResponder:(NSResponder *)responder
{
    unref(responder);
    return YES;
}

/*---------------------------------------------------------------------------*/

- (BOOL)becomeFirstResponder
{
    return [self->field becomeFirstResponder];
}

/*---------------------------------------------------------------------------*/

- (BOOL)resignFirstResponder
{
    return [self->field resignFirstResponder];
}

/*---------------------------------------------------------------------------*/

#if (defined MAC_OS_X_VERSION_10_6 && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6) || (defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
#else

- (void)drawRect:(NSRect)rect
{
    /* Draw focus ring in older mac OSX */
    if (self->focused == TRUE)
    {
        NSSetFocusRingStyle(NSFocusRingOnly);
        NSRectFill(rect);
    }
}

#endif

@end

/*---------------------------------------------------------------------------*/

@implementation OSXTextFieldCell

/*---------------------------------------------------------------------------*/

- (void)selectWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
    frame.origin.y += cast(parent, OSXEdit)->wpadding;
    [super selectWithFrame:frame inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

/*---------------------------------------------------------------------------*/

- (void)editWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
    frame.origin.y += cast(parent, OSXEdit)->wpadding;
    [super editWithFrame:frame inView:controlView editor:textObj delegate:anObject event:theEvent];
}

/*---------------------------------------------------------------------------*/

- (void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    frame.origin.y += cast(parent, OSXEdit)->wpadding;
    [super drawInteriorWithFrame:frame inView:controlView];
}

/*---------------------------------------------------------------------------*/

@end

/*---------------------------------------------------------------------------*/

@implementation OSXSecureTextFieldCell

/*---------------------------------------------------------------------------*/

- (void)selectWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
    frame.origin.y += cast(parent, OSXEdit)->wpadding;
    [super selectWithFrame:frame inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

/*---------------------------------------------------------------------------*/

- (void)editWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
    frame.origin.y += cast(parent, OSXEdit)->wpadding;
    [super editWithFrame:frame inView:controlView editor:textObj delegate:anObject event:theEvent];
}

/*---------------------------------------------------------------------------*/

- (void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    frame.origin.y += cast(parent, OSXEdit)->wpadding;
    [super drawInteriorWithFrame:frame inView:controlView];
}

/*---------------------------------------------------------------------------*/

@end

/*---------------------------------------------------------------------------*/

static void OSX_textDidChange(OSXEdit *edit, NSTextField *field)
{
    if ([field isEnabled] == YES && edit->OnFilter != NULL)
    {
        EvText params;
        EvTextFilter result;
        params.text = cast_const([[field stringValue] UTF8String], char_t);
        params.cpos = (uint32_t)[edit->editor selectedRange].location;
        params.len = INT32_MAX;
        result.apply = FALSE;
        result.text[0] = '\0';
        result.cpos = UINT32_MAX;
        listener_event(edit->OnFilter, ekGUI_EVENT_TXTFILTER, cast(edit, OSEdit), &params, &result, OSEdit, EvText, EvTextFilter);

        if (result.apply == TRUE)
            _oscontrol_set_text(field, &edit->attrs, result.text);

        if (result.cpos != UINT32_MAX)
            [edit->editor setSelectedRange:NSMakeRange((NSUInteger)result.cpos, 0)];
        else
            [edit->editor setSelectedRange:NSMakeRange((NSUInteger)params.cpos, 0)];
    }
}

/*---------------------------------------------------------------------------*/

static void OSX_textDidEndEditing(OSXEdit *edit, NSNotification *notification)
{
    unsigned int whyEnd = [[[notification userInfo] objectForKey:@"NSTextMovement"] unsignedIntValue];
    OSXEdit *ledit = cast(edit, OSXEdit);
    NSWindow *window = [edit window];
    cassert_no_null(ledit);

    ledit->select = [ledit->editor selectedRange];
    [ledit->editor setSelectedRange:NSMakeRange(0, 0)];

    if (whyEnd == NSReturnTextMovement)
    {
        [window keyDown:cast(231, NSEvent)];
    }
    else if (whyEnd == NSTabTextMovement)
    {
        _oswindow_next_tabstop(window, TRUE);
    }
    else if (whyEnd == NSBacktabTextMovement)
    {
        _oswindow_prev_tabstop(window, TRUE);
    }
}

/*---------------------------------------------------------------------------*/

@implementation OSXTextField

/*---------------------------------------------------------------------------*/

- (BOOL)resignFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (BOOL)becomeFirstResponder
{
    [super becomeFirstResponder];
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)textDidChange:(NSNotification *)notification
{
    unref(notification);
    OSX_textDidChange(cast(self->parent, OSXEdit), self);
}

/*---------------------------------------------------------------------------*/

- (void)textDidEndEditing:(NSNotification *)notification
{
    OSX_textDidEndEditing(cast(self->parent, OSXEdit), notification);
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    if (_oswindow_mouse_down(cast(self->parent, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

@end

/*---------------------------------------------------------------------------*/

@implementation OSXSecureTextField

/*---------------------------------------------------------------------------*/

- (BOOL)resignFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (BOOL)becomeFirstResponder
{
    [super becomeFirstResponder];
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)textDidChange:(NSNotification *)notification
{
    unref(notification);
    OSX_textDidChange(cast(self->parent, OSXEdit), self);
}

/*---------------------------------------------------------------------------*/

- (void)textDidEndEditing:(NSNotification *)notification
{
    OSX_textDidEndEditing(cast(self->parent, OSXEdit), notification);
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    if (_oswindow_mouse_down(cast(self->parent, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

@end

/*---------------------------------------------------------------------------*/

static void i_update_cell(OSXEdit *edit)
{
    NSCell *cell = [edit->field cell];
    if ([cast(cell, NSObject) isKindOfClass:[OSXTextFieldCell class]] == YES)
    {
        cast(cell, OSXTextFieldCell)->parent = edit;
    }
    else
    {
        cassert([cast(cell, NSObject) isKindOfClass:[OSXSecureTextFieldCell class]] == YES);
        cast(cell, OSXSecureTextFieldCell)->parent = edit;
    }
}

/*---------------------------------------------------------------------------*/

static void i_update_vpadding(OSXEdit *edit)
{
    real32_t width, height;
    uint32_t defpadding = 0;

    cassert_no_null(edit);
    font_extents(edit->attrs.font, "OO", -1.f, &width, &height);

    defpadding = (uint32_t)((.3f * height) + .5f);
    if (defpadding % 2 == 1)
        defpadding += 1;

    if (defpadding < 5)
        defpadding = 5;

    if (edit->vpadding == UINT32_MAX)
    {
        edit->rpadding = (real32_t)defpadding;
        edit->wpadding = 0;
    }
    else
    {
        real32_t leading = font_leading(edit->attrs.font);
        uint32_t padding = (uint32_t)(edit->vpadding + leading);

        padding += 4;

        if (padding > defpadding)
            edit->wpadding = (CGFloat)((padding - defpadding) / 2);
        else
            edit->wpadding = 0;

        edit->rpadding = (real32_t)padding;
    }

    i_update_cell(edit);
}

/*---------------------------------------------------------------------------*/

OSEdit *osedit_create(const uint32_t flags)
{
    OSXEdit *edit = nil;
    OSXTextField *field = nil;
    heap_auditor_add("OSXEdit");
    edit = [[OSXEdit alloc] initWithFrame:NSZeroRect];
    field = [[OSXTextField alloc] initWithFrame:NSZeroRect];
    [field setCell:[[OSXTextFieldCell alloc] init]];
    edit->editor = nil;
    edit->flags = flags;
    edit->bgcolor = kCOLOR_DEFAULT;
    edit->vpadding = UINT32_MAX;
    edit->select = NSMakeRange(0, 0);
    edit->OnFilter = NULL;
    edit->OnChange = NULL;
    edit->OnFocus = NULL;
    [edit addSubview:field];
    edit->field = field;
    field->parent = edit;
    _oscontrol_init(edit);
    _oscontrol_init_textattr(&edit->attrs);
    i_update_vpadding(edit);
    [edit->field setEditable:YES];
    [edit->field setSelectable:YES];
    [edit->field setBordered:YES];
    [edit->field setBezeled:YES];
    [edit->field setDrawsBackground:YES];
    [edit->field setStringValue:@""];
    [edit->field setAlignment:_oscontrol_text_alignment(ekLEFT)];
    _oscontrol_set_align(edit->field, &edit->attrs, ekLEFT);
    _oscontrol_set_font(edit->field, &edit->attrs, edit->attrs.font);
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [[edit->field cell] setUsesSingleLineMode:((flags & 1) == 1) ? NO : YES];
#endif
    return cast(edit, OSEdit);
}

/*---------------------------------------------------------------------------*/

void osedit_destroy(OSEdit **edit)
{
    OSXEdit *ledit = nil;
    cassert_no_null(edit);
    ledit = *dcast(edit, OSXEdit);
    cassert_no_null(ledit);
    listener_destroy(&ledit->OnFilter);
    listener_destroy(&ledit->OnChange);
    listener_destroy(&ledit->OnFocus);
    _oscontrol_remove_textattr(&ledit->attrs);
    [ledit release];
    *edit = NULL;
    heap_auditor_delete("OSXEdit");
}

/*---------------------------------------------------------------------------*/

void osedit_OnFilter(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    listener_update(&cast(edit, OSXEdit)->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnChange(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    listener_update(&cast(edit, OSXEdit)->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnFocus(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    listener_update(&cast(edit, OSXEdit)->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_text(OSEdit *edit, const char_t *text)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    _oscontrol_set_text(ledit->field, &ledit->attrs, text);
}

/*---------------------------------------------------------------------------*/

void osedit_tooltip(OSEdit *edit, const char_t *text)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    _oscontrol_tooltip_set(ledit->field, text);
}

/*---------------------------------------------------------------------------*/

void osedit_font(OSEdit *edit, const Font *font)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    _oscontrol_set_font(ledit->field, &ledit->attrs, font);
    i_update_vpadding(ledit);
}

/*---------------------------------------------------------------------------*/

void osedit_align(OSEdit *edit, const align_t align)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    [ledit->field setAlignment:_oscontrol_text_alignment(align)];
    _oscontrol_set_align(ledit->field, &ledit->attrs, align);
}

/*---------------------------------------------------------------------------*/

void osedit_passmode(OSEdit *edit, const bool_t passmode)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    NSTextField *field = nil;
    cassert_no_null(ledit);
    if (passmode == TRUE)
    {
        if ([ledit->field isKindOfClass:[OSXTextField class]])
        {
            NSRect rect = [ledit->field frame];
            OSXSecureTextField *nfield = [[OSXSecureTextField alloc] initWithFrame:rect];
            [nfield setCell:[[OSXSecureTextFieldCell alloc] init]];
            nfield->parent = ledit;
            field = nfield;
        }
    }
    else
    {
        if ([ledit->field isKindOfClass:[OSXSecureTextField class]])
        {
            NSRect rect = [ledit->field frame];
            OSXTextField *nfield = [[OSXTextField alloc] initWithFrame:rect];
            [nfield setCell:[[OSXTextFieldCell alloc] init]];
            nfield->parent = ledit;
            field = nfield;
        }
    }

    if (field != nil)
    {
        NSString *text = [[ledit->field cell] stringValue];
        [field setEditable:[ledit->field isEditable]];
        [field setSelectable:[ledit->field isSelectable]];
        [field setBordered:[ledit->field isBordered]];
        [field setBezeled:[ledit->field isBezeled]];
        [field setDrawsBackground:[ledit->field drawsBackground]];
        _oscontrol_set_font(field, &ledit->attrs, ledit->attrs.font);
        _oscontrol_set_text_color(field, &ledit->attrs, ledit->attrs.color);
        _oscontrol_set_align(field, &ledit->attrs, ledit->attrs.align);
        _oscontrol_set_text(field, &ledit->attrs, cast_const([text UTF8String], char_t));
        _oscontrol_detach_from_parent(ledit->field, ledit);
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
        [[field cell] setUsesSingleLineMode:((ledit->flags & 1) == 1) ? NO : YES];
#endif
        [ledit->field release];
        ledit->field = field;
        [ledit addSubview:field];
        i_update_cell(ledit);
    }
}

/*---------------------------------------------------------------------------*/

void osedit_editable(OSEdit *edit, const bool_t is_editable)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    [ledit->field setEditable:is_editable == TRUE ? YES : NO];
}

/*---------------------------------------------------------------------------*/

void osedit_autoselect(OSEdit *edit, const bool_t autoselect)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    if (autoselect == TRUE)
        BIT_SET(ledit->flags, ekEDIT_AUTOSEL);
    else
        BIT_CLEAR(ledit->flags, ekEDIT_AUTOSEL);
}

/*---------------------------------------------------------------------------*/

/* http://alienryderflex.com/hasFocus.html */
static bool_t i_has_focus(id control)
{
    NSWindow *window = [control window];
    id first = [window firstResponder];
    return (bool_t)([first isKindOfClass:[NSTextView class]] && [window fieldEditor:NO forObject:nil] != nil && (first == control || [first delegate] == control));
}

/*---------------------------------------------------------------------------*/

void osedit_select(OSEdit *edit, const int32_t start, const int32_t end)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    /* Deselect all text */
    if (start == -1 && end == 0)
    {
        ledit->select = NSMakeRange(0, 0);
    }
    /* Deselect all text and caret to the end */
    else if (start == -1 && end == -1)
    {
        ledit->select = NSMakeRange(NSUIntegerMax, 0);
    }
    /* Select all text and caret to the end */
    else if (start == 0 && end == -1)
    {
        ledit->select = NSMakeRange(0, NSUIntegerMax);
    }
    /* Select from position to the end */
    else if (start > 0 && end == -1)
    {
        ledit->select = NSMakeRange(start, NSIntegerMax);
    }
    /* Deselect all and move the caret */
    else if (start == end)
    {
        ledit->select = NSMakeRange(start, 0);
    }
    /* Select from start to end */
    else
    {
        ledit->select = NSMakeRange(start, end - start);
    }

    if (i_has_focus(ledit->field) == TRUE)
    {
        if (ledit->editor != nil)
            [ledit->editor setSelectedRange:ledit->select];
    }
}

/*---------------------------------------------------------------------------*/

void osedit_color(OSEdit *edit, const color_t color)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    _oscontrol_set_text_color(ledit->field, &ledit->attrs, color);
}

/*---------------------------------------------------------------------------*/

static void i_set_bgcolor(OSXEdit *edit)
{
    NSColor *nscolor = nil;
    cassert_no_null(edit);
    if (edit->bgcolor != kCOLOR_DEFAULT)
        nscolor = _oscolor_NSColor(edit->bgcolor);
    [edit->field setBackgroundColor:nscolor];
    if (edit->editor != nil)
        [edit->editor setBackgroundColor:nscolor];
}

/*---------------------------------------------------------------------------*/

void osedit_bgcolor(OSEdit *edit, const color_t color)
{
    NSColor *nscolor = color != 0 ? _oscolor_NSColor(color) : [NSColor textBackgroundColor];
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    ledit->bgcolor = color;
    i_set_bgcolor(ledit);
}

/*---------------------------------------------------------------------------*/

void osedit_vpadding(OSEdit *edit, const real32_t padding)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    cassert(padding >= 0);
    ledit->vpadding = (uint32_t)padding;
    i_update_vpadding(ledit);
}

/*---------------------------------------------------------------------------*/

void osedit_bounds(const OSEdit *edit, const real32_t refwidth, const uint32_t lines, real32_t *width, real32_t *height)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    cassert_no_null(width);
    cassert_no_null(height);
    cassert_unref(lines == 1, lines);

    if (edit_get_type(ledit->flags) == ekEDIT_SINGLE)
        font_extents(ledit->attrs.font, "O", -1.f, width, height);
    else
        font_extents(ledit->attrs.font, "O\nO", -1.f, width, height);

    *width = refwidth;
    *height += ledit->rpadding;
}

/*---------------------------------------------------------------------------*/

void osedit_clipboard(OSEdit *edit, const clipboard_t clipboard)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    if (ledit->editor != nil)
    {
        switch (clipboard)
        {
        case ekCLIPBOARD_COPY:
            [ledit->editor copy:ledit->editor];
            break;
        case ekCLIPBOARD_CUT:
            [ledit->editor cut:ledit->editor];
            break;
        case ekCLIPBOARD_PASTE:
            [ledit->editor paste:ledit->editor];
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/

void osedit_attach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(edit, NSView));
}

/*---------------------------------------------------------------------------*/

void osedit_detach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(edit, NSView));
}

/*---------------------------------------------------------------------------*/

void osedit_visible(OSEdit *edit, const bool_t visible)
{
    _oscontrol_set_visible(cast(edit, NSView), visible);
}

/*---------------------------------------------------------------------------*/

void osedit_enabled(OSEdit *edit, const bool_t enabled)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    if (enabled == FALSE)
    {
        if (i_has_focus(ledit->field) == TRUE)
            [[ledit->field window] endEditingFor:ledit->field];
    }

    _oscontrol_set_enabled(ledit->field, enabled);
    _oscontrol_set_text_color(ledit->field, &ledit->attrs, ledit->attrs.color);
}

/*---------------------------------------------------------------------------*/

void osedit_size(const OSEdit *edit, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast(edit, NSView), width, height);
}

/*---------------------------------------------------------------------------*/

void osedit_origin(const OSEdit *edit, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast(edit, NSView), x, y);
}

/*---------------------------------------------------------------------------*/

void osedit_frame(OSEdit *edit, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    _oscontrol_set_frame(cast(ledit, NSView), x, y, width, height);
    _oscontrol_set_frame(cast(ledit->field, NSView), 1, 1, width - 2, height - 2);
    [ledit setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

bool_t _osedit_resign_focus(const OSEdit *edit)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    NSWindow *window = [ledit window];
    bool_t resign = TRUE;
    cassert_no_null(ledit);
    if (ledit->OnChange != NULL && _oswindow_in_destroy(window) == NO)
    {
        EvText params;
        params.text = cast_const([[ledit->field stringValue] UTF8String], char_t);
        params.cpos = (uint32_t)[ledit->editor selectedRange].location;
        params.len = (int32_t)unicode_nchars(params.text, ekUTF8);
        listener_event(ledit->OnChange, ekGUI_EVENT_TXTCHANGE, edit, &params, &resign, OSEdit, EvText, bool_t);
    }

    if (resign == TRUE)
        [window endEditingFor:ledit];
    else
        [window makeFirstResponder:ledit];

    return resign;
}

/*---------------------------------------------------------------------------*/

void _osedit_focus(OSEdit *edit, const bool_t focus)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    if (ledit->OnFocus != NULL)
    {
        bool_t params = focus;
        listener_event(ledit->OnFocus, ekGUI_EVENT_FOCUS, edit, &params, NULL, OSEdit, bool_t, void);
    }

    ledit->focused = focus;
    if (focus == TRUE)
    {
        if ([ledit->field isEnabled] == YES)
        {
            NSWindow *window = [ledit->field window];
            ledit->editor = [window fieldEditor:YES forObject:ledit->field];
            i_set_bgcolor(ledit);

            if (BIT_TEST(ledit->flags, ekEDIT_AUTOSEL) == TRUE)
            {
                [ledit->editor selectAll:nil];
            }
            else
            {
                [ledit->editor setSelectedRange:ledit->select];
            }
        }
    }
    else
    {
        ledit->editor = nil;
    }

#if (defined MAC_OS_X_VERSION_10_6 && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6) || (defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
#else
    [ledit setNeedsDisplay:YES];
#endif
}

/*---------------------------------------------------------------------------*/

BOOL _osedit_is(NSView *view)
{
    return [view isKindOfClass:[OSXEdit class]];
}
