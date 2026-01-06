/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: vctrl.inl
 *
 */

/* Custom control optimized API. For Label, ImageView, ListBox, TableView and futures */

#include "gui.ixx"

__EXTERN_C

View *_vctrl_create_imp(const uint32_t flags, const VCtrlTbl *tbl, void *data, const char_t *type);

Cell *_vctrl_cell(View *view);

void _vctrl_add_transition(View *view, Listener *listener);

void _vctrl_delete_transition(View *view);

__END_C

#define _vctrl_create(flags, tbl, data, type) \
    ( \
        (void)(cast(data, type) == data), \
        _vctrl_create_imp(flags, tbl, cast(data, void), #type))
