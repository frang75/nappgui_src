/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ogl3d.c
 *
 */

/* OpenGL Context */

#include "ogl3d.h"
#include "ogl3d.inl"
#include <sewer/cassert.h>
#include <sewer/bstd.h>
#include <sewer/nowarn.hxx>
#include "glew.h"
#include <sewer/warn.hxx>

static uint32_t i_NUM_USERS = 0;
static uint32_t i_NUM_CONTEXT_ALLOC = 0;
static uint32_t i_NUM_CONTEXT_DEALLOC = 0;

/*---------------------------------------------------------------------------*/

void ogl3d_start(void)
{
    i_NUM_USERS += 1;
}

/*---------------------------------------------------------------------------*/

void ogl3d_finish(void)
{
    cassert(i_NUM_USERS > 0);

    if (i_NUM_USERS == 1)
    {
        if (i_NUM_CONTEXT_ALLOC != i_NUM_CONTEXT_DEALLOC)
        {
            char_t msg[128];
            bstd_sprintf(msg, sizeof(msg), "Non-dealloc OpenGL Context: %u/%u", i_NUM_CONTEXT_ALLOC, i_NUM_CONTEXT_DEALLOC);
            cassert_msg(FALSE, msg);
        }

        i_NUM_USERS = 0;
    }
    else
    {
        i_NUM_USERS -= 1;
    }
}

/*---------------------------------------------------------------------------*/

OGLCtx *ogl3d_context(const OGLProps *props, void *view, oglerr_t *err)
{
    OGLCtx *ctx = _ogl3dimp_context(props, view, err);
    if (ctx != NULL)
        i_NUM_CONTEXT_ALLOC += 1;
    return ctx;
}

/*---------------------------------------------------------------------------*/

void ogl3d_destroy(OGLCtx **ogl)
{
    _ogl3dimp_destroy(ogl);
    i_NUM_CONTEXT_DEALLOC += 1;
}

/*---------------------------------------------------------------------------*/

bool_t _ogl3dimp_check_version(const oglapi_t api)
{
    switch (api)
    {
    case ekOGL_1_1:
        return (bool_t)GLEW_VERSION_1_1;
    case ekOGL_1_2:
        return (bool_t)GLEW_VERSION_1_2;
    case ekOGL_1_2_1:
        return (bool_t)GLEW_VERSION_1_2_1;
    case ekOGL_1_3:
        return (bool_t)GLEW_VERSION_1_3;
    case ekOGL_1_4:
        return (bool_t)GLEW_VERSION_1_4;
    case ekOGL_1_5:
        return (bool_t)GLEW_VERSION_1_5;
    case ekOGL_2_0:
        return (bool_t)GLEW_VERSION_2_0;
    case ekOGL_2_1:
        return (bool_t)GLEW_VERSION_2_1;
    case ekOGL_3_0:
        return (bool_t)GLEW_VERSION_3_0;
    case ekOGL_3_1:
        return (bool_t)GLEW_VERSION_3_1;
    case ekOGL_3_2:
        return (bool_t)GLEW_VERSION_3_2;
    case ekOGL_3_3:
        return (bool_t)GLEW_VERSION_3_3;
    case ekOGL_4_0:
        return (bool_t)GLEW_VERSION_4_0;
    case ekOGL_4_1:
        return (bool_t)GLEW_VERSION_4_1;
    case ekOGL_4_2:
        return (bool_t)GLEW_VERSION_4_2;
    case ekOGL_4_3:
        return (bool_t)GLEW_VERSION_4_3;
    case ekOGL_4_4:
        return (bool_t)GLEW_VERSION_4_4;
    case ekOGL_4_5:
        return (bool_t)GLEW_VERSION_4_5;
    case ekOGL_4_6:
        return (bool_t)GLEW_VERSION_4_6;
        cassert_default();
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

const char_t *ogl3d_err_str(const oglerr_t err)
{
    switch (err)
    {
    case ekOGLAPIVERS:
        return "API Version not found";
    case ekOGLFULLSCN:
        return "Full screen not supported";
    case ekOGLVIEW:
        return "Conexion with view fails";
    case ekOGLPIXFORMAT:
        return "Pixels format fails";
    case ekOGLCONTEXT:
        return "OpenGL context fails";
    case ekOGLGLEW:
        return "GLEW fails";
    case ekOGLOK:
        return "OK";
        cassert_default();
    }

    return "";
}
