/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osentry_gtk.inl
 *
 */

/* GtkEntry, GtkTextView, GtkComboBox entry management */

#include "osgui_gtk.ixx"

__EXTERN_C

OSEntry *_osentry_from_entry(GtkWidget *widget, OSControl *control);

OSEntry *_osentry_from_combo(GtkWidget *widget, OSControl *control);

void _osentry_destroy(OSEntry **entry);

void _osentry_OnFilter(OSEntry *entry, Listener *listener);

void _osentry_OnChange(OSEntry *entry, Listener *listener);

void _osentry_OnFocus(OSEntry *entry, Listener *listener);

void _osentry_text(OSEntry *entry, const char_t *text);

void _osentry_font(OSEntry *entry, const Font *font);

void _osentry_align(OSEntry *entry, const align_t align);

void _osentry_passmode(OSEntry *entry, const bool_t passmode);

void _osentry_editable(OSEntry *entry, const bool_t is_editable);

void _osentry_autoselect(OSEntry *entry, const bool_t autoselect);

void _osentry_select(OSEntry *entry, const int32_t start, const int32_t end);

void _osentry_tooltip(OSEntry *entry, const char_t *text);

void _osentry_color(OSEntry *entry, const color_t color);

void _osentry_bgcolor(OSEntry *entry, const color_t color);

void _osentry_clipboard(OSEntry *entry, const clipboard_t clipboard);

void _osentry_enabled(OSEntry *entry, const bool_t enabled);

bool_t _osentry_resign_focus(OSEntry *entry);

void _osentry_focus(OSEntry *entry, const bool_t focus);

void _osentry_deselect(OSEntry *entry);

const char_t *_osentry_get_const_text(const OSEntry *entry);

const Font *_osentry_get_font(const OSEntry *entry);

bool_t _osentry_get_autoselect(const OSEntry *entry);

GtkCssProvider *_osentry_get_bgcolor(const OSEntry *entry);

GtkWidget *_osentry_get_widget(const OSEntry *entry);

__END_C
