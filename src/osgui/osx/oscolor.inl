/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscolor.inl
 *
 */

/* Color */

#include "osgui_osx.ixx"

void _oscolor_init(void);

void _oscolor_finish(void);

color_t _oscolor_from_NSColor(NSColor *color);

void _oscolor_NSColor_rgba(NSColor *color, CGFloat *r, CGFloat *g, CGFloat *b, CGFloat *a);

void _oscolor_NSColor_rgba_v(NSColor *color, CGFloat *rgba);

NSColor *_oscolor_NSColor(const color_t color);
