/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: layout.h
 * https://nappgui.com/en/gui/layout.html
 *
 */

/* Layouts */

#include "gui.hxx"

__EXTERN_C

Layout *layout_create(const uint32_t ncols, const uint32_t nrows);

Cell *layout_cell(Layout *layout, const uint32_t col, const uint32_t row);

void layout_label(Layout *layout, Label *label, const uint32_t col, const uint32_t row);

void layout_button(Layout *layout, Button *button, const uint32_t col, const uint32_t row);

void layout_popup(Layout *layout, PopUp *popup, const uint32_t col, const uint32_t row);

void layout_edit(Layout *layout, Edit *edit, const uint32_t col, const uint32_t row);

void layout_combo(Layout *layout, Combo *combo, const uint32_t col, const uint32_t row);

void layout_listbox(Layout *layout, ListBox *list, const uint32_t col, const uint32_t row);

void layout_updown(Layout *layout, UpDown *updown, const uint32_t col, const uint32_t row);

void layout_slider(Layout *layout, Slider *slider, const uint32_t col, const uint32_t row);

void layout_progress(Layout *layout, Progress *progress, const uint32_t col, const uint32_t row);

void layout_view(Layout *layout, View *view, const uint32_t col, const uint32_t row);

void layout_textview(Layout *layout, TextView *view, const uint32_t col, const uint32_t row);

void layout_imageview(Layout *layout, ImageView *view, const uint32_t col, const uint32_t row);

void layout_tableview(Layout *layout, TableView *view, const uint32_t col, const uint32_t row);

void layout_splitview(Layout *layout, SplitView *view, const uint32_t col, const uint32_t row);

void layout_panel(Layout *layout, Panel *panel, const uint32_t col, const uint32_t row);

void layout_layout(Layout *layout, Layout *sublayout, const uint32_t col, const uint32_t row);

void *layout_control_imp(Layout *layout, const uint32_t col, const uint32_t row, const char_t *type);

void layout_taborder(Layout *layout, const orient_t order);

void layout_tabstop(Layout *layout, const uint32_t col, const uint32_t row, const bool_t tabstop);

void layout_hsize(Layout *layout, const uint32_t col, const real32_t width);

void layout_vsize(Layout *layout, const uint32_t row, const real32_t height);

void layout_hmargin(Layout *layout, const uint32_t col, const real32_t margin);

void layout_vmargin(Layout *layout, const uint32_t row, const real32_t margin);

void layout_hexpand(Layout *layout, const uint32_t col);

void layout_hexpand2(Layout *layout, const uint32_t col1, const uint32_t col2, const real32_t exp);

void layout_hexpand3(Layout* layout, const uint32_t col1, const uint32_t col2, const uint32_t col3, const real32_t exp1, const real32_t exp2);

void layout_vexpand(Layout *layout, const uint32_t row);

void layout_vexpand2(Layout* layout, const uint32_t row1, const uint32_t row2, const real32_t exp);

void layout_vexpand3(Layout* layout, const uint32_t row1, const uint32_t row2, const uint32_t row3, const real32_t exp1, const real32_t exp2);

void layout_halign(Layout *layout, const uint32_t col, const uint32_t row, const align_t align);

void layout_valign(Layout *layout, const uint32_t col, const uint32_t row, const align_t align);

void layout_show_col(Layout *layout, const uint32_t col, const bool_t visible);

void layout_show_row(Layout *layout, const uint32_t row, const bool_t visible);

void layout_margin(Layout *layout, const real32_t mall);

void layout_margin2(Layout *layout, const real32_t mtb, const real32_t mlr);

void layout_margin4(Layout *layout, const real32_t mt, const real32_t mr, const real32_t mb, const real32_t ml);

void layout_bgcolor(Layout *layout, const color_t color);

void layout_skcolor(Layout *layout, const color_t color);

void layout_update(const Layout *layout);

void layout_dbind_imp(Layout *layout, Listener *listener, const char_t *type, const uint16_t size);

void layout_dbind_obj_imp(Layout *layout, void *obj, const char_t *type);

void layout_dbind_update_imp(Layout *layout, const char_t *type, const uint16_t size, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize);

__END_C

#define layout_get_label(layout, col, row)\
    (Label*)layout_control_imp(layout, col, row, "Label")

#define layout_get_button(layout, col, row)\
    (Button*)layout_control_imp(layout, col, row, "Button")

#define layout_get_popup(layout, col, row)\
    (PopUp*)layout_control_imp(layout, col, row, "PopUp")

#define layout_get_edit(layout, col, row)\
    (Edit*)layout_control_imp(layout, col, row, "Edit")

#define layout_get_combo(layout, col, row)\
    (Combo*)layout_control_imp(layout, col, row, "Combo")

#define layout_get_listbox(layout, col, row)\
    (ListBox*)layout_control_imp(layout, col, row, "ListBox")

#define layout_get_updown(layout, col, row)\
    (UpDown*)layout_control_imp(layout, col, row, "UpDown")

#define layout_get_slider(layout, col, row)\
    (Slider*)layout_control_imp(layout, col, row, "Slider")

#define layout_get_progress(layout, col, row)\
    (Progress*)layout_control_imp(layout, col, row, "Progress")

#define layout_get_view(layout, col, row)\
    (View*)layout_control_imp(layout, col, row, "View")

#define layout_get_textview(layout, col, row)\
    (TextView*)layout_control_imp(layout, col, row, "TextView")

#define layout_get_imageview(layout, col, row)\
    (ImageView*)layout_control_imp(layout, col, row, "ImageView")

#define layout_get_panel(layout, col, row)\
    (Panel*)layout_control_imp(layout, col, row, "Panel")

#define layout_dbind(layout, listener, type)\
    layout_dbind_imp(layout, listener, (const char_t*)#type, (uint16_t)sizeof(type))

#define layout_dbind_obj(layout, obj, type)\
    (\
        (void)((type*)obj == obj),\
        layout_dbind_obj_imp(layout, (void*)obj, (const char_t*)#type)\
    )

#define layout_dbind_update(layout, type, mtype, mname)\
    (\
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype),\
        layout_dbind_update_imp(\
                layout,\
                (const char_t*)#type,\
                (uint16_t)sizeof(type),\
                (const char_t*)#mname,\
                (const char_t*)#mtype,\
                (uint16_t)STRUCT_MEMBER_OFFSET(type, mname),\
                (uint16_t)STRUCT_MEMBER_SIZE(type, mname))\
    )
