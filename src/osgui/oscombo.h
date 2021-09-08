/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscombo.h
 * https://nappgui.com/en/osgui/oscombo.html
 *
 */

/* Operating System native combo box */

#include "osgui.hxx"

__EXTERN_C

OSCombo *oscombo_create(const combo_flag_t flags);

void oscombo_destroy(OSCombo **combo);

void oscombo_OnFilter(OSCombo *combo, Listener *listener);

void oscombo_OnChange(OSCombo *combo, Listener *listener);

void oscombo_OnFocus(OSCombo *combo, Listener *listener);

void oscombo_OnSelect(OSCombo *combo, Listener *listener);

void oscombo_text(OSCombo *combo, const char_t *text);

void oscombo_tooltip(OSCombo *combo, const char_t *text);

void oscombo_font(OSCombo *combo, const Font *font);

void oscombo_align(OSCombo *combo, const align_t align);

void oscombo_passmode(OSCombo *combo, const bool_t passmode);

void oscombo_color(OSCombo *combo, const color_t color);

void oscombo_bgcolor(OSCombo *combo, const color_t color);

void oscombo_elem(OSCombo *combo, const op_t op, const uint32_t index, const char_t *text, const Image *image);

void oscombo_selected(OSCombo *combo, const uint32_t index);

uint32_t oscombo_get_selected(const OSCombo *combo);

void oscombo_bounds(const OSCombo *combo, const real32_t refwidth, real32_t *width, real32_t *height);


void oscombo_attach(OSCombo *combo, OSPanel *panel);

void oscombo_detach(OSCombo *combo, OSPanel *panel);

void oscombo_visible(OSCombo *combo, const bool_t visible);

void oscombo_enabled(OSCombo *combo, const bool_t enabled);

void oscombo_size(const OSCombo *combo, real32_t *width, real32_t *height);

void oscombo_origin(const OSCombo *combo, real32_t *x, real32_t *y);

void oscombo_frame(OSCombo *combo, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
