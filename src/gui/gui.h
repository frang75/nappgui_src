/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gui.h
 * https://nappgui.com/en/gui/gui.html
 *
 */

/* Graphics User Interface */

#include "gui.hxx"

__EXTERN_C

void gui_start(void);

void gui_finish(void);

void gui_respack(FPtr_respack func_respack);

void gui_language(const char_t *lang);

const char_t *gui_text(const ResId id);

const Image *gui_image(const ResId id);

const byte_t *gui_file(const ResId id, uint32_t *size);

bool_t gui_dark_mode(void);

color_t gui_alt_color(const color_t light_color, const color_t dark_color);

color_t gui_label_color(void);

color_t gui_view_color(void);

color_t gui_line_color(void);

color_t gui_link_color(void);

color_t gui_border_color(void);

S2Df gui_resolution(void);

V2Df gui_mouse_pos(void);

void gui_OnThemeChanged(Listener *listener);

__END_C
