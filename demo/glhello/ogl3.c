/* OpenGL 3 Demo */

#include "ogl3.h"
#include "glhello.h"
#include <ogl3d/ogl3d.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <core/heap.h>
#include <gui/view.h>

#include <sewer/nowarn.hxx>
#include <ogl3d/glew.h>
#include <sewer/warn.hxx>

struct _ogl3_t
{
    OGLCtx *ctx;
    GLboolean init;
    GLuint texture;
    GLuint vbo;
    GLuint vao;
    GLuint ibo;
    GLuint pshader;
    GLint mvp_uniform;
    GLint tex_uniform;
};

/*---------------------------------------------------------------------------*/

static const GLchar *i_VS = "#version 330\n"
                            "uniform mat4 uMVP;\n"
                            "layout(location = 0) in vec3 vPos;\n"
                            "layout(location = 1) in vec3 vColor;\n"
                            "layout(location = 2) in vec2 vTex;\n"
                            "out vec3 outColor;\n"
                            "out vec2 texCoord;\n"
                            "void main(void) {\n"
                            "   gl_Position = uMVP * vec4(vPos.xyz, 1);\n"
                            "   outColor = vColor;\n"
                            "   texCoord = vTex;\n"
                            "}\n";

static const char_t *i_FS = "#version 330\n"
                            "in vec3 outColor;\n"
                            "in vec2 texCoord;\n"
                            "out vec4 fragColor;\n"
                            "uniform sampler2D uTexture;\n"
                            "void main (void) {\n"
                            "   fragColor = vec4(outColor.xyz, 1.0);\n"
                            "   fragColor *= texture(uTexture, texCoord);\n"
                            "}\n";

/*---------------------------------------------------------------------------*/

OGL3 *ogl3_create(View *view, oglerr_t *err)
{
    void *nview = view_native(view);
    OGLCtx *ctx = NULL;
    OGLProps props;
    props.api = ekOGL_3_3;
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
        OGL3 *ogl = heap_new0(OGL3);
        ogl->ctx = ctx;
        ogl->init = GL_FALSE;
        return ogl;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void ogl3_destroy(OGL3 **ogl)
{
    cassert_no_null(ogl);
    cassert_no_null(*ogl);
    ogl3d_begin_draw((*ogl)->ctx);

    /* Delete all objects in device (GPU) space */
    if ((*ogl)->init == GL_TRUE)
    {
        GLenum err;

        /* Unset all device objects */
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        /* Delete the texture */
        if ((*ogl)->texture != 0)
        {
            glDeleteTextures(1, &(*ogl)->texture);
            cassert((err = glGetError()) == GL_NO_ERROR);
            (*ogl)->texture = 0;
        }

        /* Delete the shader */
        if ((*ogl)->pshader != 0)
        {
            glDeleteProgram((*ogl)->pshader);
            cassert((err = glGetError()) == GL_NO_ERROR);
            (*ogl)->pshader = 0;
        }

        /* Delete the Vertex Buffer Object */
        if ((*ogl)->vbo != 0)
        {
            glDeleteBuffers(1, &(*ogl)->vbo);
            cassert((err = glGetError()) == GL_NO_ERROR);
            (*ogl)->vbo = 0;
        }

        /* Delete the Vertex Array Object */
        if ((*ogl)->vao != 0)
        {
            glDeleteVertexArrays(1, &(*ogl)->vao);
            cassert((err = glGetError()) == GL_NO_ERROR);
            (*ogl)->vao = 0;
        }

        /* Delete the Index(Element) Buffer Object */
        if ((*ogl)->ibo != 0)
        {
            glDeleteBuffers(1, &(*ogl)->ibo);
            cassert((err = glGetError()) == GL_NO_ERROR);
            (*ogl)->ibo = 0;
        }

        unref(err);
    }

    ogl3d_end_draw((*ogl)->ctx);
    ogl3d_destroy(&(*ogl)->ctx);
    heap_delete(ogl, OGL3);
}

/*---------------------------------------------------------------------------*/

/* Data to be stored in GPU memory */
static void i_device_data(OGL3 *ogl)
{
    const float vertices[] = {
        0, 1, 0, 1, 0, 0, .5f, 0, /* v0 pos, color, tex */
        -1, -1, 0, 0, 1, 0, 0, 1, /* v1 pos, color, tex */
        1, -1, 0, 0, 0, 1, 1, 1}; /* v2 pos, color, tex */
    uint32_t indices[] = {0, 1, 2};
    const byte_t *texdata = NULL;
    uint32_t texwidth, texheight;
    pixformat_t texformat;
    GLuint vshader;
    GLuint fshader;
    GLint status;
    GLenum err;
    char info[512];
    const char *version = cast_const(glGetString(GL_VERSION), char);
    const char *renderer = cast_const(glGetString(GL_RENDERER), char);
    unref(version);
    unref(renderer);

    /* Texture */
    glhello_texdata(&texdata, &texwidth, &texheight, &texformat);
    cassert(texformat == ekRGB24);
    glGenTextures(1, &ogl->texture);
    cassert(glGetError() == GL_NO_ERROR);
    glBindTexture(GL_TEXTURE_2D, ogl->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)texwidth, (GLsizei)texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, cast_const(texdata, void));
    cassert(glGetError() == GL_NO_ERROR);

    /* Vertex Array Object */
    glGenVertexArrays(1, &ogl->vao);
    glBindVertexArray(ogl->vao);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Vertex Buffer Object */
    glGenBuffers(1, &ogl->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Enable vertex attributes */
    /* 0 = layout(location = 0) vPos */
    glVertexAttribPointer(0 /* vPos */, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    cassert((err = glGetError()) == GL_NO_ERROR);
    glEnableVertexAttribArray(0 /* vPos */);
    cassert((err = glGetError()) == GL_NO_ERROR);
    /* 1 = layout(location = 1) vColor */
    glVertexAttribPointer(1 /* vColor */, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    cassert((err = glGetError()) == GL_NO_ERROR);
    glEnableVertexAttribArray(1 /* vColor */);
    cassert((err = glGetError()) == GL_NO_ERROR);
    /* 2 = layout(location = 2) vTex */
    glVertexAttribPointer(2 /* vTex */, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    cassert((err = glGetError()) == GL_NO_ERROR);
    glEnableVertexAttribArray(2 /* vTex */);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Index Buffer */
    glGenBuffers(1, &ogl->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Vertex Shader */
    vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &i_VS, NULL);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(vshader, 512, NULL, info);
    cassert(status != 0);

    /* Pixel Shader */
    fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &i_FS, NULL);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(fshader, 512, NULL, info);
    cassert(status != 0);

    /* Shader Program */
    ogl->pshader = glCreateProgram();
    glAttachShader(ogl->pshader, vshader);
    glAttachShader(ogl->pshader, fshader);
    glLinkProgram(ogl->pshader);
    glGetProgramiv(ogl->pshader, GL_LINK_STATUS, &status);
    glGetProgramInfoLog(ogl->pshader, 512, NULL, info);
    cassert(status != 0);

    /* Shader uniform access */
    ogl->mvp_uniform = glGetUniformLocation(ogl->pshader, "uMVP");
    cassert((err = glGetError()) == GL_NO_ERROR);

    ogl->tex_uniform = glGetUniformLocation(ogl->pshader, "uTexture");
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Delete vertex and pixel shared already linked */
    glDeleteShader(vshader);
    cassert((err = glGetError()) == GL_NO_ERROR);

    glDeleteShader(fshader);
    cassert((err = glGetError()) == GL_NO_ERROR);

    unref(err);
}

/*---------------------------------------------------------------------------*/

static void i_scale_rotate_Z(GLfloat *m, const real32_t a, const real32_t s)
{
    real32_t ca = bmath_cosf(a);
    real32_t sa = bmath_sinf(a);
    m[0] = s * ca;
    m[1] = s * sa;
    m[2] = 0;
    m[3] = 0;
    m[4] = -s * sa;
    m[5] = s * ca;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = 1;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

/*---------------------------------------------------------------------------*/

void ogl3_draw(OGL3 *ogl, const real32_t width, const real32_t height, const real32_t angle, const real32_t scale)
{
    GLfloat m[16];
    GLenum err;

    cassert_no_null(ogl);
    ogl3d_begin_draw(ogl->ctx);

    if (ogl->init == GL_FALSE)
    {
        i_device_data(ogl);
        ogl->init = GL_TRUE;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glClearColor(.8f, .8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Set the texture */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ogl->texture);
    cassert_unref((err = glGetError()) == GL_NO_ERROR, err);

    /* Set vertex array object */
    glBindVertexArray(ogl->vao);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Set index buffer object */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl->ibo);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Set the shader */
    glUseProgram(ogl->pshader);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Model-View-Projection and draw */
    i_scale_rotate_Z(m, angle * 2 * kBMATH_PIf, scale);
    glUniformMatrix4fv(ogl->mvp_uniform, 1, GL_FALSE, m);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    cassert((err = glGetError()) == GL_NO_ERROR);

    ogl3d_end_draw(ogl->ctx);
}

/*---------------------------------------------------------------------------*/

void ogl3_resize(OGL3 *ogl, const real32_t width, const real32_t height)
{
    cassert_no_null(ogl);
    ogl3d_set_size(ogl->ctx, (uint32_t)width, (uint32_t)height);
}
