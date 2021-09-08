/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oslabel.h
 * https://nappgui.com/en/osgui/oslabel.html
 *
 */

/* Operating System label */

#include "osgui.hxx"

__EXTERN_C

OSLabel *oslabel_create(const label_flag_t flags);

void oslabel_destroy(OSLabel **label);

void oslabel_OnClick(OSLabel *label, Listener *listener);

void oslabel_OnEnter(OSLabel *label, Listener *listener);

void oslabel_OnExit(OSLabel *label, Listener *listener);

void oslabel_text(OSLabel *label, const char_t *text);

void oslabel_font(OSLabel *label, const Font *font);

void oslabel_align(OSLabel *label, const align_t align);

void oslabel_ellipsis(OSLabel *label, const ellipsis_t ellipsis);

void oslabel_color(OSLabel *label, const color_t color);

void oslabel_bgcolor(OSLabel *label, const color_t color);

void oslabel_bounds(const OSLabel *label, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height);


void oslabel_attach(OSLabel *label, OSPanel *panel);

void oslabel_detach(OSLabel *label, OSPanel *panel);

void oslabel_visible(OSLabel *label, const bool_t visible);

void oslabel_enabled(OSLabel *label, const bool_t enabled);

void oslabel_size(const OSLabel *label, real32_t *width, real32_t *height);

void oslabel_origin(const OSLabel *label, real32_t *x, real32_t *y);

void oslabel_frame(OSLabel *label, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C
