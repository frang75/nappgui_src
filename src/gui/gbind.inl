/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gbind.inl
 *
 */

/* Gui data binding */

#include "gui.ixx"

__EXTERN_C

void _gbind_update_control(Cell *cell, const DBind *stbind, const uint32_t member_id, const void *obj);

void _gbind_update_layout(Layout *layout, const DBind *stbind, const uint32_t member_id, const void *obj);

void _gbind_update_bool(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const bool_t value);

void _gbind_update_u32(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const uint32_t value);

void _gbind_update_norm32(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const real32_t value);

void _gbind_update_incr(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const bool_t positive_incr);

void _gbind_update_str(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const char_t *str);

void _gbind_update_image(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const Image *image);

bool_t _gbind_field_modify(const EvBind *evbind, const char_t *type, const uint16_t size, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize);

__END_C
