/* OpenGL 3 Demo */

#include <ogl3d/ogl3d.hxx>
#include "gui.hxx"

typedef struct _ogl3_t OGL3;

OGL3 *ogl3_create(View *view, oglerr_t *err);

void ogl3_destroy(OGL3 **ogl);

void ogl3_draw(OGL3 *ogl, const real32_t width, const real32_t height, const real32_t angle, const real32_t scale);

void ogl3_resize(OGL3 *ogl, const real32_t width, const real32_t height);
