/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: osedit_gtk.inl
 *
 */

/* Operating System edit box */

#include "osgui_gtk.ixx"

__EXTERN_C

bool_t _osedit_autosel(const OSEdit *edit);

GtkWidget *_osedit_focus_widget(OSEdit *edit);

bool_t _osedit_has_focus(const OSEdit *edit);

__END_C
