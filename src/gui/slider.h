/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: slider.h
 * https://nappgui.com/en/gui/slider.html
 *
 */

/* Slider */

#include "gui.hxx"

__EXTERN_C

Slider *slider_create(void);

Slider *slider_vertical(void);

void slider_OnMoved(Slider *slider, Listener *listener);

void slider_tooltip(Slider *slider, const char_t *text);

void slider_steps(Slider *slider, const uint32_t steps);

void slider_value(Slider *slider, const real32_t value);

real32_t slider_get_value(const Slider *slider);

__END_C

