/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscolor.inl
 *
 */

/* Color */

color_t oscolor_from_NSColor(NSColor *color);

void oscolor_NSColor_rgba(NSColor *color, CGFloat *r, CGFloat *g, CGFloat *b, CGFloat *a);

NSColor *oscolor_NSColor(const color_t color);
