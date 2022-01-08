/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: button.h
 * https://nappgui.com/en/gui/button.html
 *
 */

/* Button */

#include "gui.hxx"

__EXTERN_C

Button *button_push(void);

Button *button_check(void);

Button *button_check3(void);

Button *button_radio(void);

Button *button_flat(void);

Button *button_flatgle(void);

void button_OnClick(Button *button, Listener *listener);

void button_text(Button *button, const char_t *text);

void button_text_alt(Button *button, const char_t *text);

void button_tooltip(Button *button, const char_t *text);

void button_font(Button *button, const Font *font);

void button_image(Button *button, const Image *image);

void button_image_alt(Button *button, const Image *image);

void button_state(Button *button, const state_t state);

state_t button_get_state(const Button *button);

__END_C
