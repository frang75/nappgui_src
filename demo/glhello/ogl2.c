/* OpenGL 2 Demo */

#include "ogl2.h"
#include "glhello.h"
#include <ogl3d/ogl3d.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <core/heap.h>
#include <gui/view.h>

#include <sewer/nowarn.hxx>
#include <ogl3d/glew.h>
#include <sewer/warn.hxx>

struct _ogl2_t
{
    OGLCtx *ctx;
    GLboolean init;
    GLuint texture;
    GLuint vbo;
    GLuint ibo;
    GLuint pshader;
    GLint pos_vertex;
    GLint col_vertex;
    GLint tex_vertex;
    GLint mvp_uniform;
    GLint tex_uniform;
};

/*---------------------------------------------------------------------------*/

static const GLchar *i_VS = "#version 120\n"
                            "uniform mat4 uMVP;\n"
                            "attribute vec3 vPos;\n"
                            "attribute vec3 vColor;\n"
                            "attribute vec2 vTex;\n"
                            "varying vec3 outColor;\n"
                            "varying vec2 texCoord;\n"
                            "void main(void) {\n"
                            "   gl_Position = uMVP * vec4(vPos.xyz, 1);\n"
                            "   outColor = vColor;\n"
                            "   texCoord = vTex;\n"
                            "}\n ";

static const GLchar *i_FS = "#version 120\n"
                            "varying vec3 outColor;\n"
                            "varying vec2 texCoord;\n"
                            "uniform sampler2D uTexture;\n"
                            "void main (void) {\n"
                            "    gl_FragColor = vec4(outColor.xyz, 1.0);\n"
                            "    gl_FragColor *= texture2D(uTexture, texCoord);\n"
                            "}\n";

/*---------------------------------------------------------------------------*/

OGL2 *ogl2_create(View *view, oglerr_t *err)
{
    void *nview = view_native(view);
    OGLCtx *ctx = NULL;
    OGLProps props;
    props.api = ekOGL_2_1;
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
        OGL2 *ogl = heap_new0(OGL2);
        ogl->ctx = ctx;
        ogl->init = GL_FALSE;
        return ogl;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void ogl2_destroy(OGL2 **ogl)
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
    heap_delete(ogl, OGL2);
}

/*---------------------------------------------------------------------------*/

/* Data to be stored in GPU memory */
static void i_device_data(OGL2 *ogl)
{
    float vertices[] = {
        0, 1, 0, 1, 0, 0, .5f, 0, /* v0 pos, color, tex */
        -1, -1, 0, 0, 1, 0, 0, 1, /* v1 pos, color, tex */
        1, -1, 0, 0, 0, 1, 1, 1}; /* v2 pos, color, tex */
    uint32_t indices[] = {0, 1, 2};
    const byte_t *texdata;
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)texwidth, (GLsizei)texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, (const void *)texdata);
    cassert(glGetError() == GL_NO_ERROR);

    /* Vertex Buffer Object */
    glGenBuffers(1, &ogl->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* Index Buffer */
    glGenBuffers(1, &ogl->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

    /* The association between an attribute variable name and a generic attribute
     index can be specified at any time by calling glBindAttribLocation.
     Attribute bindings do not go into effect until glLinkProgram is called.
     After a program object has been linked successfully, the index values for
     attribute variables remain fixed until the next link command occurs.
     The attribute values can only be queried after a link if the link was
     successful. glGetAttribLocation returns the binding that actually went
     into effect the last time glLinkProgram was called for the specified
     program object. Attribute bindings that have been specified since the last
     link operation are not returned by glGetAttribLocation. */
    glBindAttribLocation(ogl->pshader, 0, "vPos");
    cassert((err = glGetError()) == GL_NO_ERROR);
    glBindAttribLocation(ogl->pshader, 1, "vColor");
    cassert((err = glGetError()) == GL_NO_ERROR);
    glBindAttribLocation(ogl->pshader, 2, "vTex");
    cassert((err = glGetError()) == GL_NO_ERROR);
    glLinkProgram(ogl->pshader);
    glGetProgramiv(ogl->pshader, GL_LINK_STATUS, &status);
    glGetProgramInfoLog(ogl->pshader, 512, NULL, info);
    cassert(status != 0);

    /* Shader Parameters */
    ogl->pos_vertex = glGetAttribLocation(ogl->pshader, "vPos");
    cassert((err = glGetError()) == GL_NO_ERROR);
    cassert(ogl->pos_vertex == 0);

    ogl->col_vertex = glGetAttribLocation(ogl->pshader, "vColor");
    cassert((err = glGetError()) == GL_NO_ERROR);
    cassert(ogl->col_vertex == 1);

    ogl->tex_vertex = glGetAttribLocation(ogl->pshader, "vTex");
    cassert((err = glGetError()) == GL_NO_ERROR);
    cassert(ogl->tex_vertex == 2);

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

void ogl2_draw(OGL2 *ogl, const real32_t width, const real32_t height, const real32_t angle, const real32_t scale)
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
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ogl->texture);
    cassert_unref((err = glGetError()) == GL_NO_ERROR, err);

    /* Set vertex buffer */
    glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Set index buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl->ibo);
    cassert((err = glGetError()) == GL_NO_ERROR);

    /* Enable vertex attributes */
    glVertexAttribPointer((GLuint)ogl->pos_vertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    cassert((err = glGetError()) == GL_NO_ERROR);
    glEnableVertexAttribArray((GLuint)ogl->pos_vertex);
    cassert((err = glGetError()) == GL_NO_ERROR);
    glVertexAttribPointer((GLuint)ogl->col_vertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    cassert((err = glGetError()) == GL_NO_ERROR);
    glEnableVertexAttribArray((GLuint)ogl->col_vertex);
    cassert((err = glGetError()) == GL_NO_ERROR);
    glVertexAttribPointer((GLuint)ogl->tex_vertex, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    cassert((err = glGetError()) == GL_NO_ERROR);
    glEnableVertexAttribArray((GLuint)ogl->tex_vertex);
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

void ogl2_resize(OGL2 *ogl, const real32_t width, const real32_t height)
{
    cassert_no_null(ogl);
    ogl3d_set_size(ogl->ctx, (uint32_t)width, (uint32_t)height);
}
