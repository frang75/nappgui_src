/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osprogress.h
 *
 */

/* Operating System native progress indicator */

#include "osgui.hxx"

__EXTERN_C

OSProgress *osprogress_create(const progress_flag_t flags);

void osprogress_destroy(OSProgress **progress);

void osprogress_position(OSProgress *progress, const real32_t position);

real32_t osprogress_thickness(const OSProgress *progress, const fsize_t size);


void osprogress_attach(OSProgress *progress, OSPanel *panel);

void osprogress_detach(OSProgress *progress, OSPanel *panel);

void osprogress_visible(OSProgress *progress, const bool_t visible);

void osprogress_enabled(OSProgress *progress, const bool_t enabled);

void osprogress_size(const OSProgress *progress, real32_t *width, real32_t *height);

void osprogress_origin(const OSProgress *progress, real32_t *x, real32_t *y);

void osprogress_frame(OSProgress *progress, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
