/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabstop.inl
 *
 */

/* Tabstops functionality */

#include "osgui.ixx"

__EXTERN_C

void _ostabstop_init(OSTabStop *tabstop, OSWindow *window);

void _ostabstop_remove(OSTabStop *tabstop);

bool_t _ostabstop_can_close_window(OSTabStop *tabstop);

bool_t _ostabstop_capture_return(OSTabStop *tabstop);

bool_t _ostabstop_capture_tab(OSTabStop *tabstop);

void _ostabstop_list_add(OSTabStop *tabstop, OSControl *control);

gui_focus_t _ostabstop_next(OSTabStop *tabstop, const bool_t from_key);

gui_focus_t _ostabstop_prev(OSTabStop *tabstop, const bool_t from_key);

gui_focus_t _ostabstop_move(OSTabStop *tabstop, OSControl *control);

gui_focus_t _ostabstop_restore(OSTabStop *tabstop);

gui_tab_t _ostabstop_info_focus(const OSTabStop *tabstop, void **next_ctrl);

bool_t _ostabstop_mouse_down(OSTabStop *tabstop, OSControl *control);

void _ostabstop_release_transient(OSTabStop *tabstop, OSControl *control);

__END_C
