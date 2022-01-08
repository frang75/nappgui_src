/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osupdown.inl
 *
 */

/* Operating System native updown */

#include "osgui_win.ixx"

__EXTERN_C

void _osupdown_detach_and_destroy(OSUpDown **updown, OSPanel *panel);

void _osupdown_OnNotification(OSUpDown *updown, const NMHDR *nmhdr, LPARAM lParam);

__END_C

