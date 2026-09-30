/* OpenGL 4 Demo */

#include <ogl3d/ogl3d.hxx>
#include <gui/gui.hxx>

typedef struct _ogl4_t OGL4;

OGL4 *ogl4_create(View *view, oglerr_t *err);

void ogl4_destroy(OGL4 **ogl);

void ogl4_draw(OGL4 *ogl, const real32_t width, const real32_t height, const real32_t angle, const real32_t scale);

void ogl4_resize(OGL4 *ogl, const real32_t width, const real32_t height);
