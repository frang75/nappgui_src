/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ogl3d.inl
 *
 */

/* OpenGL */

#include "ogl3d.hxx"

__EXTERN_C

OGLCtx *_ogl3dimp_context(const OGLProps *props, void *view, oglerr_t *err);

void _ogl3dimp_destroy(OGLCtx **ogl);

bool_t _ogl3dimp_check_version(const oglapi_t api);

__END_C
