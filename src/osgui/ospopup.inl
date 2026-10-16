/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: ospopup.inl
 *
 */

/* Operating System native popup button */

#include "osgui.ixx"

__EXTERN_C

void _ospopup_detach_and_destroy(OSPopUp **popup, OSPanel *panel);

void _ospopup_focus(OSPopUp *popup, const bool_t focus);

__END_C
