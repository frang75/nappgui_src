/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: col2dgui.h
 *
 */

/* Col2D Hello GUI */

#include "col2dhello.hxx"

Window *col2dhello_window(App *app);

void col2dhello_dbind(void);

void col2dhello_new_shape(App *app, const V2Df pos);

void col2dhello_update_gui(App *app);

Box2Df col2dhello_cloud_box(const Cloud *cloud);

void col2dhello_update_cloud(Cloud *cloud);

void col2dhello_update_cloud_bounds(Cloud *cloud);

void col2dhello_update_seg(Seg *seg);

void col2dhello_update_box(Box *box);

void col2dhello_update_obb(OBB *obb);

void col2dhello_update_tri(Tri *tri);

void col2dhello_update_pol(Pol *pol);

void col2dhello_mouse_collisions(App *app, const real32_t mouse_x, const real32_t mouse_y);

void col2dhello_collisions(App *app);

void col2dhello_dbind_shape(App *app);
