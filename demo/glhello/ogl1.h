/* OpenGL 1 Demo */

#include <ogl3d/ogl3d.hxx>
#include <gui/gui.hxx>

typedef struct _ogl1_t OGL1;

OGL1 *ogl1_create(View *view, oglerr_t *err);

void ogl1_destroy(OGL1 **ogl);

void ogl1_draw(OGL1 *ogl, const real32_t width, const real32_t height, const real32_t angle, const real32_t scale);

void ogl1_resize(OGL1 *ogl, const real32_t width, const real32_t height);
