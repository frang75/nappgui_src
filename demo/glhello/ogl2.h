/* OpenGL 2 Demo */

#include <ogl3d/ogl3d.hxx>
#include "gui.hxx"

typedef struct _ogl2_t OGL2;

OGL2 *ogl2_create(View *view, oglerr_t *err);

void ogl2_destroy(OGL2 **ogl);

void ogl2_draw(OGL2 *ogl, const real32_t width, const real32_t height, const real32_t angle, const real32_t scale);

void ogl2_resize(OGL2 *ogl, const real32_t width, const real32_t height);


