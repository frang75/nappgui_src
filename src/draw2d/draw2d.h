/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw2d.h
 * https://nappgui.com/en/draw2d/draw2d.html
 *
 */

/* Operating system 2D drawing support */

#include "draw2d.hxx"

__EXTERN_C

void draw2d_start(void);

void draw2d_finish(void);

#define resid_image(resid) (const Image*)(resid)

__END_C
