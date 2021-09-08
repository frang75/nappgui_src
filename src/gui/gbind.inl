/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gbind.inl
 *
 */

/* Gui data binding */

#include "gui.ixx"

__EXTERN_C

void gbind_set_component(Cell *cell, StBind *stbind, DBind *dbind, void *obj);

void gbind_set_layout(Layout *layout, StBind *stbind, DBind *dbind, void *obj);

void gbind_upd_bool(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, bool_t value);

void gbind_upd_enum(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, enum_t value);

void gbind_upd_uint32(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, uint32_t value);

void gbind_upd_norm_real32(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, const real32_t value);

void gbind_upd_string(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, const char_t *str);

void gbind_upd_image(Layout *layout, DBind *dbind, void *objbind, const Image *image);

void gbind_upd_increment(Layout *layout, DBind *dbind, void *objbind);

void gbind_upd_decrement(Layout *layout, DBind *dbind, void *objbind);

__END_C

