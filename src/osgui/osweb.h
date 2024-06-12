/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osweb.h
 *
 */

/* Operating System native web view */

#include "osgui.hxx"

__EXTERN_C

_osgui_api OSWeb *osweb_create(const uint32_t flags);

_osgui_api void osweb_destroy(OSWeb **view);

_osgui_api void osweb_OnFocus(OSWeb *view, Listener *listener);

_osgui_api void osweb_command(OSWeb *view, const gui_web_t cmd, const void *param, void *result);

_osgui_api void osweb_scroller_visible(OSWeb *view, const bool_t horizontal, const bool_t vertical);

_osgui_api void osweb_set_need_display(OSWeb *view);

_osgui_api void osweb_clipboard(OSWeb *view, const clipboard_t clipboard);

_osgui_api void osweb_attach(OSWeb *view, OSPanel *panel);

_osgui_api void osweb_detach(OSWeb *view, OSPanel *panel);

_osgui_api void osweb_visible(OSWeb *view, const bool_t visible);

_osgui_api void osweb_enabled(OSWeb *view, const bool_t enabled);

_osgui_api void osweb_size(const OSWeb *view, real32_t *width, real32_t *height);

_osgui_api void osweb_origin(const OSWeb *view, real32_t *x, real32_t *y);

_osgui_api void osweb_frame(OSWeb *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
