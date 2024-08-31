/* OpenGL 1 Demo */

#include "ogl1.h"
#include "glhello.h"
#include <ogl3d/ogl3d.h>
#include <sewer/cassert.h>
#include <core/heap.h>
#include <gui/view.h>

#include "nowarn.hxx"
#include <ogl3d/glew.h>
#include "warn.hxx"

struct _ogl1_t
{
    OGLCtx *ctx;
    GLboolean init;
    GLuint texture;
};

/*---------------------------------------------------------------------------*/

OGL1 *ogl1_create(View *view, oglerr_t *err)
{
    void *nview = view_native(view);
    OGLCtx *ctx = NULL;
    OGLProps props;
    props.api = ekOGL_1_1;
    props.hdaccel = TRUE;
    props.color_bpp = 32;
    props.depth_bpp = 0;
    props.stencil_bpp = 0;
    props.aux_buffers = 0;
    props.transparent = FALSE;
    props.shared = NULL;
    ctx = ogl3d_context(&props, nview, err);

    if (ctx != NULL)
    {
        OGL1 *ogl = heap_new0(OGL1);
        ogl->ctx = ctx;
        ogl->init = GL_FALSE;
        return ogl;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void ogl1_destroy(OGL1 **ogl)
{
    cassert_no_null(ogl);
    cassert_no_null(*ogl);
    ogl3d_begin_draw((*ogl)->ctx);

    if ((*ogl)->init == GL_TRUE)
    {
        GLenum err;
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &(*ogl)->texture);
        cassert((err = glGetError()) == GL_NO_ERROR);
        unref(err);
    }

    ogl3d_end_draw((*ogl)->ctx);
    ogl3d_destroy(&(*ogl)->ctx);
    heap_delete(ogl, OGL1);
}

/*---------------------------------------------------------------------------*/

/* Data to be stored in GPU memory */
static void i_device_data(OGL1 *ogl)
{
    const byte_t *texdata = NULL;
    uint32_t texwidth, texheight;
    pixformat_t texformat;
    glhello_texdata(&texdata, &texwidth, &texheight, &texformat);
    cassert(texformat == ekRGB24);
    glGenTextures(1, &ogl->texture);
    cassert(glGetError() == GL_NO_ERROR);
    glBindTexture(GL_TEXTURE_2D, ogl->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)texwidth, (GLsizei)texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, (const void *)texdata);
    cassert(glGetError() == GL_NO_ERROR);
}

/*---------------------------------------------------------------------------*/

void ogl1_draw(OGL1 *ogl, const real32_t width, const real32_t height, const real32_t angle, const real32_t scale)
{
    GLenum err;

    cassert_no_null(ogl);
    ogl3d_begin_draw(ogl->ctx);

    if (ogl->init == GL_FALSE)
    {
        i_device_data(ogl);
        ogl->init = GL_TRUE;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glClearColor(.8f, .8f, .8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(scale, scale, 0);
    glRotatef(angle * 360, 0, 0, 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ogl->texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    /* Not in GL 1.1 */
    /* glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS); */
    /* glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE); */
    cassert((err = glGetError()) == GL_NO_ERROR);

    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0);
    glTexCoord2f(.5, 0);
    glVertex3f(0, 1, 0);
    glColor3f(0, 1, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-1, -1, 0);
    glColor3f(0, 0, 1);
    glTexCoord2f(1, 1);
    glVertex3f(1, -1, 0);
    glEnd();
    unref(err);

    ogl3d_end_draw(ogl->ctx);
}

/*---------------------------------------------------------------------------*/

void ogl1_resize(OGL1 *ogl, const real32_t width, const real32_t height)
{
    cassert_no_null(ogl);
    ogl3d_set_size(ogl->ctx, (uint32_t)width, (uint32_t)height);
}
