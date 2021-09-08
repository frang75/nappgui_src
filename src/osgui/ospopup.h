/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospopup.h
 * https://nappgui.com/en/osgui/ospopup.html
 *
 */

/* Operating System native popup button */

#include "osgui.hxx"

__EXTERN_C

OSPopUp *ospopup_create(const popup_flag_t flags);

void ospopup_destroy(OSPopUp **popup);

void ospopup_OnSelect(OSPopUp *popup, Listener *listener);

void ospopup_elem(OSPopUp *popup, const op_t op, const uint32_t index, const char_t *text, const Image *image);

void ospopup_tooltip(OSPopUp *popup, const char_t *text);

void ospopup_font(OSPopUp *popup, const Font *font);

void ospopup_list_height(OSPopUp *popup, const uint32_t num_elems);

void ospopup_selected(OSPopUp *popup, const uint32_t index);

uint32_t ospopup_get_selected(const OSPopUp *popup);

void ospopup_bounds(const OSPopUp *popup, const char_t *text, real32_t *width, real32_t *height);


void ospopup_attach(OSPopUp *popup, OSPanel *panel);

void ospopup_detach(OSPopUp *popup, OSPanel *panel);

void ospopup_visible(OSPopUp *popup, const bool_t visible);

void ospopup_enabled(OSPopUp *popup, const bool_t enabled);

void ospopup_size(const OSPopUp *popup, real32_t *width, real32_t *height);

void ospopup_origin(const OSPopUp *popup, real32_t *x, real32_t *y);

void ospopup_frame(OSPopUp *popup, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
