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

#include "osedit_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "oswindow_osx.inl"
#include "ostextfield.inl"
#include "../osedit.h"
#include "../osedit.inl"
#include <draw2d/font.h>
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXEdit : NSView
{
  @public
    uint32_t flags;
    uint32_t vpadding;
    real32_t rpadding;
    OSTextField *field;
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
    return _ostextfield_becomeFirstResponder(self->field);
}

/*---------------------------------------------------------------------------*/

- (BOOL)resignFirstResponder
{
    return _ostextfield_resignFirstResponder(self->field);
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    if (_oswindow_mouse_down(cast(self, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

/*---------------------------------------------------------------------------*/

#if (defined MAC_OS_X_VERSION_10_6 && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6) || (defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
#else

- (void)drawRect:(NSRect)rect
{
    /* Draw focus ring in older mac OSX */
    if (_ostextfield_is_focused(self->field) == TRUE)
    {
        NSSetFocusRingStyle(NSFocusRingOnly);
        NSRectFill(rect);
    }
}

#endif

@end

/*---------------------------------------------------------------------------*/

static void i_update_vpadding(OSXEdit *edit)
{
    const Font *font = NULL;
    real32_t width, height;
    uint32_t defpadding = 0;
    CGFloat wpadding = 0;
    cassert_no_null(edit);
    font = _ostextfield_get_font(edit->field);
    font_extents(font, "OO", -1.f, &width, &height);
    defpadding = (uint32_t)((.3f * height) + .5f);
    if (defpadding % 2 == 1)
        defpadding += 1;

    if (defpadding < 8)
        defpadding = 8;

    if (edit->vpadding == UINT32_MAX)
    {
        edit->rpadding = (real32_t)defpadding;
    }
    else
    {
        real32_t leading = font_leading(font);
        uint32_t padding = (uint32_t)(edit->vpadding + leading);
        padding += 4;
        edit->rpadding = (real32_t)padding;
    }

    wpadding = (CGFloat)((edit->rpadding - defpadding) / 2);

    if (wpadding < 0)
        wpadding = 0;

    _ostextfield_wpadding(edit->field, wpadding);
}

/*---------------------------------------------------------------------------*/

OSEdit *osedit_create(const uint32_t flags)
{
    OSXEdit *edit = nil;
    bool_t single_line = edit_get_type(flags) == ekEDIT_SINGLE;
    heap_auditor_add("OSXEdit");
    edit = [[OSXEdit alloc] initWithFrame:NSZeroRect];
    edit->field = _ostextfield_from_edit(edit, single_line);
    edit->flags = flags;
    edit->vpadding = UINT32_MAX;
    _oscontrol_init(edit);
    i_update_vpadding(edit);
    return cast(edit, OSEdit);
}

/*---------------------------------------------------------------------------*/

void osedit_destroy(OSEdit **edit)
{
    OSXEdit *ledit = nil;
    cassert_no_null(edit);
    ledit = *dcast(edit, OSXEdit);
    cassert_no_null(ledit);
    _ostextfield_destroy(&ledit->field);
    [ledit release];
    *edit = NULL;
    heap_auditor_delete("OSXEdit");
}

/*---------------------------------------------------------------------------*/

void osedit_OnFilter(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_OnFilter(cast(edit, OSXEdit)->field, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnChange(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_OnChange(cast(edit, OSXEdit)->field, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnFocus(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_OnFocus(cast(edit, OSXEdit)->field, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_text(OSEdit *edit, const char_t *text)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_text(cast(edit, OSXEdit)->field, text);
}

/*---------------------------------------------------------------------------*/

void osedit_tooltip(OSEdit *edit, const char_t *text)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_tooltip(cast(edit, OSXEdit)->field, text);
}

/*---------------------------------------------------------------------------*/

void osedit_font(OSEdit *edit, const Font *font)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    _ostextfield_font(ledit->field, font);
    i_update_vpadding(ledit);
}

/*---------------------------------------------------------------------------*/

void osedit_align(OSEdit *edit, const align_t align)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_align(cast(edit, OSXEdit)->field, align);
}

/*---------------------------------------------------------------------------*/

void osedit_passmode(OSEdit *edit, const bool_t passmode)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_passmode(cast(edit, OSXEdit)->field, passmode);
}

/*---------------------------------------------------------------------------*/

void osedit_editable(OSEdit *edit, const bool_t is_editable)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_editable(cast(edit, OSXEdit)->field, is_editable);
}

/*---------------------------------------------------------------------------*/

void osedit_autoselect(OSEdit *edit, const bool_t autoselect)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_autoselect(cast(edit, OSXEdit)->field, autoselect);
}

/*---------------------------------------------------------------------------*/

void osedit_select(OSEdit *edit, const int32_t start, const int32_t end)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_select(cast(edit, OSXEdit)->field, start, end);
}

/*---------------------------------------------------------------------------*/

void osedit_color(OSEdit *edit, const color_t color)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_color(cast(edit, OSXEdit)->field, color);
}

/*---------------------------------------------------------------------------*/

void osedit_bgcolor(OSEdit *edit, const color_t color)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_bgcolor(cast(edit, OSXEdit)->field, color);
}

/*---------------------------------------------------------------------------*/

void osedit_vpadding(OSEdit *edit, const real32_t padding)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    cassert_no_null(ledit);
    ledit->vpadding = (padding < 0) ? UINT32_MAX : (uint32_t)padding;
    i_update_vpadding(ledit);
}

/*---------------------------------------------------------------------------*/

void osedit_bounds(const OSEdit *edit, const real32_t refwidth, const uint32_t lines, real32_t *width, real32_t *height)
{
    OSXEdit *ledit = cast(edit, OSXEdit);
    const Font *font = NULL;
    cassert_no_null(ledit);
    cassert_no_null(width);
    cassert_no_null(height);
    cassert_unref(lines == 1, lines);
    font = _ostextfield_get_font(ledit->field);

    if (edit_get_type(ledit->flags) == ekEDIT_SINGLE)
        font_extents(font, "O", -1.f, width, height);
    else
        font_extents(font, "O\nO", -1.f, width, height);

    *width = refwidth;
    *height += ledit->rpadding;
}

/*---------------------------------------------------------------------------*/

void osedit_clipboard(OSEdit *edit, const clipboard_t clipboard)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_clipboard(cast(edit, OSXEdit)->field, clipboard);
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
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_enabled(cast(edit, OSXEdit)->field, enabled);
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
    NSView *impl = nil;
    cassert_no_null(ledit);
    impl = _ostextfield_get_impl(ledit->field);
    _oscontrol_set_frame(cast(ledit, NSView), x, y, width, height);
    _oscontrol_set_frame(impl, 1, 1, width - 2, height - 2);
    [ledit setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

bool_t _osedit_resign_focus(const OSEdit *edit)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    return _ostextfield_resign_focus(cast_const(edit, OSXEdit)->field);
}

/*---------------------------------------------------------------------------*/

void _osedit_focus(OSEdit *edit, const bool_t focus)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    _ostextfield_focus(cast(edit, OSXEdit)->field, focus);
}

/*---------------------------------------------------------------------------*/

BOOL _osedit_is(NSView *view)
{
    return [view isKindOfClass:[OSXEdit class]];
}

/*---------------------------------------------------------------------------*/

bool_t _osedit_is_enabled(NSView *edit)
{
    cassert_no_null(edit);
    cassert([cast(edit, NSObject) isKindOfClass:[OSXEdit class]] == YES);
    return _ostextfield_is_enabled(cast_const(edit, OSXEdit)->field);
}
