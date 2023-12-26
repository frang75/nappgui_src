/* Die Types */

#ifndef __DIE_HXX__
#define __DIE_HXX__

#include <gui/gui.hxx>

typedef struct _app_t App;

struct _app_t
{
    real32_t padding;
    real32_t corner;
    real32_t radius;
    uint32_t face;
    View *view;
    Window *window;
};

#endif