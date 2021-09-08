/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dctx.inl
 *
 */

/* Draw context */

#include "draw2d.hxx"

__EXTERN_C

DCtx *dctx_create(void *custom_data);

void dctx_destroy(DCtx **ctx);

void dctx_init(DCtx *ctx);

void dctx_set_gcontext(DCtx *ctx, void *gcontext, const uint32_t width, const uint32_t height, const real32_t offset_x, const real32_t offset_y, const uint32_t background, const bool_t reset);
 
void dctx_unset_gcontext(DCtx *ctx);

void dctx_size(const DCtx *ctx, uint32_t *width, uint32_t *height);

void dctx_transform(DCtx *ctx, const T2Df *t2d, const bool_t cartesian);

__END_C




