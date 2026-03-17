/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osline.h
 *
 */

/* Operating System native line */

#include "osgui.hxx"

__EXTERN_C

_osgui_api OSLine *osline_create(const uint32_t flags);

_osgui_api void osline_destroy(OSLine **line);

_osgui_api void osline_bounds(const OSLine *line, const real32_t length, real32_t *width, real32_t *height);

_osgui_api void osline_attach(OSLine *line, OSPanel *panel);

_osgui_api void osline_detach(OSLine *line, OSPanel *panel);

_osgui_api void osline_visible(OSLine *line, const bool_t visible);

_osgui_api void osline_enabled(OSLine *line, const bool_t enabled);

_osgui_api void osline_size(const OSLine *line, real32_t *width, real32_t *height);

_osgui_api void osline_origin(const OSLine *line, real32_t *x, real32_t *y);

_osgui_api void osline_frame(OSLine *line, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
