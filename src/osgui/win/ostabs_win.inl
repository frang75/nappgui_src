/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: ostabs_win.inl
 *
 */

/* Operating System native tabctrl */

#include "osgui_win.ixx"

__EXTERN_C

void _ostabs_update_dpi(OSTabs *tabs);

void _ostabs_OnNotification(OSTabs *tabs, const NMHDR *nmhdr);

__END_C
