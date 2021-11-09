/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.h
 *
 */

/* Operating System edit box */

#include "osgui.hxx"

__EXTERN_C

OSEdit *osedit_create(const edit_flag_t flags);

void osedit_destroy(OSEdit **edit);

void osedit_OnFilter(OSEdit *edit, Listener *listener);

void osedit_OnChange(OSEdit *edit, Listener *listener);

void osedit_OnFocus(OSEdit *edit, Listener *listener);

void osedit_text(OSEdit *edit, const char_t *text);

void osedit_tooltip(OSEdit *edit, const char_t *text);

void osedit_font(OSEdit *edit, const Font *font);

void osedit_align(OSEdit *edit, const align_t align);

void osedit_passmode(OSEdit *edit, const bool_t passmode);

void osedit_editable(OSEdit *edit, const bool_t is_editable);

void osedit_autoselect(OSEdit *edit, const bool_t autoselect);

void osedit_color(OSEdit *edit, const color_t color);

void osedit_bgcolor(OSEdit *edit, const color_t color);

void osedit_bounds(const OSEdit *edit, const real32_t refwidth, const uint32_t lines, real32_t *width, real32_t *height);


void osedit_attach(OSEdit *edit, OSPanel *panel);

void osedit_detach(OSEdit *edit, OSPanel *panel);

void osedit_visible(OSEdit *edit, const bool_t visible);

void osedit_enabled(OSEdit *edit, const bool_t enabled);

void osedit_size(const OSEdit *edit, real32_t *width, real32_t *height);

void osedit_origin(const OSEdit *edit, real32_t *x, real32_t *y);

void osedit_frame(OSEdit *edit, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C

