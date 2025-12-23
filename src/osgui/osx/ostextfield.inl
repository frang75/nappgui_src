/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostextfield.inl
 *
 */

/* Edit common implementation for OSXEdit and OSXCombo */

#include "osgui_osx.ixx"

__EXTERN_C

OSTextField *_ostextfield_from_edit(NSView *control, const bool_t single_line);

OSTextField *_ostextfield_from_combo(NSView *control);

void _ostextfield_destroy(OSTextField **field);

BOOL _ostextfield_becomeFirstResponder(OSTextField *field);

BOOL _ostextfield_resignFirstResponder(OSTextField *field);

void _ostextfield_textDidChange(OSTextField *field);

void _ostextfield_textDidEndEditing(OSTextField *field, NSNotification *notification);

void _ostextfield_wpadding(OSTextField *field, CGFloat wpadding);

void _ostextfield_OnFilter(OSTextField *field, Listener *listener);

void _ostextfield_OnChange(OSTextField *field, Listener *listener);

void _ostextfield_OnFocus(OSTextField *field, Listener *listener);

void _ostextfield_text(OSTextField *field, const char_t *text);

void _ostextfield_tooltip(OSTextField *field, const char_t *text);

void _ostextfield_font(OSTextField *field, const Font *font);

void _ostextfield_align(OSTextField *field, const align_t align);

void _ostextfield_passmode(OSTextField *field, const bool_t passmode);

void _ostextfield_editable(OSTextField *field, const bool_t is_editable);

void _ostextfield_autoselect(OSTextField *field, const bool_t autoselect);

void _ostextfield_select(OSTextField *field, const int32_t start, const int32_t end);

void _ostextfield_color(OSTextField *field, const color_t color);

void _ostextfield_bgcolor(OSTextField *field, const color_t color);

void _ostextfield_clipboard(OSTextField *field, const clipboard_t clipboard);

void _ostextfield_enabled(OSTextField *field, const bool_t enabled);

bool_t _ostextfield_resign_focus(const OSTextField *field);

void _ostextfield_focus(OSTextField *field, const bool_t focus);

const char_t *_ostextfield_get_text(const OSTextField *field);

const Font *_ostextfield_get_font(const OSTextField *field);

bool_t _ostextfield_is_focused(const OSTextField *field);

bool_t _ostextfield_is_enabled(const OSTextField *field);

NSView *_ostextfield_get_impl(OSTextField *field);

__END_C
