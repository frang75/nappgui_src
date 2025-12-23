/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostextfield.m
 *
 */

/* TextField common implementation for OSXEdit and OSXCombo */
/*
 * This implementation combines a dual NSTextField (visible and secure)
 * Allow to OSXEdit and OSXCombo controls switch from/to normal/secure modes
 * Apple doesn't provide support for this property switch.
 * Also provides common support for text events and callbacks.
 */
#include "ostextfield.inl"
#include "oscontrol_osx.inl"
#include "oswindow_osx.inl"
#include "oscolor.inl"
#include <draw2d/color.h>
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
    OSTextField *field;
}

@end

/*---------------------------------------------------------------------------*/

@interface OSXSecureTextFieldCell : NSSecureTextFieldCell
{
  @public
    OSTextField *field;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXTextField : NSTextField
{
  @public
    OSTextField *field;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXSecureTextField : NSSecureTextField
{
  @public
    OSTextField *field;
}
@end

struct _ostext_field_t
{
    NSTextField *impl;
    String *type;
    NSView *control; /* OSXEdit, OSXCombo */
    NSText *editor;
    CGFloat wpadding;
    NSRange select;
    bool_t single_line;
    bool_t autoselect;
    bool_t focused;
    color_t bgcolor;
    OSTextAttr attrs;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
};

/*---------------------------------------------------------------------------*/

@implementation OSXTextFieldCell

/*---------------------------------------------------------------------------*/

- (void)selectWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
    frame.origin.y += field->wpadding;
    frame.size.height -= field->wpadding;
    [super selectWithFrame:frame inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

/*---------------------------------------------------------------------------*/

- (void)editWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
    frame.origin.y += field->wpadding;
    frame.size.height -= field->wpadding;
    [super editWithFrame:frame inView:controlView editor:textObj delegate:anObject event:theEvent];
}

/*---------------------------------------------------------------------------*/

- (void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    frame.origin.y += field->wpadding;
    frame.size.height -= field->wpadding;
    [super drawInteriorWithFrame:frame inView:controlView];
}

/*---------------------------------------------------------------------------*/

@end

/*---------------------------------------------------------------------------*/

@implementation OSXSecureTextFieldCell

/*---------------------------------------------------------------------------*/

- (void)selectWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
    frame.origin.y += field->wpadding;
    frame.size.height -= field->wpadding;
    [super selectWithFrame:frame inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

/*---------------------------------------------------------------------------*/

- (void)editWithFrame:(NSRect)frame inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
    frame.origin.y += field->wpadding;
    frame.size.height -= field->wpadding;
    [super editWithFrame:frame inView:controlView editor:textObj delegate:anObject event:theEvent];
}

/*---------------------------------------------------------------------------*/

- (void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    frame.origin.y += field->wpadding;
    frame.size.height -= field->wpadding;
    [super drawInteriorWithFrame:frame inView:controlView];
}

/*---------------------------------------------------------------------------*/

@end

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
    _ostextfield_textDidChange(self->field);
}

/*---------------------------------------------------------------------------*/

- (void)textDidEndEditing:(NSNotification *)notification
{
    _ostextfield_textDidEndEditing(self->field, notification);
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    cassert_no_null(self->field);
    if (_oswindow_mouse_down(cast(self->field->control, OSControl)) == TRUE)
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
    _ostextfield_textDidChange(self->field);
}

/*---------------------------------------------------------------------------*/

- (void)textDidEndEditing:(NSNotification *)notification
{
    _ostextfield_textDidEndEditing(self->field, notification);
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    cassert_no_null(self->field);
    if (_oswindow_mouse_down(cast(self->field->control, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

@end

/*---------------------------------------------------------------------------*/

static void i_update_impl(OSTextField *field)
{
    NSCell *cell = [field->impl cell];
    if ([cast(cell, NSObject) isKindOfClass:[OSXTextFieldCell class]] == YES)
    {
        cast(field->impl, OSXTextField)->field = field;
        cast(cell, OSXTextFieldCell)->field = field;
    }
    else
    {
        cassert([cast(cell, NSObject) isKindOfClass:[OSXSecureTextFieldCell class]] == YES);
        cast(field->impl, OSXSecureTextField)->field = field;
        cast(cell, OSXSecureTextFieldCell)->field = field;
    }
}

/*---------------------------------------------------------------------------*/

static OSTextField *i_create_field(NSView *control, const bool_t single_line, const char_t *type)
{
    OSTextField *field = heap_new0(OSTextField);
    field->type = str_c(type);
    field->control = control;
    field->editor = nil;
    field->wpadding = 0;
    field->select = NSMakeRange(0, 0);
    field->single_line = single_line;
    field->focused = FALSE;
    field->bgcolor = kCOLOR_DEFAULT;
    field->OnFilter = NULL;
    field->OnChange = NULL;
    field->OnFocus = NULL;
    _oscontrol_init_textattr(&field->attrs);
    return field;
}

/*---------------------------------------------------------------------------*/

OSTextField *_ostextfield_from_edit(NSView *control, const bool_t single_line)
{
    OSTextField *field = i_create_field(control, single_line, "OSEdit");
    field->impl = [[OSXTextField alloc] initWithFrame:NSZeroRect];
    [field->impl setCell:[[OSXTextFieldCell alloc] init]];
    [field->impl setEditable:YES];
    [field->impl setSelectable:YES];
    [field->impl setBordered:YES];
    [field->impl setBezeled:YES];
    [field->impl setDrawsBackground:YES];
    [field->impl setStringValue:@""];
    [field->impl setAlignment:_oscontrol_text_alignment(ekLEFT)];
    _oscontrol_set_align(field->impl, &field->attrs, ekLEFT);
    _oscontrol_set_font(field->impl, &field->attrs, field->attrs.font);
    [[field->impl cell] setScrollable:(BOOL)field->single_line];
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [[field->impl cell] setUsesSingleLineMode:(BOOL)field->single_line];
#endif
    _oscontrol_attach_to_parent(field->impl, field->control);
    _oscontrol_detach_from_parent(field->impl, field->control);
    _oscontrol_attach_to_parent(field->impl, field->control);
    i_update_impl(field);
    return field;
}

/*---------------------------------------------------------------------------*/

OSTextField *_ostextfield_from_combo(NSView *control)
{
    OSTextField *field = i_create_field(control, TRUE, "OSCombo");
    field->impl = nil;
    return field;
}

/*---------------------------------------------------------------------------*/

void _ostextfield_destroy(OSTextField **field)
{
    cassert_no_null(field);
    cassert_no_null(*field);
    /* impl will be destroyed with control */
    cassert((*field)->impl == nil || [(*field)->impl isDescendantOf:(*field)->control]);
    str_destroy(&(*field)->type);
    listener_destroy(&(*field)->OnFilter);
    listener_destroy(&(*field)->OnChange);
    listener_destroy(&(*field)->OnFocus);
    _oscontrol_remove_textattr(&(*field)->attrs);
    heap_delete(field, OSTextField);
}

/*---------------------------------------------------------------------------*/

static NSTextField *i_impl(const OSTextField *field)
{
    cassert_no_null(field);
    if (field->impl != nil)
    {
        return field->impl;
    }
    else
    {
        cassert([field->control isKindOfClass:[NSTextField class]] == YES);
        return cast(field->control, NSTextField);
    }
}

/*---------------------------------------------------------------------------*/

BOOL _ostextfield_becomeFirstResponder(OSTextField *field)
{
    cassert_no_null(field);
    cassert_no_null(i_impl(field));
    return [i_impl(field) becomeFirstResponder];
}

/*---------------------------------------------------------------------------*/

BOOL _ostextfield_resignFirstResponder(OSTextField *field)
{
    cassert_no_null(field);
    cassert_no_null(i_impl(field));
    return [i_impl(field) resignFirstResponder];
}

/*---------------------------------------------------------------------------*/

void _ostextfield_textDidChange(OSTextField *field)
{
    NSTextField *impl = i_impl(field);
    cassert_no_null(field);
    cassert_no_null(impl);
    if ([impl isEnabled] == YES && field->OnFilter != NULL)
    {
        EvText params;
        EvTextFilter result;
        params.text = cast_const([[impl stringValue] UTF8String], char_t);
        params.cpos = (uint32_t)[field->editor selectedRange].location;
        params.len = INT32_MAX;
        result.apply = FALSE;
        result.text[0] = '\0';
        result.cpos = UINT32_MAX;
        listener_event_imp(field->OnFilter, ekGUI_EVENT_TXTFILTER, cast(field->control, void), cast(&params, void), cast(&result, void), tc(field->type), "EvText", "EvTextFilter");

        if (result.apply == TRUE)
            _oscontrol_set_text(impl, &field->attrs, result.text);

        if (result.cpos != UINT32_MAX)
            [field->editor setSelectedRange:NSMakeRange((NSUInteger)result.cpos, 0)];
        else
            [field->editor setSelectedRange:NSMakeRange((NSUInteger)params.cpos, 0)];
    }
}

/*---------------------------------------------------------------------------*/

void _ostextfield_textDidEndEditing(OSTextField *field, NSNotification *notification)
{
    NSWindow *window = nil;
    unsigned int whyEnd = [[[notification userInfo] objectForKey:@"NSTextMovement"] unsignedIntValue];
    cassert_no_null(field);
    window = [field->control window];
    field->select = [field->editor selectedRange];
    [field->editor setSelectedRange:NSMakeRange(0, 0)];

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

void _ostextfield_wpadding(OSTextField *field, CGFloat wpadding)
{
    cassert_no_null(field);
    field->wpadding = wpadding;
}

/*---------------------------------------------------------------------------*/

void _ostextfield_OnFilter(OSTextField *field, Listener *listener)
{
    cassert_no_null(field);
    listener_update(&field->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void _ostextfield_OnChange(OSTextField *field, Listener *listener)
{
    cassert_no_null(field);
    listener_update(&field->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void _ostextfield_OnFocus(OSTextField *field, Listener *listener)
{
    cassert_no_null(field);
    listener_update(&field->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void _ostextfield_text(OSTextField *field, const char_t *text)
{
    cassert_no_null(field);
    _oscontrol_set_text(i_impl(field), &field->attrs, text);
}

/*---------------------------------------------------------------------------*/

void _ostextfield_tooltip(OSTextField *field, const char_t *text)
{
    cassert_no_null(field);
    _oscontrol_tooltip_set(i_impl(field), text);
}

/*---------------------------------------------------------------------------*/

void _ostextfield_font(OSTextField *field, const Font *font)
{
    cassert_no_null(field);
    _oscontrol_set_font(i_impl(field), &field->attrs, font);
}

/*---------------------------------------------------------------------------*/

void _ostextfield_align(OSTextField *field, const align_t align)
{
    NSTextField *impl = i_impl(field);
    cassert_no_null(field);
    [impl setAlignment:_oscontrol_text_alignment(align)];
    _oscontrol_set_align(impl, &field->attrs, align);
}

/*---------------------------------------------------------------------------*/

static void i_text_color(OSTextField *field, const color_t color)
{
    NSTextField *impl = i_impl(field);
    cassert_no_null(field);
    _oscontrol_set_text_color(impl, &field->attrs, color);
    if (field->editor != nil)
    {
        NSColor *nscolor = _oscontrol_text_color(impl, color);
        [field->editor setTextColor:nscolor];
        [impl setTextColor:nscolor];
    }
}

/*---------------------------------------------------------------------------*/

void _ostextfield_passmode(OSTextField *field, const bool_t passmode)
{
    NSTextField *impl = nil;
    cassert_no_null(field);
    cassert_no_null(field->impl);
    if (passmode == TRUE)
    {
        if ([field->impl isKindOfClass:[OSXTextField class]])
        {
            NSRect rect = [field->impl frame];
            OSXSecureTextField *nimpl = [[OSXSecureTextField alloc] initWithFrame:rect];
            [nimpl setCell:[[OSXSecureTextFieldCell alloc] init]];
            nimpl->field = field;
            impl = nimpl;
        }
    }
    else
    {
        if ([field->impl isKindOfClass:[OSXSecureTextField class]])
        {
            NSRect rect = [field->impl frame];
            OSXTextField *nimpl = [[OSXTextField alloc] initWithFrame:rect];
            [nimpl setCell:[[OSXTextFieldCell alloc] init]];
            nimpl->field = field;
            impl = nimpl;
        }
    }

    if (impl != nil)
    {
        NSString *text = [[field->impl cell] stringValue];
        [impl setEditable:[field->impl isEditable]];
        [impl setSelectable:[field->impl isSelectable]];
        [impl setBordered:[field->impl isBordered]];
        [impl setBezeled:[field->impl isBezeled]];
        [impl setDrawsBackground:[field->impl drawsBackground]];
        _oscontrol_set_font(impl, &field->attrs, field->attrs.font);
        _oscontrol_set_align(impl, &field->attrs, field->attrs.align);
        _oscontrol_set_text(impl, &field->attrs, cast_const([text UTF8String], char_t));
        [[impl cell] setScrollable:(BOOL)field->single_line];
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
        [[impl cell] setUsesSingleLineMode:(BOOL)field->single_line];
#endif
        _oscontrol_detach_from_parent(field->impl, field->control);
        field->impl = impl;
        _oscontrol_attach_to_parent(field->impl, field->control);
        i_text_color(field, field->attrs.color);
        i_update_impl(field);
    }
}

/*---------------------------------------------------------------------------*/

void _ostextfield_editable(OSTextField *field, const bool_t is_editable)
{
    cassert_no_null(field);
    [i_impl(field) setEditable:(BOOL)is_editable];
}

/*---------------------------------------------------------------------------*/

void _ostextfield_autoselect(OSTextField *field, const bool_t autoselect)
{
    cassert_no_null(field);
    field->autoselect = autoselect;
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

void _ostextfield_select(OSTextField *field, const int32_t start, const int32_t end)
{
    cassert_no_null(field);
    /* Deselect all text */
    if (start == -1 && end == 0)
    {
        field->select = NSMakeRange(0, 0);
    }
    /* Deselect all text and caret to the end */
    else if (start == -1 && end == -1)
    {
        field->select = NSMakeRange(NSUIntegerMax, 0);
    }
    /* Select all text and caret to the end */
    else if (start == 0 && end == -1)
    {
        field->select = NSMakeRange(0, NSUIntegerMax);
    }
    /* Select from position to the end */
    else if (start > 0 && end == -1)
    {
        field->select = NSMakeRange(start, NSIntegerMax);
    }
    /* Deselect all and move the caret */
    else if (start == end)
    {
        field->select = NSMakeRange(start, 0);
    }
    /* Select from start to end */
    else
    {
        field->select = NSMakeRange(start, end - start);
    }

    if (i_has_focus(i_impl(field)) == TRUE)
    {
        if (field->editor != nil)
            [field->editor setSelectedRange:field->select];
    }
}

/*---------------------------------------------------------------------------*/

void _ostextfield_color(OSTextField *field, const color_t color)
{
    i_text_color(field, color);
}

/*---------------------------------------------------------------------------*/

static void i_set_bgcolor(OSTextField *field)
{
    NSColor *nscolor = nil;
    cassert_no_null(field);
    nscolor = field->bgcolor != kCOLOR_DEFAULT ? _oscolor_NSColor(field->bgcolor) : [NSColor textBackgroundColor];
    [i_impl(field) setBackgroundColor:nscolor];
    if (field->editor != nil)
        [field->editor setBackgroundColor:nscolor];
}

/*---------------------------------------------------------------------------*/

void _ostextfield_bgcolor(OSTextField *field, const color_t color)
{
    cassert_no_null(field);
    field->bgcolor = color;
    i_set_bgcolor(field);
}

/*---------------------------------------------------------------------------*/

void _ostextfield_clipboard(OSTextField *field, const clipboard_t clipboard)
{
    cassert_no_null(field);
    if (field->editor != nil)
    {
        switch (clipboard)
        {
        case ekCLIPBOARD_COPY:
            [field->editor copy:field->editor];
            break;
        case ekCLIPBOARD_CUT:
            [field->editor cut:field->editor];
            break;
        case ekCLIPBOARD_PASTE:
            [field->editor paste:field->editor];
            break;
        default:
            cassert_default(clipboard);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _ostextfield_enabled(OSTextField *field, const bool_t enabled)
{
    NSTextField *impl = i_impl(field);
    cassert_no_null(field);
    cassert_no_null(impl);
    if (enabled == FALSE)
    {
        if (i_has_focus(impl) == TRUE)
            [[impl window] endEditingFor:impl];
    }

    _oscontrol_set_enabled(impl, enabled);
    i_text_color(field, field->attrs.color);
}

/*---------------------------------------------------------------------------*/

bool_t _ostextfield_resign_focus(const OSTextField *field)
{
    NSWindow *window = nil;
    bool_t resign = TRUE;
    cassert_no_null(field);
    window = [field->control window];
    if (field->OnChange != NULL && _oswindow_in_destroy(window) == NO)
    {
        EvText params;
        params.text = cast_const([[i_impl(field) stringValue] UTF8String], char_t);
        params.cpos = (uint32_t)[field->editor selectedRange].location;
        params.len = (int32_t)unicode_nchars(params.text, ekUTF8);
        listener_event_imp(field->OnChange, ekGUI_EVENT_TXTCHANGE, cast(field->control, void), &params, &resign, tc(field->type), "EvText", "bool_t");
    }

    if (resign == TRUE)
        [window endEditingFor:field->control];
    else
        [window makeFirstResponder:field->control];

    return resign;
}

/*---------------------------------------------------------------------------*/

void _ostextfield_focus(OSTextField *field, const bool_t focus)
{
    cassert_no_null(field);
    if (field->OnFocus != NULL)
    {
        bool_t params = focus;
        listener_event_imp(field->OnFocus, ekGUI_EVENT_FOCUS, cast(field->control, void), &params, NULL, tc(field->type), "bool_t", "void");
    }

    field->focused = focus;
    if (focus == TRUE)
    {
        NSTextField *impl = i_impl(field);
        if ([impl isEnabled] == YES)
        {
            NSWindow *window = [impl window];
            field->editor = [window fieldEditor:YES forObject:impl];
            i_set_bgcolor(field);
            if (field->autoselect == TRUE)
                [field->editor selectAll:nil];
            else
                [field->editor setSelectedRange:field->select];
        }
    }
    else
    {
        field->editor = nil;
    }

#if (defined MAC_OS_X_VERSION_10_6 && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6) || (defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
#else
    [field->control setNeedsDisplay:YES];
#endif
}

/*---------------------------------------------------------------------------*/

const char_t *_ostextfield_get_text(const OSTextField *field)
{
    NSTextField *impl = i_impl(field);
    cassert_no_null(impl);
    return cast_const([[impl stringValue] UTF8String], char_t);
}

/*---------------------------------------------------------------------------*/

const Font *_ostextfield_get_font(const OSTextField *field)
{
    cassert_no_null(field);
    return field->attrs.font;
}

/*---------------------------------------------------------------------------*/

bool_t _ostextfield_is_focused(const OSTextField *field)
{
    cassert_no_null(field);
    return field->focused;
}

/*---------------------------------------------------------------------------*/

bool_t _ostextfield_is_enabled(const OSTextField *field)
{
    NSTextField *impl = i_impl(field);
    cassert_no_null(impl);
    return (bool_t)[impl isEnabled];
}

/*---------------------------------------------------------------------------*/

NSView *_ostextfield_get_impl(OSTextField *field)
{
    cassert_no_null(field);
    return field->impl;
}
