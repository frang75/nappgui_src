/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscombo.m
 *
 */

/* Operating System native combo box */

#include "oscombo_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "oswindow_osx.inl"
#include "ostextfield.inl"
#include "../oscombo.h"
#include "../oscombo.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

@interface OSXCombo : NSComboBox
{
  @public
    OSTextField *field;
    bool_t launch_OnSelect;
    Listener *OnSelect;
}
@end

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
@interface OSXComboDelegate : NSObject < NSComboBoxDelegate >
#else
@interface OSXComboDelegate : NSObject
#endif
{
    uint32_t dummy;
}

@end

/*---------------------------------------------------------------------------*/

@implementation OSXCombo

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
    if (_oswindow_mouse_down(cast(self, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

@end

/*---------------------------------------------------------------------------*/

@implementation OSXComboDelegate

/*---------------------------------------------------------------------------*/

- (void)comboBoxSelectionDidChange:(NSNotification *)notification
{
    OSXCombo *combo = [notification object];
    cassert_no_null(combo);
    unref(notification);
    if (combo->launch_OnSelect == TRUE)
    {
        NSString *text = [combo objectValueOfSelectedItem];
        if (text != nil)
        {
            const char_t *utf8 = cast_const([text UTF8String], char_t);
            _ostextfield_text(combo->field, utf8);
        }

        _ostextfield_textDidChange(combo->field);

        if ([combo isEnabled] == YES && combo->OnSelect != NULL)
        {
            EvButton params;
            params.state = ekGUI_ON;
            params.index = (uint32_t)[combo indexOfSelectedItem];
            params.text = NULL;
            listener_event(combo->OnSelect, ekGUI_EVENT_BUTTON, cast(combo, OSCombo), &params, NULL, OSCombo, EvButton, void);
        }
    }
}

@end

/*---------------------------------------------------------------------------*/

OSCombo *oscombo_create(const uint32_t flags)
{
    OSXCombo *combo = nil;
    OSXComboDelegate *delegate = [[OSXComboDelegate alloc] init];
    unref(flags);
    heap_auditor_add("OSXCombo");
    combo = [[OSXCombo alloc] initWithFrame:NSZeroRect];
    combo->field = _ostextfield_from_combo(combo);
    combo->OnSelect = NULL;
    combo->launch_OnSelect = TRUE;
    _oscontrol_init(combo);
    [combo setStringValue:@""];
    [combo setUsesDataSource:NO];
    [combo setEditable:YES];
    [combo setSelectable:YES];
    [combo setHasVerticalScroller:YES];
    [combo setNumberOfVisibleItems:10];
    [combo setDelegate:delegate];
    return cast(combo, OSCombo);
}

/*---------------------------------------------------------------------------*/

void oscombo_destroy(OSCombo **combo)
{
    OSXCombo *lcombo = nil;
    cassert_no_null(combo);
    lcombo = *dcast(combo, OSXCombo);
    cassert_no_null(lcombo);
    listener_destroy(&lcombo->OnSelect);
    _ostextfield_destroy(&lcombo->field);
    [lcombo release];
    *combo = NULL;
    heap_auditor_delete("OSXCombo");
}

/*---------------------------------------------------------------------------*/

void oscombo_OnFilter(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_OnFilter(cast(combo, OSXCombo)->field, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnChange(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_OnChange(cast(combo, OSXCombo)->field, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnFocus(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_OnFocus(cast(combo, OSXCombo)->field, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnSelect(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    listener_update(&cast(combo, OSXCombo)->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_text(OSCombo *combo, const char_t *text)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_text(cast(combo, OSXCombo)->field, text);
}

/*---------------------------------------------------------------------------*/

void oscombo_tooltip(OSCombo *combo, const char_t *text)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_tooltip(cast(combo, OSXCombo)->field, text);
}

/*---------------------------------------------------------------------------*/

void oscombo_font(OSCombo *combo, const Font *font)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_font(cast(combo, OSXCombo)->field, font);
}

/*---------------------------------------------------------------------------*/

void oscombo_align(OSCombo *combo, const align_t align)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_align(cast(combo, OSXCombo)->field, align);
}

/*---------------------------------------------------------------------------*/

void oscombo_passmode(OSCombo *combo, const bool_t passmode)
{
    /*
     * Native NSComboBox doesn't support passmode.
     * A custom control have to be created.
     */
    unref(combo);
    unref(passmode);
}

/*---------------------------------------------------------------------------*/

void oscombo_editable(OSCombo *combo, const bool_t is_editable)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_editable(cast(combo, OSXCombo)->field, is_editable);
}

/*---------------------------------------------------------------------------*/

void oscombo_autoselect(OSCombo *combo, const bool_t autoselect)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_autoselect(cast(combo, OSXCombo)->field, autoselect);
}

/*---------------------------------------------------------------------------*/

void oscombo_select(OSCombo *combo, const int32_t start, const int32_t end)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_select(cast(combo, OSXCombo)->field, start, end);
}

/*---------------------------------------------------------------------------*/

void oscombo_color(OSCombo *combo, const color_t color)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_color(cast(combo, OSXCombo)->field, color);
}

/*---------------------------------------------------------------------------*/

void oscombo_bgcolor(OSCombo *combo, const color_t color)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_bgcolor(cast(combo, OSXCombo)->field, color);
}

/*---------------------------------------------------------------------------*/

void oscombo_elem(OSCombo *combo, const ctrl_op_t op, const uint32_t idx, const char_t *text, const Image *image)
{
    OSXCombo *lcombo = cast(combo, OSXCombo);
    cassert_no_null(lcombo);
    unref(image);
    lcombo->launch_OnSelect = FALSE;
    if (op != ekCTRL_OP_DEL)
    {
        NSString *str = [[NSString alloc] initWithUTF8String:cast_const(text, char)];

        switch (op)
        {
        case ekCTRL_OP_ADD:
            [lcombo addItemWithObjectValue:str];
            if (oscombo_get_selected(combo) == UINT32_MAX && str_empty_c(_ostextfield_get_text(lcombo->field)) == TRUE)
            {
                oscombo_selected(combo, 0);
            }
            break;

        case ekCTRL_OP_INS:
            [lcombo insertItemWithObjectValue:str atIndex:(NSInteger)idx];
            if (oscombo_get_selected(combo) == UINT32_MAX && str_empty_c(_ostextfield_get_text(lcombo->field)) == TRUE)
            {
                oscombo_selected(combo, 0);
            }
            break;

        case ekCTRL_OP_SET:
            [lcombo removeItemAtIndex:(NSInteger)idx];
            [lcombo insertItemWithObjectValue:str atIndex:(NSInteger)idx];
            break;

        case ekCTRL_OP_DEL:
        default:
            cassert_default(op);
        }

        [str release];
    }
    else
    {
        [lcombo removeItemAtIndex:(NSInteger)idx];
    }

    lcombo->launch_OnSelect = TRUE;
}

/*---------------------------------------------------------------------------*/

void oscombo_list_height(OSCombo *combo, const uint32_t num_elems)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    [cast(combo, OSXCombo) setNumberOfVisibleItems:(NSInteger)num_elems];
}

/*---------------------------------------------------------------------------*/

void oscombo_selected(OSCombo *combo, const uint32_t idx)
{
    OSXCombo *lcombo = cast(combo, OSXCombo);
    cassert_no_null(lcombo);
    if (idx == UINT32_MAX)
    {
        NSInteger selected = [lcombo indexOfSelectedItem];
        if (selected != -1)
            [lcombo deselectItemAtIndex:selected];
    }
    else
    {
        [lcombo selectItemAtIndex:(NSInteger)idx];
    }
}

/*---------------------------------------------------------------------------*/

uint32_t oscombo_get_selected(const OSCombo *combo)
{
    OSXCombo *lcombo = cast(combo, OSXCombo);
    cassert_no_null(lcombo);
    return (uint32_t)[lcombo indexOfSelectedItem];
}

/*---------------------------------------------------------------------------*/

void oscombo_bounds(const OSCombo *combo, const real32_t refwidth, real32_t *width, real32_t *height)
{
    OSXCombo *lcombo = cast(combo, OSXCombo);
    const Font *font = NULL;
    cassert_no_null(lcombo);
    cassert_no_null(width);
    cassert_no_null(height);
    font = _ostextfield_get_font(lcombo->field);
    font_extents(font, "OO", -1.f, width, height);
    *width = refwidth;
    *height = 27.f;
}

/*---------------------------------------------------------------------------*/

void oscombo_clipboard(OSCombo *combo, const clipboard_t clipboard)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_clipboard(cast(combo, OSXCombo)->field, clipboard);
}

/*---------------------------------------------------------------------------*/

void oscombo_attach(OSCombo *combo, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(combo, NSView));
}

/*---------------------------------------------------------------------------*/

void oscombo_detach(OSCombo *combo, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(combo, NSView));
}

/*---------------------------------------------------------------------------*/

void oscombo_visible(OSCombo *combo, const bool_t visible)
{
    _oscontrol_set_visible(cast(combo, NSView), visible);
}

/*---------------------------------------------------------------------------*/

void oscombo_enabled(OSCombo *combo, const bool_t enabled)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_enabled(cast(combo, OSXCombo)->field, enabled);
}

/*---------------------------------------------------------------------------*/

void oscombo_size(const OSCombo *combo, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast(combo, NSView), width, height);
}

/*---------------------------------------------------------------------------*/

void oscombo_origin(const OSCombo *combo, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast(combo, NSView), x, y);
}

/*---------------------------------------------------------------------------*/

void oscombo_frame(OSCombo *combo, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    NSView *lcombo = cast(combo, NSView);
    cassert_no_null(lcombo);
    _oscontrol_set_frame(lcombo, x, y, width, height);
    [lcombo setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

bool_t _oscombo_resign_focus(const OSCombo *combo)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    return _ostextfield_resign_focus(cast(combo, OSXCombo)->field);
}

/*---------------------------------------------------------------------------*/

void _oscombo_focus(OSCombo *combo, const bool_t focus)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    _ostextfield_focus(cast(combo, OSXCombo)->field, focus);
}

/*---------------------------------------------------------------------------*/

BOOL _oscombo_is(NSView *view)
{
    return [view isKindOfClass:[OSXCombo class]];
}

/*---------------------------------------------------------------------------*/

bool_t _oscombo_is_enabled(NSView *combo)
{
    cassert_no_null(combo);
    cassert([cast(combo, NSObject) isKindOfClass:[OSXCombo class]] == YES);
    return _ostextfield_is_enabled(cast_const(combo, OSXCombo)->field);
}
