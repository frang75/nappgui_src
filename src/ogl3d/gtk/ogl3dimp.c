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

#include "ogl3d.h"
#include "ogl3d.inl"
#include <sewer/bmem.h>
#include <sewer/blib.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#include <sewer/nowarn.hxx>
#include <EGL/egl.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "glew.h"
#include <sewer/warn.hxx>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _oglctx_t
{
    OGLProps props;
    GtkWidget *widget;
    EGLDisplay *display;
    EGLSurface *surface;
    EGLContext *context;
    GLenum glew;
    oglerr_t err;
};

/*---------------------------------------------------------------------------*/
/* https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglChooseConfig.xhtml */
static EGLBoolean i_egl_pixel_format(const OGLProps *props, EGLDisplay *display, EGLConfig *config)
{
    EGLint attrib[32];
    EGLint i = 0, nconfig;
    cassert_no_null(props);
    attrib[i++] = EGL_RENDERABLE_TYPE;
    attrib[i++] = EGL_OPENGL_BIT;

    attrib[i++] = EGL_CONFIG_CAVEAT;
    if (props->hdaccel == TRUE)
        attrib[i++] = EGL_NONE;
    else
        attrib[i++] = EGL_SLOW_CONFIG;

    attrib[i++] = EGL_BUFFER_SIZE;
    attrib[i++] = (EGLint)props->color_bpp;
    attrib[i++] = EGL_DEPTH_SIZE;
    attrib[i++] = (EGLint)(props->depth_bpp > 0 ? 24 : 0);
    attrib[i++] = EGL_STENCIL_SIZE;
    attrib[i++] = (EGLint)(props->stencil_bpp > 0 ? 8 : 0);

    attrib[i++] = EGL_TRANSPARENT_TYPE;
    if (props->transparent == TRUE)
        attrib[i++] = EGL_TRANSPARENT_RGB;
    else
        attrib[i++] = EGL_NONE;

    attrib[i++] = EGL_NONE;

    return eglChooseConfig(display, attrib, config, 1, &nconfig);
}

/*---------------------------------------------------------------------------*/

#if defined(EGL_VERSION_1_5)

static EGLint i_egl_major(const oglapi_t api)
{
    switch (api)
    {
    case ekOGL_1_1:
    case ekOGL_1_2:
    case ekOGL_1_2_1:
    case ekOGL_1_3:
    case ekOGL_1_4:
    case ekOGL_1_5:
        return 1;
    case ekOGL_2_0:
    case ekOGL_2_1:
        return 2;
    case ekOGL_3_0:
    case ekOGL_3_1:
    case ekOGL_3_2:
    case ekOGL_3_3:
        return 3;
    case ekOGL_4_0:
    case ekOGL_4_1:
    case ekOGL_4_2:
    case ekOGL_4_3:
    case ekOGL_4_4:
    case ekOGL_4_5:
    case ekOGL_4_6:
        return 4;
        cassert_default();
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static EGLint i_egl_minor(const oglapi_t api)
{
    switch (api)
    {
    case ekOGL_1_1:
        return 1;
    case ekOGL_1_2:
    case ekOGL_1_2_1:
        return 2;
    case ekOGL_1_3:
        return 3;
    case ekOGL_1_4:
        return 4;
    case ekOGL_1_5:
        return 5;
    case ekOGL_2_0:
        return 0;
    case ekOGL_2_1:
        return 1;
    case ekOGL_3_0:
        return 0;
    case ekOGL_3_1:
        return 1;
    case ekOGL_3_2:
        return 2;
    case ekOGL_3_3:
        return 3;
    case ekOGL_4_0:
        return 0;
    case ekOGL_4_1:
        return 1;
    case ekOGL_4_2:
        return 2;
    case ekOGL_4_3:
        return 3;
    case ekOGL_4_4:
        return 4;
    case ekOGL_4_5:
        return 5;
    case ekOGL_4_6:
        return 6;
        cassert_default();
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static EGLint i_egl_profile(const oglapi_t api)
{
    switch (api)
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
        return EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;
    case ekOGL_3_2:
    case ekOGL_3_3:
    case ekOGL_4_0:
    case ekOGL_4_1:
    case ekOGL_4_2:
    case ekOGL_4_3:
    case ekOGL_4_4:
    case ekOGL_4_5:
    case ekOGL_4_6:
        return EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
        cassert_default();
    }

    return EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;
}

#endif

/*---------------------------------------------------------------------------*/

static void i_egl_config(GtkWidget *widget, OGLCtx *ogl)
{
    GdkDisplay *gdk_display;
    Display *x11_display;
    GdkWindow *gdk_window;
    EGLBoolean ok;
    EGLConfig config;
    EGLContext *sctx = NULL;
    EGLint attribs[32], i = 0;

    cassert_no_null(ogl);
    gdk_display = gtk_widget_get_display(widget);
    x11_display = gdk_x11_display_get_xdisplay(gdk_display);
    gdk_window = gtk_widget_get_window(widget);

    ogl->display = eglGetDisplay((EGLNativeDisplayType)x11_display);
    if (ogl->display == EGL_NO_DISPLAY)
    {
        ogl->err = ekOGLVIEW;
        return;
    }

    ok = eglInitialize(ogl->display, NULL, NULL);
    if (ok != EGL_TRUE)
    {
        ogl->err = ekOGLCONTEXT;
        return;
    }

    if (i_egl_pixel_format(&ogl->props, ogl->display, &config) == EGL_FALSE)
    {
        ogl->err = ekOGLPIXFORMAT;
        return;
    }

    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
    {
        ogl->err = ekOGLCONTEXT;
        return;
    }

    ogl->surface = eglCreateWindowSurface(ogl->display, config, gdk_x11_window_get_xid(gdk_window), NULL);
    if (ogl->surface == EGL_NO_SURFACE)
    {
        ogl->err = ekOGLVIEW;
        return;
    }

    if (ogl->props.shared != NULL)
        sctx = ogl->props.shared->context;

#if defined(EGL_VERSION_1_5)
    attribs[i++] = EGL_CONTEXT_MAJOR_VERSION;
    attribs[i++] = i_egl_major(ogl->props.api);
    attribs[i++] = EGL_CONTEXT_MINOR_VERSION;
    attribs[i++] = i_egl_minor(ogl->props.api);
    attribs[i++] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
    attribs[i++] = i_egl_profile(ogl->props.api);
#endif

    attribs[i++] = EGL_NONE;

    ogl->context = eglCreateContext(ogl->display, config, sctx, attribs);
    if (ogl->context == EGL_NO_CONTEXT)
    {
        ogl->err = ekOGLCONTEXT;
        return;
    }

    eglMakeCurrent(ogl->display, ogl->surface, ogl->surface, ogl->context);
    ogl->glew = glewInit();
    if (ogl->glew != GLEW_OK)
    {
        ogl->err = ekOGLGLEW;
        return;
    }

    if (_ogl3dimp_check_version(ogl->props.api) == FALSE)
    {
        ogl->err = ekOGLAPIVERS;
        return;
    }

    ogl->err = ekOGLOK;
}

/*---------------------------------------------------------------------------*/

OGLCtx *_ogl3dimp_context(const OGLProps *props, void *view, oglerr_t *err)
{
    GtkWidget *widget = NULL;
    OGLCtx *ogl = NULL;

    cassert_no_null(props);

    /* Non-screen context */
    if (view == NULL)
    {
        ptr_assign(err, ekOGLFULLSCN);
        return NULL;
    }

    /* View should be attached to top-level window */
    widget = GTK_WIDGET(view);
    if (GTK_IS_WINDOW(gtk_widget_get_toplevel(widget)) == FALSE)
    {
        ptr_assign(err, ekOGLVIEW);
        return NULL;
    }

    ogl = cast(bmem_malloc(sizeof(OGLCtx)), OGLCtx);
    ogl->widget = widget;
    ogl->props = *props;

    if (blib_strcmp(G_OBJECT_TYPE_NAME(widget), "GtkDrawingArea") == 0)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        gtk_widget_set_double_buffered(widget, FALSE);
#pragma GCC diagnostic pop

        /* The widget should to be realized for access to GdkWindow 'gtk_widget_get_window'
           necessary to create the EGL Surface */
        if (gtk_widget_get_realized(widget) == FALSE)
            gtk_widget_realize(widget);

        ogl->err = ekOGLCONTEXT;
        i_egl_config(widget, ogl);
    }
    else
    {
        cassert(blib_strcmp(G_OBJECT_TYPE_NAME(widget), "GtkGLArea"));
    }

    if (ogl->err == ekOGLOK)
    {
        ptr_assign(err, ekOGLOK);

        /* The widget is a Cairo-based NAppGUI view */
        if (blib_strcmp(gtk_widget_get_name(widget), "NAppGUICairoCtx") == 0)
            gtk_widget_set_name(widget, "NAppGUIOpenGLCtx");

        return ogl;
    }
    else
    {
        ptr_assign(err, ogl->err);
        _ogl3dimp_destroy(&ogl);
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

void _ogl3dimp_destroy(OGLCtx **ogl)
{
    cassert_no_null(ogl);
    cassert_no_null(*ogl);
    if ((*ogl)->context != NULL)
    {
        EGLBoolean ok;
        cassert(blib_strcmp(G_OBJECT_TYPE_NAME((*ogl)->widget), "GtkDrawingArea") == 0);

        if (eglGetCurrentContext() == (*ogl)->context)
            eglMakeCurrent((*ogl)->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        ok = eglDestroyContext((*ogl)->display, (*ogl)->context);
        cassert_unref(ok == EGL_TRUE, ok);
        (*ogl)->context = NULL;
    }

    if ((*ogl)->surface != NULL)
    {
        EGLBoolean ok = eglDestroySurface((*ogl)->display, (*ogl)->surface);
        cassert_unref(ok == EGL_TRUE, ok);
        (*ogl)->surface = NULL;
    }

    if ((*ogl)->display != NULL)
    {
        EGLBoolean ok = eglTerminate((*ogl)->display);
        cassert_unref(ok == EGL_TRUE, ok);
        (*ogl)->display = NULL;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_widget_set_double_buffered((*ogl)->widget, TRUE);
#pragma GCC diagnostic pop

    if (blib_strcmp(gtk_widget_get_name((*ogl)->widget), "NAppGUIOpenGLCtx") == 0)
        gtk_widget_set_name((*ogl)->widget, "NAppGUICairoCtx");

    bmem_free(*dcast(ogl, byte_t));
    *ogl = NULL;
}

/*---------------------------------------------------------------------------*/

void ogl3d_begin_draw(OGLCtx *ogl)
{
    cassert_no_null(ogl);
    if (eglGetCurrentContext() != ogl->context)
    {
        EGLBoolean ok = eglMakeCurrent(ogl->display, ogl->surface, ogl->surface, ogl->context);
        cassert_unref(ok == EGL_TRUE, ok);
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
    if (ogl->context != NULL)
        eglSwapBuffers(ogl->display, ogl->surface);
}

/*---------------------------------------------------------------------------*/

void ogl3d_set_size(OGLCtx *ogl, const uint32_t width, const uint32_t height)
{
    unref(ogl);
    unref(width);
    unref(height);
}
