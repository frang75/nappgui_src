/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.h
 * https://nappgui.com/en/osgui/osbutton.html
 *
 */

/* Operating System native button */

#include "osgui.hxx"

__EXTERN_C

OSButton *osbutton_create(const button_flag_t flags);

void osbutton_destroy(OSButton **button);

void osbutton_OnClick(OSButton *button, Listener *listener);

void osbutton_text(OSButton *button, const char_t *text);

void osbutton_tooltip(OSButton *button, const char_t *text);

void osbutton_font(OSButton *button, const Font *font);

void osbutton_align(OSButton *button, const align_t align);

void osbutton_image(OSButton *button, const Image *image);

void osbutton_state(OSButton *button, const state_t state);

state_t osbutton_get_state(const OSButton *button);

void osbutton_bounds(const OSButton *button, const char_t *text, const real32_t refwidth, const real32_t refheight, real32_t *width, real32_t *height);


void osbutton_attach(OSButton *button, OSPanel *panel);

void osbutton_detach(OSButton *button, OSPanel *panel);

void osbutton_visible(OSButton *button, const bool_t visible);

void osbutton_enabled(OSButton *button, const bool_t enabled);

void osbutton_size(const OSButton *button, real32_t *width, real32_t *height);

void osbutton_origin(const OSButton *button, real32_t *x, real32_t *y);

void osbutton_frame(OSButton *button, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C

