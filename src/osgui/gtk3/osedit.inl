/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.inl
 *
 */

/* Operating System edit box */

#include "osgui_gtk.ixx"

__EXTERN_C

void _osedit_detach_and_destroy(OSEdit **edit, OSPanel *panel);

void _osedit_set_focus(OSEdit *edit);

void _osedit_unset_focus(OSEdit *edit);

bool_t _osedit_autoselect(const OSEdit *edit);

GtkWidget *_osedit_focus(OSEdit *edit);

__END_C

