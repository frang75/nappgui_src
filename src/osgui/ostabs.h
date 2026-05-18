/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabs.h
 *
 */

/* Operating System native tabctrl */

#include "osgui.hxx"

__EXTERN_C

_osgui_api OSTabs *ostabs_create(const uint32_t flags);

_osgui_api void ostabs_destroy(OSTabs **tabs);

_osgui_api void ostabs_OnSelect(OSTabs *tabs, Listener *listener);

_osgui_api void ostabs_tooltip(OSTabs *tabs, const char_t *text);

_osgui_api void ostabs_font(OSTabs *tabs, const Font *font);

_osgui_api void ostabs_elem(OSTabs *tabs, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image);

_osgui_api void ostabs_selected(OSTabs *tabs, const uint32_t index);

_osgui_api uint32_t ostabs_get_selected(const OSTabs *tabs);

_osgui_api void ostabs_bounds(const OSTabs *tabs, const real32_t length, real32_t *width, real32_t *height);

_osgui_api void ostabs_attach(OSTabs *tabs, OSPanel *panel);

_osgui_api void ostabs_detach(OSTabs *tabs, OSPanel *panel);

_osgui_api void ostabs_visible(OSTabs *tabs, const bool_t visible);

_osgui_api void ostabs_enabled(OSTabs *tabs, const bool_t enabled);

_osgui_api void ostabs_size(const OSTabs *tabs, real32_t *width, real32_t *height);

_osgui_api void ostabs_origin(const OSTabs *tabs, real32_t *x, real32_t *y);

_osgui_api void ostabs_frame(OSTabs *tabs, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
