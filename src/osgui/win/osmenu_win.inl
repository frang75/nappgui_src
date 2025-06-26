/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu_win.inl
 *
 */

/* Operating System native menu */

#include "osgui_win.ixx"

__EXTERN_C

HMENU _osmenu_hmenu(OSMenu *menu);

HMENU _osmenu_menubar(OSMenu *menu, OSWindow *window);

HMENU _osmenu_menubar_unlink(OSMenu *menu, OSWindow *window);

void _osmenu_hmenu_recompute(OSMenu *menu);

void _osmenu_hmenu_redraw(OSMenu *menu);

void _osmenu_attach_to_item(OSMenu *menu, OSMenuItem *item);

void _osmenu_detach_from_item(OSMenu *menu, OSMenuItem *item);

__END_C
