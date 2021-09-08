/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: globals.h
 * https://nappgui.com/en/gui/globals.html
 *
 */

/* System globals */

#include "gui.hxx"

__EXTERN_C

device_t globals_device(void);

void globals_resolution(S2Df *resolution);

V2Df globals_mouse_position(void);

uint32_t globals_scrollbar_width(void);

__END_C
