/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: window.h
 * https://nappgui.com/en/gui/window.html
 *
 */

/* Windows */

#include "gui.hxx"

__EXTERN_C

Window *window_create(const uint32_t flags);

void window_destroy(Window **window);

void window_panel(Window *window, Panel *panel);

void window_OnClose(Window *window, Listener *listener);

void window_OnMoved(Window *window, Listener *listener);

void window_OnResize(Window *window, Listener *listener);

void window_title(Window *window, const char_t *text);

void window_show(Window *window);

void window_hide(Window *window);

uint32_t window_modal(Window *window, Window *parent);

void window_stop_modal(Window *window, const uint32_t return_value);

void window_update(Window *window);

void window_origin(Window *window, const V2Df origin);

void window_size(Window *window, const S2Df size);

V2Df window_get_origin(const Window *window);

S2Df window_get_size(const Window *window);

S2Df window_get_client_size(const Window *window);

void window_defbutton(Window *window, Button *button);

void window_cursor(Window *window, const cursor_t cursor, const Image *image, const real32_t hot_x, const real32_t hot_y);

__END_C
