/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: cell.h
 * https://nappgui.com/en/gui/cell.html
 *
 */

/* Layout-cells */

#include "gui.hxx"

__EXTERN_C

void *cell_control_imp(Cell *cell, const char_t *type);

Layout *cell_layout(Cell *cell);

void cell_enabled(Cell *cell, const bool_t enabled);

void cell_visible(Cell *cell, const bool_t visible);

void cell_focus(Cell *cell);

void cell_padding(Cell *cell, const real32_t pall);

void cell_padding2(Cell *cell, const real32_t ptb, const real32_t plr);

void cell_padding4(Cell *cell, const real32_t pt, const real32_t pr, const real32_t pb, const real32_t pl);

void cell_dbind_imp(
            Cell *cell,
            const char_t *type, 
            const uint16_t size,
            const char_t *mname, 
            const char_t *mtype, 
            const uint16_t moffset, 
            const uint16_t msize);

__END_C

#define cell_label(cell)\
    (Label*)cell_control_imp(cell, "Label")

#define cell_button(cell)\
    (Button*)cell_control_imp(cell, "Button")

#define cell_popup(cell)\
    (PopUp*)cell_control_imp(cell, "PopUp")

#define cell_edit(cell)\
    (Edit*)cell_control_imp(cell, "Edit")

#define cell_combo(cell)\
    (Combo*)cell_control_imp(cell, "Combo")

#define cell_updown(cell)\
    (UpDown*)cell_control_imp(cell, "UpDown")

#define cell_slider(cell)\
    (Slider*)cell_control_imp(cell, "Slider")

#define cell_progress(cell)\
    (Progress*)cell_control_imp(cell, "Progress")

#define cell_view(cell)\
    (View*)cell_control_imp(cell, "View")

#define cell_textview(cell)\
    (TextView*)cell_control_imp(cell, "TextView")

#define cell_imageview(cell)\
    (ImageView*)cell_control_imp(cell, "ImageView")

#define cell_dbind(cell, type, mtype, mname)\
    (\
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype),\
        cell_dbind_imp(\
                cell,\
                (const char_t*)#type,\
                (uint16_t)sizeof(type),\
                (const char_t*)#mname,\
                (const char_t*)#mtype,\
                (uint16_t)STRUCT_MEMBER_OFFSET(type, mname),\
                (uint16_t)STRUCT_MEMBER_SIZE(type, mname))\
    )
