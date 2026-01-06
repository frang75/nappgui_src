/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: view.inl
 *
 */

/* Custom view */

#include "gui.ixx"

__EXTERN_C

_gui_api View *_view_create(const uint32_t flags);

void _view_destroy(View **view);

const char_t *_view_subtype(const View *view);

void _view_locale(View *view);

void _view_natural(View *view, const uint32_t di, real32_t *dim0, real32_t *dim1);

void _view_OnResize(View *view, const S2Df *size);

void _view_empty(View *view);

void _view_uint32(View *view, const uint32_t value);

void _view_image(View *view, const Image *image);

__END_C
