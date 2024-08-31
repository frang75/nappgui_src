/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ogl3d.h
 *
 */

/* OpenGL Context */

#include "ogl3d.hxx"

__EXTERN_C

_ogl3d_api void ogl3d_start(void);

_ogl3d_api void ogl3d_finish(void);

_ogl3d_api OGLCtx *ogl3d_context(const OGLProps *props, void *view, oglerr_t *err);

_ogl3d_api void ogl3d_destroy(OGLCtx **ogl);

_ogl3d_api void ogl3d_begin_draw(OGLCtx *ogl);

_ogl3d_api void ogl3d_end_draw(OGLCtx *ogl);

_ogl3d_api void ogl3d_set_size(OGLCtx *ogl, const uint32_t width, const uint32_t height);

_ogl3d_api const char_t *ogl3d_err_str(const oglerr_t err);

__END_C
