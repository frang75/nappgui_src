/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ogl3dimp.m
 *
 */

/* OpenGL support macos */

#include "glew.h"
#include <Cocoa/Cocoa.h>
#include "ogl3d.inl"
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

#if defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

struct _oglctx_t
{
    NSView *view;
    NSOpenGLContext *nsgl;
    GLenum glew;
};

/*---------------------------------------------------------------------------*/

static NSOpenGLPixelFormat *i_pixel_format(const OGLProps *props)
{
    NSOpenGLPixelFormatAttribute attribs[32];
    uint32_t i = 0;

    cassert_no_null(props);
    if (props->hdaccel == TRUE)
    {
        attribs[i++] = NSOpenGLPFAAccelerated;
        attribs[i++] = NSOpenGLPFANoRecovery;
    }

#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
    switch (props->api)
    {
    case ekOGL_1_1:
    case ekOGL_1_2:
    case ekOGL_1_2_1:
    case ekOGL_1_3:
    case ekOGL_1_4:
    case ekOGL_1_5:
    case ekOGL_2_0:
    case ekOGL_2_1:
    case ekOGL_3_0:
    case ekOGL_3_1:
        attribs[i++] = NSOpenGLPFAOpenGLProfile;
        attribs[i++] = NSOpenGLProfileVersionLegacy;
        break;
    case ekOGL_3_2:
    case ekOGL_3_3:
    case ekOGL_4_0:
        attribs[i++] = NSOpenGLPFAOpenGLProfile;
        attribs[i++] = NSOpenGLProfileVersion3_2Core;
        break;
    case ekOGL_4_1:
    case ekOGL_4_2:
    case ekOGL_4_3:
    case ekOGL_4_4:
    case ekOGL_4_5:
    case ekOGL_4_6:
        attribs[i++] = NSOpenGLPFAOpenGLProfile;
#if defined(MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
        attribs[i++] = NSOpenGLProfileVersion4_1Core;
#else
        attribs[i++] = NSOpenGLProfileVersion3_2Core;
#endif
        break;
        cassert_default();
    }
#endif

    // attribs[i++] = NSOpenGLPFAMinimumPolicy;
#if defined(MAC_OS_X_VERSION_10_9) && MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_9
    attribs[i++] = NSOpenGLPFAWindow;
#endif

    attribs[i++] = NSOpenGLPFADoubleBuffer;
    /* attribs[i++] = NSOpenGLPFABackingStore;*/
    attribs[i++] = NSOpenGLPFAColorSize;
    attribs[i++] = (NSOpenGLPixelFormatAttribute)props->color_bpp;
    attribs[i++] = NSOpenGLPFAAuxBuffers;
    attribs[i++] = (NSOpenGLPixelFormatAttribute)props->aux_buffers;
    attribs[i++] = NSOpenGLPFADepthSize;
    attribs[i++] = (NSOpenGLPixelFormatAttribute)props->depth_bpp;
    attribs[i++] = NSOpenGLPFAStencilSize;
    attribs[i++] = (NSOpenGLPixelFormatAttribute)props->stencil_bpp;
    attribs[i] = 0;

    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
}

/*---------------------------------------------------------------------------*/

#include <sewer/nowarn.hxx>

static ___INLINE void i_set_opengl(NSView *view)
{
    if (view != NULL)
        [view NAppGUIOSX_setOpenGL];
}

/*---------------------------------------------------------------------------*/

static ___INLINE void i_unset_opengl(NSView *view)
{
    if (view != NULL)
        [view NAppGUIOSX_unsetOpenGL];
}

#include <sewer/warn.hxx>

/*---------------------------------------------------------------------------*/

OGLCtx *_ogl3dimp_context(const OGLProps *props, void *view, oglerr_t *err)
{
    NSOpenGLPixelFormat *pixformat = NULL;
    NSOpenGLContext *nsgl = NULL;
    NSOpenGLContext *snsgl = NULL;
    GLenum glew;
    cassert_no_null(props);

    /* Non-screen context */
    if (view == NULL)
    {
        ptr_assign(err, ekOGLFULLSCN);
        return NULL;
    }

    pixformat = i_pixel_format(props);
    if (pixformat == NULL)
    {
        ptr_assign(err, ekOGLPIXFORMAT);
        return NULL;
    }

    if (props->shared != NULL)
        snsgl = props->shared->nsgl;

    nsgl = [[NSOpenGLContext alloc] initWithFormat:pixformat shareContext:snsgl];
    [pixformat release];
    if (nsgl == nil)
    {
        ptr_assign(err, ekOGLCONTEXT);
        return NULL;
    }

    [nsgl makeCurrentContext];
    glew = glewInit();
    if (glew != GLEW_OK)
    {
        [NSOpenGLContext clearCurrentContext];
        [nsgl release];
        ptr_assign(err, ekOGLGLEW);
        return NULL;
    }

    if (_ogl3dimp_check_version(props->api) == FALSE)
    {
        [NSOpenGLContext clearCurrentContext];
        [nsgl release];
        ptr_assign(err, ekOGLAPIVERS);
        return NULL;
    }

    {
        GLint params = 1;
        [nsgl setValues:&params forParameter:NSOpenGLCPSwapInterval];
    }

    {
        OGLCtx *ctx = cast(bmem_malloc(sizeof(OGLCtx)), OGLCtx);
        ctx->view = (NSView *)view;
        ctx->nsgl = nsgl;
        ctx->glew = glew;
        i_set_opengl(ctx->view);
        ptr_assign(err, ekOGLOK);
        return ctx;
    }
}

/*---------------------------------------------------------------------------*/

void _ogl3dimp_destroy(OGLCtx **ogl)
{
    cassert_no_null(ogl);
    cassert_no_null(*ogl);

    [(*ogl)->nsgl setView:nil];
    i_unset_opengl((*ogl)->view);

    if ((*ogl)->nsgl != nil)
    {
        if ([NSOpenGLContext currentContext] == (*ogl)->nsgl)
            [NSOpenGLContext clearCurrentContext];
        [(*ogl)->nsgl release];
    }

    bmem_free(*dcast(ogl, byte_t));
}

/*---------------------------------------------------------------------------*/

void ogl3d_begin_draw(OGLCtx *ogl)
{
    cassert_no_null(ogl);
    if ([ogl->nsgl view] != ogl->view)
        [ogl->nsgl setView:ogl->view];

    if ([NSOpenGLContext currentContext] != ogl->nsgl)
        [ogl->nsgl makeCurrentContext];
}

/*---------------------------------------------------------------------------*/

void ogl3d_end_draw(OGLCtx *ogl)
{
    cassert_no_null(ogl);
    [ogl->nsgl flushBuffer];
}

/*---------------------------------------------------------------------------*/

void ogl3d_set_size(OGLCtx *ogl, const uint32_t width, const uint32_t height)
{
    cassert_no_null(ogl);
    unref(width);
    unref(height);
    [ogl->nsgl update];
}
