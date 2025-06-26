/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu.h
 *
 */

/* Operating System native menu */

#include "osgui.hxx"

__EXTERN_C

_osgui_api OSMenu *osmenu_create(const uint32_t flags);

_osgui_api void osmenu_destroy(OSMenu **menu);

_osgui_api void osmenu_insert_item(OSMenu *menu, const uint32_t pos, OSMenuItem *item);

_osgui_api void osmenu_delete_item(OSMenu *menu, OSMenuItem *item);

_osgui_api void osmenu_launch(OSMenu *menu, OSWindow *window, const real32_t x, const real32_t y);

_osgui_api void osmenu_hide(OSMenu *menu);

_osgui_api bool_t osmenu_is_menubar(const OSMenu *menu);

__END_C
