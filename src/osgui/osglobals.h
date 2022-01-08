/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osglobals.h
 *
 */

/* Operating System globals */

#include "osgui.hxx"

__EXTERN_C

device_t osglobals_device(const void *non_used);

color_t osglobals_color(const syscolor_t *color);

void osglobals_resolution(const void *non_used, real32_t *width, real32_t *height);

void osglobals_mouse_position(const void *non_used, real32_t *x, real32_t *y);

Cursor *osglobals_cursor(const cursor_t cursor, const Image *image, const real32_t hot_x, const real32_t hot_y);

void osglobals_cursor_destroy(Cursor **cursor);

void osglobals_value(const uint32_t index, void *value);

__END_C
