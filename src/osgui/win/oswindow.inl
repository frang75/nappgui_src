/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.inl
 *
 */

/* Operating System native window */

#include "osgui_win.ixx"

__EXTERN_C

void _oswindow_set_menubar(OSWindow *window, HMENU hmenu);

void _oswindow_unset_menubar(OSWindow *window, HMENU hmenu);

void _oswindow_change_menubar(OSWindow *window, HMENU prev_hmenu, HMENU new_hmenu);

HWND _oswindow_set_current_popup_menu(OSWindow *window, HMENU hmenu);

HWND _oswindow_hwnd(OSWindow *window);

bool_t _oswindow_proccess_message(MSG *msg, HACCEL accelerator_table);

bool_t _oswindow_in_resizing(HWND child_hwnd);

void _oswindow_focus(OSControl *control);

__END_C

