/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.h
 * https://nappgui.com/en/osgui/ossplit.html
 *
 */

/* Operating System split view */

#include "osgui.hxx"

__EXTERN_C

OSSplit *ossplit_create(const split_flag_t flags);

void ossplit_destroy(OSSplit **view);

void ossplit_attach_control(OSSplit *view, OSControl *control);

void ossplit_detach_control(OSSplit *view, OSControl *control);

void ossplit_OnMoved(OSSplit *view, Listener *listener);

void ossplit_track_area(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height);


void ossplit_attach(OSSplit *view, OSPanel *panel);

void ossplit_detach(OSSplit *view, OSPanel *panel);

void ossplit_visible(OSSplit *view, const bool_t is_visible);

void ossplit_enabled(OSSplit *view, const bool_t is_enabled);

void ossplit_size(const OSSplit *view, real32_t *width, real32_t *height);

void ossplit_origin(const OSSplit *view, real32_t *x, real32_t *y);

void ossplit_frame(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C

