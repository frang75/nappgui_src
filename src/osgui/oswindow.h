/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.h
 *
 */

/* Operating System native window */

#include "osgui.hxx"

__EXTERN_C

OSWindow *oswindow_create(const window_flag_t flags);

OSWindow *oswindow_managed(void *native_ptr);

void oswindow_destroy(OSWindow **window);

void oswindow_OnMoved(OSWindow *window, Listener *listener);

void oswindow_OnResize(OSWindow *window, Listener *listener);

void oswindow_OnClose(OSWindow *window, Listener *listener);

void oswindow_title(OSWindow *window, const char_t *text);

void oswindow_edited(OSWindow *window, const bool_t is_edited);

void oswindow_movable(OSWindow *window, const bool_t is_movable);

void oswindow_z_order(OSWindow *window, OSWindow *below_window);

void oswindow_alpha(OSWindow *window, const real32_t alpha);

void oswindow_enable_mouse_events(OSWindow *window, const bool_t enabled);

void oswindow_taborder(OSWindow *window, OSControl *control);

void oswindow_focus(OSWindow *window, OSControl *control);

void oswindow_attach_panel(OSWindow *window, OSPanel *panel);

void oswindow_detach_panel(OSWindow *window, OSPanel *panel);

void oswindow_attach_window(OSWindow *parent_window, OSWindow *child_window);

void oswindow_detach_window(OSWindow *parent_window, OSWindow *child_window);

void oswindow_launch(OSWindow *window, OSWindow *parent_window);

void oswindow_hide(OSWindow *window, OSWindow *parent_window);

uint32_t oswindow_launch_modal(OSWindow *window, OSWindow *parent_window);

void oswindow_stop_modal(OSWindow *window, const uint32_t return_value);

void oswindow_get_origin(const OSWindow *window, real32_t *x, real32_t *y);

void oswindow_origin(OSWindow *window, const real32_t x, const real32_t y);

void oswindow_get_size(const OSWindow *window, real32_t *width, real32_t *height);

void oswindow_size(OSWindow *window, const real32_t content_width, const real32_t content_height);

void oswindow_set_default_pushbutton(OSWindow *window, OSButton *button);

void oswindow_set_cursor(OSWindow *window, Cursor *cursor);

void oswindow_property(OSWindow *window, const guiprop_t property, const void *value);

__END_C
