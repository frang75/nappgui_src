/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: textview.h
 * https://nappgui.com/en/gui/textview.html
 *
 */

/* Text view */

#include "gui.hxx"

__EXTERN_C

TextView *textview_create(void);

void textview_size(TextView *view, const S2Df size);

void textview_clear(TextView *view);

uint32_t textview_printf(TextView *view, const char_t *format, ...);

void textview_writef(TextView *view, const char_t *text);

void textview_rtf(TextView *view, Stream *rtf_in);

void textview_units(TextView *view, const uint32_t units);

void textview_family(TextView *view, const char_t *family);

void textview_fsize(TextView *view, const real32_t size);

void textview_fstyle(TextView *view, const uint32_t fstyle);

void textview_color(TextView *view, const color_t color);

void textview_bgcolor(TextView *view, const color_t color);

void textview_pgcolor(TextView *view, const color_t color);

void textview_halign(TextView *view, const align_t align);

void textview_lspacing(TextView *view, const real32_t scale);

void textview_bfspace(TextView *view, const real32_t space);

void textview_afspace(TextView *view, const real32_t space);

void textview_scroll_down(TextView *view);

void textview_editable(TextView *view, const bool_t is_editable);

__END_C

