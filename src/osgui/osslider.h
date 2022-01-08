/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osslider.h
 *
 */

/* Operating System native slider */

#include "osgui.hxx"

__EXTERN_C

OSSlider *osslider_create(const slider_flag_t flags);

void osslider_destroy(OSSlider **slider);

void osslider_OnMoved(OSSlider *slider, Listener *listener);

void osslider_tooltip(OSSlider *slider, const char_t *text);

void osslider_tickmarks(OSSlider *slider, const uint32_t num_tickmarks, const bool_t tickmarks_at_left_top);

void osslider_position(OSSlider *slider, const real32_t position);

real32_t osslider_get_position(const OSSlider *slider);

void osslider_bounds(const OSSlider *slider, const real32_t length, const fsize_t knob_size, real32_t *width, real32_t *height);


void osslider_attach(OSSlider *slider, OSPanel *panel);

void osslider_detach(OSSlider *slider, OSPanel *panel);

void osslider_visible(OSSlider *slider, const bool_t visible);

void osslider_enabled(OSSlider *slider, const bool_t enabled);

void osslider_size(const OSSlider *slider, real32_t *width, real32_t *height);

void osslider_origin(const OSSlider *slider, real32_t *x, real32_t *y);

void osslider_frame(OSSlider *slider, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
