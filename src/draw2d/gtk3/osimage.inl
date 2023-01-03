/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osimage.inl
 *
 */

/* Images */

#include "draw2d.ixx"
#include "nowarn.hxx"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "warn.hxx"

__EXTERN_C

const GdkPixbuf *osimage_pixbuf(const OSImage *image, const uint32_t frame_index);

__END_C
