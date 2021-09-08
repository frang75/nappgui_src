/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osfont.inl
 *
 */

/* OSFont support */

#include "draw2d_gtk.ixx"

void osfont_set_default(const PangoFontDescription *fdesc);

real32_t osfont_device_to_pixels(void);

