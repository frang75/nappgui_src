/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: comwin.h
 * https://nappgui.com/en/gui/comwin.html
 *
 */

/* Common windows */

#include "gui.hxx"

__EXTERN_C

const char_t *comwin_open_file(Window *parent, const char_t **ftypes, const uint32_t size, const char_t *start_dir);

const char_t *comwin_save_file(Window *parent, const char_t **ftypes, const uint32_t size, const char_t *start_dir);

void comwin_color(Window *parent, const char_t *title, const real32_t x, const real32_t y, const align_t halign, const align_t valign, const color_t current, color_t *colors, const uint32_t n, Listener *OnChange);

__END_C
