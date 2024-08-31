/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ogl3dimp.c
 *
 */

/* OpenGL Context */

#include <sewer/nowarn.hxx>
#include "glew.h"
#include "wglew.h"
#include <sewer/warn.hxx>

#include "ogl3d.h"
#include "ogl3d.inl"
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

struct _oglctx_t
{
    HWND hwnd;
    HDC hdc;
    HGLRC glrc;
    GLenum glew;
};

/*---------------------------------------------------------------------------*/

static BOOL i_pixel_format(HDC hdc, const OGLProps *props)
{
    PIXELFORMATDESCRIPTOR pfd;
    int pf;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP | PFD_DOUBLEBUFFER | (props->hdaccel ? PFD_SUPPORT_OPENGL : PFD_GENERIC_FORMAT);
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = (BYTE)props->color_bpp;
    pfd.cDepthBits = (BYTE)props->depth_bpp;
    pfd.cStencilBits = (BYTE)props->stencil_bpp;
    pfd.cAuxBuffers = (BYTE)props->aux_buffers;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pf = ChoosePixelFormat(hdc, &pfd);
    if (pf == 0)
        return FALSE;
    return SetPixelFormat(hdc, pf, &pfd);
}

/*---------------------------------------------------------------------------*/

OGLCtx *_ogl3dimp_context(const OGLProps *props, void *view, oglerr_t *err)
{
    BOOL ok;
    HWND hwnd = (HWND)view;
    HDC hdc = NULL;
    HGLRC glrc = NULL;
    GLenum glew = 0;
    cassert_no_null(props);

    /* Non-screen context */
    if (hwnd == NULL)
    {
        ptr_assign(err, ekOGLFULLSCN);
        return NULL;
    }

    hdc = GetDC(hwnd);
    if (hdc == NULL)
    {
        ptr_assign(err, ekOGLVIEW);
        return NULL;
    }

    if (i_pixel_format(hdc, props) == FALSE)
    {
        ReleaseDC(hwnd, hdc);
        ptr_assign(err, ekOGLPIXFORMAT);
        return NULL;
    }

    glrc = wglCreateContext(hdc);
    if (glrc == NULL)
    {
        ReleaseDC(hwnd, hdc);
        ptr_assign(err, ekOGLCONTEXT);
        return NULL;
    }

    ok = wglMakeCurrent(hdc, glrc);
    if (!ok)
    {
        ReleaseDC(hwnd, hdc);
        wglDeleteContext(glrc);
        ptr_assign(err, ekOGLCONTEXT);
        return NULL;
    }

    glew = glewInit();
    if (glew != GLEW_OK)
    {
        ReleaseDC(hwnd, hdc);
        wglDeleteContext(glrc);
        ptr_assign(err, ekOGLGLEW);
        return NULL;
    }

    if (_ogl3dimp_check_version(props->api) == FALSE)
    {
        ReleaseDC(hwnd, hdc);
        wglDeleteContext(glrc);
        ptr_assign(err, ekOGLAPIVERS);
        return NULL;
    }

    if (props->shared != NULL)
    {
        ok = wglShareLists(props->shared->glrc, glrc);
        cassert_unref(ok, ok);
    }

    {
        OGLCtx *ctx = cast(bmem_malloc(sizeof(OGLCtx)), OGLCtx);
        WCHAR className[32];
        ctx->hdc = NULL;
        ctx->glrc = glrc;
        ctx->glew = glew;
        ctx->hwnd = hwnd;
        ReleaseDC(hwnd, hdc);

        /* Mark the window as non-GDI 'i_registry_view_class' */
        GetClassName(hwnd, className, sizeof(className));
        if (wcscmp(className, L"com.nappgui.view") == 0)
        {
            cassert(GetWindowLongPtr(hwnd, 0) == 0);
            SetWindowLongPtr(hwnd, 0, 1);
        }

        ptr_assign(err, ekOGLOK);
        return ctx;
    }
}

/*---------------------------------------------------------------------------*/

void _ogl3dimp_destroy(OGLCtx **ogl)
{
    WCHAR className[32];

    cassert_no_null(ogl);
    cassert_no_null(*ogl);

    /* Mark the window as GDI 'i_registry_view_class' */
    GetClassName((*ogl)->hwnd, className, sizeof(className));
    if (wcscmp(className, L"com.nappgui.view") == 0)
    {
        cassert(GetWindowLongPtr((*ogl)->hwnd, 0) == 1);
        SetWindowLongPtr((*ogl)->hwnd, 0, 0);
    }

    if ((*ogl)->glrc != NULL)
    {
        if (wglGetCurrentContext() == (*ogl)->glrc)
        {
            BOOL ok = wglMakeCurrent(NULL, NULL);
            cassert_unref(ok, ok);
        }

        {
            BOOL ok = wglDeleteContext((*ogl)->glrc);
            cassert_unref(ok, ok);
        }
    }

    bmem_free(*dcast(ogl, byte_t));
    *ogl = NULL;
}

/*---------------------------------------------------------------------------*/

void ogl3d_begin_draw(OGLCtx *ogl)
{
    cassert_no_null(ogl);
    cassert(ogl->hwnd != NULL);
    cassert(ogl->hdc == NULL);
    ogl->hdc = GetDC(ogl->hwnd);

    if (ogl->hdc == NULL)
        return;

    if (wglGetCurrentContext() != ogl->glrc)
    {
        BOOL ok = wglMakeCurrent(ogl->hdc, ogl->glrc);
        cassert_unref(ok, ok);
        if (__TRUE_EXPECTED(ogl->glew != GLEW_OK))
        {
            ogl->glew = glewInit();
            cassert(ogl->glew == GLEW_OK);
        }
    }
}

/*---------------------------------------------------------------------------*/

void ogl3d_end_draw(OGLCtx *ogl)
{
    cassert_no_null(ogl);
    cassert(ogl->hwnd != NULL);

    if (ogl->hdc != NULL)
    {
        BOOL ok = SwapBuffers(ogl->hdc);
        cassert_unref(ok, ok);
        ReleaseDC(ogl->hwnd, ogl->hdc);
        /* Avoid a new WM_PAINT */
        ValidateRect(ogl->hwnd, NULL);
        ogl->hdc = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void ogl3d_set_size(OGLCtx *ogl, const uint32_t width, const uint32_t height)
{
    unref(ogl);
    unref(width);
    unref(height);
}
