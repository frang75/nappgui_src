/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.h
 *
 */

/* Operating System native text view */

#include "osgui.hxx"

__EXTERN_C

OSText *ostext_create(const tview_flag_t flags);

void ostext_destroy(OSText **view);

void ostext_OnTextChange(OSText *view, Listener *listener);

void ostext_insert_text(OSText *view, const char_t *text);

void ostext_set_text(OSText *view, const char_t *text);

void ostext_set_rtf(OSText *view, Stream *rtf_in);

void ostext_param(OSText *view, const guiprop_t param, const void *value);

void ostext_editable(OSText *view, const bool_t is_editable);

const char_t *ostext_get_text(const OSText *view);

void ostext_set_need_display(OSText *view);


void ostext_attach(OSText *view, OSPanel *panel);

void ostext_detach(OSText *view, OSPanel *panel);

void ostext_visible(OSText *view, const bool_t visible);

void ostext_enabled(OSText *view, const bool_t enabled);

void ostext_size(const OSText *view, real32_t *width, real32_t *height);

void ostext_origin(const OSText *view, real32_t *x, real32_t *y);

void ostext_frame(OSText *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C

