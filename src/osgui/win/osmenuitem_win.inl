/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenuitem_win.inl
 *
 */

/* Operating System native menu item */

#include "osgui_win.ixx"

__EXTERN_C

void _osmenuitem_append_to_hmenu(OSMenuItem *item, OSMenu *menu);

void _osmenuitem_unset_parent(OSMenuItem *item, OSMenu *menu);

void _osmenuitem_click(OSMenuItem *item, UINT id, UINT type, UINT state);

__END_C
