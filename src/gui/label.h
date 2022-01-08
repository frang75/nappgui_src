/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
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

Label *label_create(void);

Label *label_multiline(void);

void label_OnClick(Label *label, Listener *listener);

void label_text(Label *label, const char_t *text);

void label_font(Label *label, const Font *font);

void label_style_over(Label *label, const uint32_t fstyle);

void label_align(Label *label, const align_t align);

void label_color(Label *label, const color_t color);

void label_color_over(Label *label, const color_t color);

void label_bgcolor(Label *label, const color_t color);

void label_bgcolor_over(Label *label, const color_t color);

__END_C

