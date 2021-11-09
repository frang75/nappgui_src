/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui.h
 *
 */

/* Operating system native gui */

#include "osgui.hxx"

__EXTERN_C

void osgui_start(void);

void osgui_finish(void);

void osgui_set_menubar(OSMenu *menu, OSWindow *window);

void osgui_unset_menubar(OSMenu *menu, OSWindow *window);

void osgui_redraw_menubar(void);

__END_C

