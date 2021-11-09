/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospanel.h
 *
 */

/* Operating System native panel */

#include "osgui.hxx"

__EXTERN_C

OSPanel *ospanel_create(const uint32_t flags);

void ospanel_destroy(OSPanel **panel);

void ospanel_area(OSPanel *panel, void *obj, const color_t bgcolor, const color_t skcolor, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

void ospanel_scroller_size(const OSPanel *panel, real32_t *width, real32_t *height);

void ospanel_content_size(OSPanel *panel, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height);

void ospanel_display(OSPanel *panel);


void ospanel_attach(OSPanel *panel, OSPanel *parent_panel);

void ospanel_detach(OSPanel *panel, OSPanel *parent_panel);

void ospanel_visible(OSPanel *panel, const bool_t visible);

void ospanel_enabled(OSPanel *panel, const bool_t enabled);

void ospanel_size(const OSPanel *panel, real32_t *width, real32_t *height);

void ospanel_origin(const OSPanel *panel, real32_t *x, real32_t *y);

void ospanel_frame(OSPanel *panel, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
