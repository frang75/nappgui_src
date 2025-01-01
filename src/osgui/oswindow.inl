/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.inl
 *
 */

/* Operating System native window */

#include "osgui.ixx"

__EXTERN_C

void _oswindow_widget_set_focus(OSWindow *window, OSWidget *widget);

OSButton *_oswindow_apply_default_button(OSWindow *window, OSButton *button);

void _oswindow_find_all_controls(OSWindow *window, ArrPt(OSControl) *controls);

const ArrPt(OSControl) *_oswindow_get_all_controls(const OSWindow *window);

void _oswindow_hotkey_destroy(ArrSt(OSHotKey) **hotkeys);

void _oswindow_hotkey_set(ArrSt(OSHotKey) **hotkeys, const vkey_t key, const uint32_t modifiers, Listener *listener);

bool_t _oswindow_hotkey_process(OSWindow *window, ArrSt(OSHotKey) *hotkeys, const vkey_t key, const uint32_t modifiers);

void _oswindow_set_app(void *app, void *icon);

void _oswindow_set_app_terminate(void);

__END_C
