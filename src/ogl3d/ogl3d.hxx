/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ogl3d.hxx
 * https://nappgui.com/en/ogl3d/ogl3d.html
 *
 */

/* OpenGL */

#ifndef __OPENGL_HXX__
#define __OPENGL_HXX__

#include <sewer/sewer.hxx>
#include "ogl3d.def"

typedef struct _oglprops_t OGLProps;
typedef struct _oglctx_t OGLCtx;

typedef enum _oglapi_t
{
    ekOGL_1_1,
    ekOGL_1_2,
    ekOGL_1_2_1,
    ekOGL_1_3,
    ekOGL_1_4,
    ekOGL_1_5,
    ekOGL_2_0,
    ekOGL_2_1,
    ekOGL_3_0,
    ekOGL_3_1,
    ekOGL_3_2,
    ekOGL_3_3,
    ekOGL_4_0,
    ekOGL_4_1,
    ekOGL_4_2,
    ekOGL_4_3,
    ekOGL_4_4,
    ekOGL_4_5,
    ekOGL_4_6
} oglapi_t;

typedef enum _oglerr_t
{
    ekOGLAPIVERS = 1,
    ekOGLFULLSCN,
    ekOGLVIEW,
    ekOGLPIXFORMAT,
    ekOGLCONTEXT,
    ekOGLGLEW,
    ekOGLOK
} oglerr_t;

struct _oglprops_t
{
    oglapi_t api;
    bool_t hdaccel;
    uint32_t color_bpp;
    uint32_t depth_bpp;
    uint32_t stencil_bpp;
    uint32_t aux_buffers;
    bool_t transparent;
    OGLCtx *shared;
};

#endif
