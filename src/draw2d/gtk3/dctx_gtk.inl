/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dctx_gtk.inl
 *
 */

/* Draw context */

#include "draw2d.ixx"
#include "draw2d_gtk.ixx"

//#include <gtk/gtk.h>

__EXTERN_C

void dctx_update_view(DCtx *ctx, void *view);

void _dctx_gradient_transform(DCtx *ctx);

cairo_t *_dctx_cairo(DCtx *ctx);

//GtkStyleContext *_dctx_entry_context(void);
//
//GtkStyleContext *_dctx_button_context(void);
//
//GtkStyleContext *_dctx_check_context(void);
//
//GtkStyleContext *_dctx_table_context(void);
//
//GtkStyleContext *_dctx_header_context(void);
//
//GtkWidget *_dctx_entry(void);
//
//GdkPixbuf *_dctx_checks_bitmap(void);
//
//uint32_t _dctx_check_width(void);
//
//uint32_t _dctx_check_height(void);

__END_C




