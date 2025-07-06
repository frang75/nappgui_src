/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: label.h
 * https://nappgui.com/en/gui/label.html
 *
 */

/* Label */

#include "gui.hxx"

__EXTERN_C

_gui_api Label *label_create(void);

_gui_api void label_OnClick(Label *label, Listener *listener);

_gui_api void label_min_width(Label *label, const real32_t width);

_gui_api void label_text(Label *label, const char_t *text);

_gui_api void label_size_text(Label *label, const char_t *text);

_gui_api void label_font(Label *label, const Font *font);

_gui_api void label_style_over(Label *label, const uint32_t fstyle);

_gui_api void label_multiline(Label *label, const bool_t multiline);

_gui_api void label_align(Label *label, const align_t align);

_gui_api void label_color(Label *label, const color_t color);

_gui_api void label_color_over(Label *label, const color_t color);

_gui_api void label_bgcolor(Label *label, const color_t color);

_gui_api void label_bgcolor_over(Label *label, const color_t color);

_gui_api const char_t *label_get_text(const Label *label);

_gui_api const Font *label_get_font(const Label *label);

__END_C