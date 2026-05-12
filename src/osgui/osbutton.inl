/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.inl
 *
 */

/* Operating System native button */

#include "osgui.ixx"

__EXTERN_C

bool_t _osbutton_text_allowed(const uint32_t flags);

bool_t _osbutton_image_allowed(const uint32_t flags);

void _osbutton_set_default(OSButton *button, const bool_t is_default);

void _osbutton_detach_and_destroy(OSButton **button, OSPanel *panel);

void _osbutton_flat_bounds(const char_t *text, const Font *font, const real32_t imgwidth, const real32_t imgheight, const real32_t imgsep, const gui_pos_t imgpos, const uint32_t hpadding, const uint32_t vpadding, real32_t *width, real32_t *height);

void _osbutton_flat_position(const real32_t width, const real32_t height, const real32_t imgwidth, const real32_t imgheight, const real32_t imgsep, const gui_pos_t imgpos, const real32_t twidth, const real32_t theight, real32_t *imgx, real32_t *imgy, real32_t *tx, real32_t *ty);

__END_C
