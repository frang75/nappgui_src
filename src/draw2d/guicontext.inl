/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: guicontext.inl
 *
 */

/* Gui rendering context */

#include "draw2d.ixx"

__EXTERN_C

GuiContext *gui_context_create(void);

void _gui_context_append_label_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_label_create,
                        FPtr_destroy func_label_destroy,
                        FPtr_set_listener func_label_OnClick,
                        FPtr_set_listener func_label_OnMouseEnter,
                        FPtr_set_listener func_label_OnMouseExit,
                        FPtr_set_text func_label_set_text,
                        FPtr_set_const_ptr func_label_set_font,
                        FPtr_set_enum func_label_set_align,
                        FPtr_set_enum func_label_set_ellipsis,
                        FPtr_set_uint32 func_label_set_text_color,
                        FPtr_set_uint32 func_label_set_bg_color,
                        FPtr_bounds1 func_label_bounds,
                        FPtr_set_ptr func_attach_label_to_panel,
                        FPtr_set_ptr func_detach_label_from_panel,
                        FPtr_set_bool func_label_set_visible,
                        FPtr_set_bool func_label_set_enabled,
                        FPtr_get2_real32 func_label_get_size,
                        FPtr_get2_real32 func_label_get_origin,
                        FPtr_set4_real32 func_label_set_frame);
#define gui_context_append_label_manager(\
                        context,\
                        func_label_create,\
                        func_label_destroy,\
                        func_label_OnClick,\
                        func_label_OnMouseEnter,\
                        func_label_OnMouseExit,\
                        func_label_set_text,\
                        func_label_set_font,\
                        func_label_set_align,\
                        func_label_set_ellipsis,\
                        func_label_set_text_color,\
                        func_label_set_bg_color,\
                        func_label_bounds,\
                        func_attach_label_to_panel,\
                        func_detach_label_from_panel,\
                        func_label_set_visible,\
                        func_label_set_enabled,\
                        func_label_get_size,\
                        func_label_get_origin,\
                        func_label_set_frame,\
                        label_type, panel_type, font_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_label_create, label_type, label_flag_t),\
        FUNC_CHECK_DESTROY(func_label_destroy, label_type),\
        FUNC_CHECK_SET_LISTENER(func_label_OnClick, label_type),\
        FUNC_CHECK_SET_LISTENER(func_label_OnMouseEnter, label_type),\
        FUNC_CHECK_SET_LISTENER(func_label_OnMouseExit, label_type),\
        FUNC_CHECK_SET_TEXT(func_label_set_text, label_type),\
        FUNC_CHECK_SET_CONST_PTR(func_label_set_font, label_type, font_type),\
        FUNC_CHECK_SET_ENUM(func_label_set_align, label_type, align_t),\
        FUNC_CHECK_SET_ENUM(func_label_set_ellipsis, label_type, ellipsis_t),\
        FUNC_CHECK_SET_UINT32(func_label_set_text_color, label_type),\
        FUNC_CHECK_SET_UINT32(func_label_set_bg_color, label_type),\
        FUNC_CHECK_BOUNDS1(func_label_bounds, label_type),\
        FUNC_CHECK_SET_PTR(func_attach_label_to_panel, label_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_label_from_panel, label_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_label_set_visible, label_type),\
        FUNC_CHECK_SET_BOOL(func_label_set_enabled, label_type),\
        FUNC_CHECK_GET2_REAL32(func_label_get_size, label_type),\
        FUNC_CHECK_GET2_REAL32(func_label_get_origin, label_type),\
        FUNC_CHECK_SET4_REAL32(func_label_set_frame, label_type),\
        _gui_context_append_label_manager(\
                        context,\
                        (FPtr_create_enum)func_label_create,\
                        (FPtr_destroy)func_label_destroy,\
                        (FPtr_set_listener)func_label_OnClick,\
                        (FPtr_set_listener)func_label_OnMouseEnter,\
                        (FPtr_set_listener)func_label_OnMouseExit,\
                        (FPtr_set_text)func_label_set_text,\
                        (FPtr_set_const_ptr)func_label_set_font,\
                        (FPtr_set_enum)func_label_set_align,\
                        (FPtr_set_enum)func_label_set_ellipsis,\
                        (FPtr_set_uint32)func_label_set_text_color,\
                        (FPtr_set_uint32)func_label_set_bg_color,\
                        (FPtr_bounds1)func_label_bounds,\
                        (FPtr_set_ptr)func_attach_label_to_panel,\
                        (FPtr_set_ptr)func_detach_label_from_panel,\
                        (FPtr_set_bool)func_label_set_visible,\
                        (FPtr_set_bool)func_label_set_enabled,\
                        (FPtr_get2_real32)func_label_get_size,\
                        (FPtr_get2_real32)func_label_get_origin,\
                        (FPtr_set4_real32)func_label_set_frame)\
    )

void _gui_context_append_button_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_button_create,
                        FPtr_destroy func_button_destroy,
                        FPtr_set_listener func_button_OnClick,
                        FPtr_set_text func_button_set_text,
                        FPtr_set_text func_button_set_tooltip,
                        FPtr_set_const_ptr func_button_set_font,
                        FPtr_set_enum func_button_set_align,
                        FPtr_set_const_ptr func_button_set_image,
                        FPtr_set_enum func_button_set_state,
                        FPtr_get_enum func_button_get_state,
                        FPtr_bounds2 func_button_bounds,
                        FPtr_set_ptr func_attach_button_to_panel,
                        FPtr_set_ptr func_detach_button_from_panel,
                        FPtr_set_bool func_button_set_visible,
                        FPtr_set_bool func_button_set_enabled,
                        FPtr_get2_real32 func_button_get_size,
                        FPtr_get2_real32 func_button_get_origin,
                        FPtr_set4_real32 func_button_set_frame);
#define gui_context_append_button_manager(\
                        context,\
                        func_button_create,\
                        func_button_destroy,\
                        func_button_OnClick,\
                        func_button_set_text,\
                        func_button_set_tooltip,\
                        func_button_set_font,\
                        func_button_set_align,\
                        func_button_set_image,\
                        func_button_set_state,\
                        func_button_get_state,\
                        func_button_bounds,\
                        func_attach_button_to_panel,\
                        func_detach_button_from_panel,\
                        func_button_set_visible,\
                        func_button_set_enabled,\
                        func_button_get_size,\
                        func_button_get_origin,\
                        func_button_set_frame,\
                        button_type, panel_type, image_type, font_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_button_create, button_type, button_flag_t),\
        FUNC_CHECK_DESTROY(func_button_destroy, button_type),\
        FUNC_CHECK_SET_LISTENER(func_button_OnClick, button_type),\
        FUNC_CHECK_SET_TEXT(func_button_set_text, button_type),\
        FUNC_CHECK_SET_TEXT(func_button_set_tooltip, button_type),\
        FUNC_CHECK_SET_CONST_PTR(func_button_set_font, button_type, font_type),\
        FUNC_CHECK_SET_ENUM(func_button_set_align, button_type, align_t),\
        FUNC_CHECK_SET_CONST_PTR(func_button_set_image, button_type, image_type),\
        FUNC_CHECK_SET_ENUM(func_button_set_state, button_type, state_t),\
        FUNC_CHECK_GET_ENUM(func_button_get_state, button_type, state_t),\
        FUNC_CHECK_BOUNDS2(func_button_bounds, button_type),\
        FUNC_CHECK_SET_PTR(func_attach_button_to_panel, button_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_button_from_panel, button_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_button_set_visible, button_type),\
        FUNC_CHECK_SET_BOOL(func_button_set_enabled, button_type),\
        FUNC_CHECK_GET2_REAL32(func_button_get_size, button_type),\
        FUNC_CHECK_GET2_REAL32(func_button_get_origin, button_type),\
        FUNC_CHECK_SET4_REAL32(func_button_set_frame, button_type),\
        _gui_context_append_button_manager(\
                        context,\
                        (FPtr_create_enum)func_button_create,\
                        (FPtr_destroy)func_button_destroy,\
                        (FPtr_set_listener)func_button_OnClick,\
                        (FPtr_set_text)func_button_set_text,\
                        (FPtr_set_text)func_button_set_tooltip,\
                        (FPtr_set_const_ptr)func_button_set_font,\
                        (FPtr_set_enum)func_button_set_align,\
                        (FPtr_set_const_ptr)func_button_set_image,\
                        (FPtr_set_enum)func_button_set_state,\
                        (FPtr_get_enum)func_button_get_state,\
                        (FPtr_bounds2)func_button_bounds,\
                        (FPtr_set_ptr)func_attach_button_to_panel,\
                        (FPtr_set_ptr)func_detach_button_from_panel,\
                        (FPtr_set_bool)func_button_set_visible,\
                        (FPtr_set_bool)func_button_set_enabled,\
                        (FPtr_get2_real32)func_button_get_size,\
                        (FPtr_get2_real32)func_button_get_origin,\
                        (FPtr_set4_real32)func_button_set_frame)\
    )

void _gui_context_append_popup_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_popup_create,
                        FPtr_destroy func_popup_destroy,
                        FPtr_set_listener func_popup_OnChange,
                        FPtr_set_elem func_popup_set_elem,
                        FPtr_set_text func_popup_set_tooltip,
                        FPtr_set_const_ptr func_popup_set_font,
                        FPtr_set_uint32 func_popup_list_height,
                        FPtr_set_uint32 func_popup_set_selected,
                        FPtr_get_uint32 func_popup_get_selected,
                        FPtr_bounds3 func_popup_bounds,
                        FPtr_set_ptr func_attach_popup_to_panel,
                        FPtr_set_ptr func_detach_popup_from_panel,
                        FPtr_set_bool func_popup_set_visible,
                        FPtr_set_bool func_popup_set_enabled,
                        FPtr_get2_real32 func_popup_get_size,
                        FPtr_get2_real32 func_popup_get_origin,
                        FPtr_set4_real32 func_popup_set_frame);
#define gui_context_append_popup_manager(\
                        context,\
                        func_popup_create,\
                        func_popup_destroy,\
                        func_popup_OnChange,\
                        func_popup_set_elem,\
                        func_popup_set_tooltip,\
                        func_popup_set_font,\
                        func_popup_list_height,\
                        func_popup_set_selected,\
                        func_popup_get_selected,\
                        func_popup_bounds,\
                        func_attach_popup_to_panel,\
                        func_detach_popup_from_panel,\
                        func_popup_set_visible,\
                        func_popup_set_enabled,\
                        func_popup_get_size,\
                        func_popup_get_origin,\
                        func_popup_set_frame,\
                        popup_type, panel_type, image_type, font_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_popup_create, popup_type, popup_flag_t),\
        FUNC_CHECK_DESTROY(func_popup_destroy, popup_type),\
        FUNC_CHECK_SET_LISTENER(func_popup_OnChange, popup_type),\
        FUNC_CHECK_SET_ELEM(func_popup_set_elem, popup_type),\
        FUNC_CHECK_SET_TEXT(func_popup_set_tooltip, popup_type),\
        FUNC_CHECK_SET_CONST_PTR(func_popup_set_font, popup_type, font_type),\
        FUNC_CHECK_SET_UINT32(func_popup_list_height, popup_type),\
        FUNC_CHECK_SET_UINT32(func_popup_set_selected, popup_type),\
        FUNC_CHECK_GET_UINT32(func_popup_get_selected, popup_type),\
        FUNC_CHECK_BOUNDS3(func_popup_bounds, popup_type),\
        FUNC_CHECK_SET_PTR(func_attach_popup_to_panel, popup_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_popup_from_panel, popup_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_popup_set_visible, popup_type),\
        FUNC_CHECK_SET_BOOL(func_popup_set_enabled, popup_type),\
        FUNC_CHECK_GET2_REAL32(func_popup_get_size, popup_type),\
        FUNC_CHECK_GET2_REAL32(func_popup_get_origin, popup_type),\
        FUNC_CHECK_SET4_REAL32(func_popup_set_frame, popup_type),\
        _gui_context_append_popup_manager(\
                        context,\
                        (FPtr_create_enum)func_popup_create,\
                        (FPtr_destroy)func_popup_destroy,\
                        (FPtr_set_listener)func_popup_OnChange,\
                        (FPtr_set_elem)func_popup_set_elem,\
                        (FPtr_set_text)func_popup_set_tooltip,\
                        (FPtr_set_const_ptr)func_popup_set_font,\
                        (FPtr_set_uint32)func_popup_list_height,\
                        (FPtr_set_uint32)func_popup_set_selected,\
                        (FPtr_get_uint32)func_popup_get_selected,\
                        (FPtr_bounds3)func_popup_bounds,\
                        (FPtr_set_ptr)func_attach_popup_to_panel,\
                        (FPtr_set_ptr)func_detach_popup_from_panel,\
                        (FPtr_set_bool)func_popup_set_visible,\
                        (FPtr_set_bool)func_popup_set_enabled,\
                        (FPtr_get2_real32)func_popup_get_size,\
                        (FPtr_get2_real32)func_popup_get_origin,\
                        (FPtr_set4_real32)func_popup_set_frame)\
    )

void _gui_context_append_edit_manager(
                        GuiContext *context,
                        FPtr_create_enum func_edit_create,
                        FPtr_destroy func_edit_destroy,
                        FPtr_set_listener func_edit_OnFilter,
                        FPtr_set_listener func_edit_OnChange,
                        FPtr_set_listener func_edit_OnFocus,
                        FPtr_set_text func_edit_set_text,
                        FPtr_set_text func_edit_set_tooltip,
                        FPtr_set_const_ptr func_edit_set_font,
                        FPtr_set_enum func_edit_set_align,
                        FPtr_set_bool func_edit_set_passmode,
                        FPtr_set_bool func_edit_set_editable,
                        FPtr_set_bool func_edit_set_autoselect,
                        FPtr_set_uint32 func_edit_set_text_color,
                        FPtr_set_uint32 func_edit_set_bg_color,
                        FPtr_bounds4 func_edit_bounds,
                        FPtr_set_ptr func_attach_edit_to_panel,
                        FPtr_set_ptr func_detach_edit_from_panel,
                        FPtr_set_bool func_edit_set_visible,
                        FPtr_set_bool func_edit_set_enabled,
                        FPtr_get2_real32 func_edit_get_size,
                        FPtr_get2_real32 func_edit_get_origin,
                        FPtr_set4_real32 func_edit_set_frame);
#define gui_context_append_edit_manager(\
                        context,\
                        func_edit_create,\
                        func_edit_destroy,\
                        func_edit_OnFilter,\
                        func_edit_OnChange,\
                        func_edit_OnFocus,\
                        func_edit_set_text,\
                        func_edit_set_tooltip,\
                        func_edit_set_font,\
                        func_edit_set_align,\
                        func_edit_set_passmode,\
                        func_edit_set_editable,\
                        func_edit_set_autoselect,\
                        func_edit_set_text_color,\
                        func_edit_set_bg_color,\
                        func_edit_bounds,\
                        func_attach_edit_to_panel,\
                        func_detach_edit_from_panel,\
                        func_edit_set_visible,\
                        func_edit_set_enabled,\
                        func_edit_get_size,\
                        func_edit_get_origin,\
                        func_edit_set_frame,\
                        edit_type, panel_type, font_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_edit_create, edit_type, edit_flag_t),\
        FUNC_CHECK_DESTROY(func_edit_destroy, edit_type),\
        FUNC_CHECK_SET_LISTENER(func_edit_OnFilter, edit_type),\
        FUNC_CHECK_SET_LISTENER(func_edit_OnChange, edit_type),\
        FUNC_CHECK_SET_LISTENER(func_edit_OnFocus, edit_type),\
        FUNC_CHECK_SET_TEXT(func_edit_set_text, edit_type),\
        FUNC_CHECK_SET_TEXT(func_edit_set_tooltip, edit_type),\
        FUNC_CHECK_SET_CONST_PTR(func_edit_set_font, edit_type, font_type),\
        FUNC_CHECK_SET_ENUM(func_edit_set_align, edit_type, align_t),\
        FUNC_CHECK_SET_BOOL(func_edit_set_passmode, edit_type),\
        FUNC_CHECK_SET_BOOL(func_edit_set_editable, edit_type),\
        FUNC_CHECK_SET_BOOL(func_edit_set_autoselect, edit_type),\
        FUNC_CHECK_SET_UINT32(func_edit_set_text_color, edit_type),\
        FUNC_CHECK_SET_UINT32(func_edit_set_bg_color, edit_type),\
        FUNC_CHECK_BOUNDS4(func_edit_bounds, edit_type),\
        FUNC_CHECK_SET_PTR(func_attach_edit_to_panel, edit_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_edit_from_panel, edit_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_edit_set_visible, edit_type),\
        FUNC_CHECK_SET_BOOL(func_edit_set_enabled, edit_type),\
        FUNC_CHECK_GET2_REAL32(func_edit_get_size, edit_type),\
        FUNC_CHECK_GET2_REAL32(func_edit_get_origin, edit_type),\
        FUNC_CHECK_SET4_REAL32(func_edit_set_frame, edit_type),\
        _gui_context_append_edit_manager(\
                        context,\
                        (FPtr_create_enum)func_edit_create,\
                        (FPtr_destroy)func_edit_destroy,\
                        (FPtr_set_listener)func_edit_OnFilter,\
                        (FPtr_set_listener)func_edit_OnChange,\
                        (FPtr_set_listener)func_edit_OnFocus,\
                        (FPtr_set_text)func_edit_set_text,\
                        (FPtr_set_text)func_edit_set_tooltip,\
                        (FPtr_set_const_ptr)func_edit_set_font,\
                        (FPtr_set_enum)func_edit_set_align,\
                        (FPtr_set_bool)func_edit_set_passmode,\
                        (FPtr_set_bool)func_edit_set_editable,\
                        (FPtr_set_bool)func_edit_set_autoselect,\
                        (FPtr_set_uint32)func_edit_set_text_color,\
                        (FPtr_set_uint32)func_edit_set_bg_color,\
                        (FPtr_bounds4)func_edit_bounds,\
                        (FPtr_set_ptr)func_attach_edit_to_panel,\
                        (FPtr_set_ptr)func_detach_edit_from_panel,\
                        (FPtr_set_bool)func_edit_set_visible,\
                        (FPtr_set_bool)func_edit_set_enabled,\
                        (FPtr_get2_real32)func_edit_get_size,\
                        (FPtr_get2_real32)func_edit_get_origin,\
                        (FPtr_set4_real32)func_edit_set_frame)\
    )

void _gui_context_append_combo_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_combo_create,
                        FPtr_destroy func_combo_destroy,
                        FPtr_set_listener func_combo_OnFilter,
                        FPtr_set_listener func_combo_OnChange,
                        FPtr_set_listener func_combo_OnFocus,
                        FPtr_set_listener func_combo_OnSelect,
                        FPtr_set_text func_combo_set_text,
                        FPtr_set_text func_combo_set_tooltip,
                        FPtr_set_const_ptr func_combo_set_font,
                        FPtr_set_enum func_combo_set_align,
                        FPtr_set_bool func_combo_set_passmode,
                        FPtr_set_uint32 func_combo_set_text_color,
                        FPtr_set_uint32 func_combo_set_bg_color,
                        FPtr_set_elem func_combo_set_elem,
                        FPtr_set_uint32 func_combo_set_selected,
                        FPtr_get_uint32 func_combo_get_selected,
                        FPtr_bounds5 func_combo_bounds,
                        FPtr_set_ptr func_attach_combo_to_panel,
                        FPtr_set_ptr func_detach_combo_from_panel,
                        FPtr_set_bool func_combo_set_visible,
                        FPtr_set_bool func_combo_set_enabled,
                        FPtr_get2_real32 func_combo_get_size,
                        FPtr_get2_real32 func_combo_get_origin,
                        FPtr_set4_real32 func_combo_set_frame);
#define gui_context_append_combo_manager(\
                        context,\
                        func_combo_create,\
                        func_combo_destroy,\
                        func_combo_OnFilter,\
                        func_combo_OnChange,\
                        func_combo_OnFocus,\
                        func_combo_OnSelect,\
                        func_combo_set_text,\
                        func_combo_set_tooltip,\
                        func_combo_set_font,\
                        func_combo_set_align,\
                        func_combo_set_passmode,\
                        func_combo_set_text_color,\
                        func_combo_set_bg_color,\
                        func_combo_set_elem,\
                        func_combo_set_selected,\
                        func_combo_get_selected,\
                        func_combo_bounds,\
                        func_attach_combo_to_panel,\
                        func_detach_combo_from_panel,\
                        func_combo_set_visible,\
                        func_combo_set_enabled,\
                        func_combo_get_size,\
                        func_combo_get_origin,\
                        func_combo_set_frame,\
                        combo_type, panel_type, image_type, font_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_combo_create, combo_type, combo_flag_t),\
        FUNC_CHECK_DESTROY(func_combo_destroy, combo_type),\
        FUNC_CHECK_SET_LISTENER(func_combo_OnFilter, combo_type),\
        FUNC_CHECK_SET_LISTENER(func_combo_OnChange, combo_type),\
        FUNC_CHECK_SET_LISTENER(func_combo_OnFocus, combo_type),\
        FUNC_CHECK_SET_LISTENER(func_combo_OnSelect, combo_type),\
        FUNC_CHECK_SET_TEXT(func_combo_set_text, combo_type),\
        FUNC_CHECK_SET_TEXT(func_combo_set_tooltip, combo_type),\
        FUNC_CHECK_SET_CONST_PTR(func_combo_set_font, combo_type, font_type),\
        FUNC_CHECK_SET_ENUM(func_combo_set_align, combo_type, align_t),\
        FUNC_CHECK_SET_BOOL(func_combo_set_passmode, combo_type),\
        FUNC_CHECK_SET_UINT32(func_combo_set_text_color, combo_type),\
        FUNC_CHECK_SET_UINT32(func_combo_set_bg_color, combo_type),\
        FUNC_CHECK_SET_ELEM(func_combo_set_elem, combo_type),\
        FUNC_CHECK_SET_UINT32(func_combo_set_selected, combo_type),\
        FUNC_CHECK_GET_UINT32(func_combo_get_selected, combo_type),\
        FUNC_CHECK_BOUNDS5(func_combo_bounds, combo_type),\
        FUNC_CHECK_SET_PTR(func_attach_combo_to_panel, combo_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_combo_from_panel, combo_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_combo_set_visible, combo_type),\
        FUNC_CHECK_SET_BOOL(func_combo_set_enabled, combo_type),\
        FUNC_CHECK_GET2_REAL32(func_combo_get_size, combo_type),\
        FUNC_CHECK_GET2_REAL32(func_combo_get_origin, combo_type),\
        FUNC_CHECK_SET4_REAL32(func_combo_set_frame, combo_type),\
        _gui_context_append_combo_manager(\
                        context,\
                        (FPtr_create_enum)func_combo_create,\
                        (FPtr_destroy)func_combo_destroy,\
                        (FPtr_set_listener)func_combo_OnFilter,\
                        (FPtr_set_listener)func_combo_OnChange,\
                        (FPtr_set_listener)func_combo_OnFocus,\
                        (FPtr_set_listener)func_combo_OnSelect,\
                        (FPtr_set_text)func_combo_set_text,\
                        (FPtr_set_text)func_combo_set_tooltip,\
                        (FPtr_set_const_ptr)func_combo_set_font,\
                        (FPtr_set_enum)func_combo_set_align,\
                        (FPtr_set_bool)func_combo_set_passmode,\
                        (FPtr_set_uint32)func_combo_set_text_color,\
                        (FPtr_set_uint32)func_combo_set_bg_color,\
                        (FPtr_set_elem)func_combo_set_elem,\
                        (FPtr_set_uint32)func_combo_set_selected,\
                        (FPtr_get_uint32)func_combo_get_selected,\
                        (FPtr_bounds5)func_combo_bounds,\
                        (FPtr_set_ptr)func_attach_combo_to_panel,\
                        (FPtr_set_ptr)func_detach_combo_from_panel,\
                        (FPtr_set_bool)func_combo_set_visible,\
                        (FPtr_set_bool)func_combo_set_enabled,\
                        (FPtr_get2_real32)func_combo_get_size,\
                        (FPtr_get2_real32)func_combo_get_origin,\
                        (FPtr_set4_real32)func_combo_set_frame)\
    )

void _gui_context_append_updown_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_updown_create,
                        FPtr_destroy func_updown_destroy,
                        FPtr_set_listener func_updown_OnClick,
                        FPtr_set_text func_updown_set_tooltip,
                        FPtr_set_ptr func_attach_updown_to_panel,
                        FPtr_set_ptr func_detach_updown_from_panel,
                        FPtr_set_bool func_updown_set_visible,
                        FPtr_set_bool func_updown_set_enabled,
                        FPtr_get2_real32 func_updown_get_size,
                        FPtr_get2_real32 func_updown_get_origin,
                        FPtr_set4_real32 func_updown_set_frame);
#define gui_context_append_updown_manager(\
                        context,\
                        func_updown_create,\
                        func_updown_destroy,\
                        func_updown_OnClick,\
                        func_updown_set_tooltip,\
                        func_attach_updown_to_panel,\
                        func_detach_updown_from_panel,\
                        func_updown_set_visible,\
                        func_updown_set_enabled,\
                        func_updown_get_size,\
                        func_updown_get_origin,\
                        func_updown_set_frame,\
                        updown_type, panel_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_updown_create, updown_type, updown_flag_t),\
        FUNC_CHECK_DESTROY(func_updown_destroy, updown_type),\
        FUNC_CHECK_SET_LISTENER(func_updown_OnClick, updown_type),\
        FUNC_CHECK_SET_TEXT(func_updown_set_tooltip, updown_type),\
        FUNC_CHECK_SET_PTR(func_attach_updown_to_panel, updown_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_updown_from_panel, updown_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_updown_set_visible, updown_type),\
        FUNC_CHECK_SET_BOOL(func_updown_set_enabled, updown_type),\
        FUNC_CHECK_GET2_REAL32(func_updown_get_size, updown_type),\
        FUNC_CHECK_GET2_REAL32(func_updown_get_origin, updown_type),\
        FUNC_CHECK_SET4_REAL32(func_updown_set_frame, updown_type),\
        _gui_context_append_updown_manager(\
                        context,\
                        (FPtr_create_enum)func_updown_create,\
                        (FPtr_destroy)func_updown_destroy,\
                        (FPtr_set_listener)func_updown_OnClick,\
                        (FPtr_set_text)func_updown_set_tooltip,\
                        (FPtr_set_ptr)func_attach_updown_to_panel,\
                        (FPtr_set_ptr)func_detach_updown_from_panel,\
                        (FPtr_set_bool)func_updown_set_visible,\
                        (FPtr_set_bool)func_updown_set_enabled,\
                        (FPtr_get2_real32)func_updown_get_size,\
                        (FPtr_get2_real32)func_updown_get_origin,\
                        (FPtr_set4_real32)func_updown_set_frame)\
    )

void _gui_context_append_slider_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_slider_create,
                        FPtr_destroy func_slider_destroy,
                        FPtr_set_listener func_slider_OnMoved,
                        FPtr_set_text func_slider_set_tooltip,
                        FPtr_bar_tickmarks func_slider_set_tickmarks,
                        FPtr_get_real32 func_slider_get_position,
                        FPtr_set_real32 func_slider_set_position,
                        FPtr_bounds6 func_slider_bounds,
                        FPtr_set_ptr func_attach_slider_to_panel,
                        FPtr_set_ptr func_detach_slider_from_panel,
                        FPtr_set_bool func_slider_set_visible,
                        FPtr_set_bool func_slider_set_enabled,
                        FPtr_get2_real32 func_slider_get_size,
                        FPtr_get2_real32 func_slider_get_origin,
                        FPtr_set4_real32 func_slider_set_frame);
#define gui_context_append_slider_manager(\
                        context,\
                        func_slider_create,\
                        func_slider_destroy,\
                        func_slider_OnMoved,\
                        func_slider_set_tooltip,\
                        func_slider_set_tickmarks,\
                        func_slider_get_position,\
                        func_slider_set_position,\
                        func_slider_bounds,\
                        func_attach_slider_to_panel,\
                        func_detach_slider_from_panel,\
                        func_slider_set_visible,\
                        func_slider_set_enabled,\
                        func_slider_get_size,\
                        func_slider_get_origin,\
                        func_slider_set_frame,\
                        slider_type, panel_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_slider_create, slider_type, slider_flag_t),\
        FUNC_CHECK_DESTROY(func_slider_destroy, slider_type),\
        FUNC_CHECK_SET_LISTENER(func_slider_OnMoved, slider_type),\
        FUNC_CHECK_SET_TEXT(func_slider_set_tooltip, slider_type),\
        FUNC_CHECK_BAR_TICKMARKS(func_slider_set_tickmarks, slider_type),\
        FUNC_CHECK_GET_REAL32(func_slider_get_position, slider_type),\
        FUNC_CHECK_SET_REAL32(func_slider_set_position, slider_type),\
        FUNC_CHECK_BOUNDS6(func_slider_bounds, slider_type),\
        FUNC_CHECK_SET_PTR(func_attach_slider_to_panel, slider_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_slider_from_panel, slider_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_slider_set_visible, slider_type),\
        FUNC_CHECK_SET_BOOL(func_slider_set_enabled, slider_type),\
        FUNC_CHECK_GET2_REAL32(func_slider_get_size, slider_type),\
        FUNC_CHECK_GET2_REAL32(func_slider_get_origin, slider_type),\
        FUNC_CHECK_SET4_REAL32(func_slider_set_frame, slider_type),\
        _gui_context_append_slider_manager(\
                        context,\
                        (FPtr_create_enum)func_slider_create,\
                        (FPtr_destroy)func_slider_destroy,\
                        (FPtr_set_listener)func_slider_OnMoved,\
                        (FPtr_set_text)func_slider_set_tooltip,\
                        (FPtr_bar_tickmarks)func_slider_set_tickmarks,\
                        (FPtr_get_real32)func_slider_get_position,\
                        (FPtr_set_real32)func_slider_set_position,\
                        (FPtr_bounds6)func_slider_bounds,\
                        (FPtr_set_ptr)func_attach_slider_to_panel,\
                        (FPtr_set_ptr)func_detach_slider_from_panel,\
                        (FPtr_set_bool)func_slider_set_visible,\
                        (FPtr_set_bool)func_slider_set_enabled,\
                        (FPtr_get2_real32)func_slider_get_size,\
                        (FPtr_get2_real32)func_slider_get_origin,\
                        (FPtr_set4_real32)func_slider_set_frame)\
    )

void _gui_context_append_progress_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_progress_create,
                        FPtr_destroy func_progress_destroy,
                        FPtr_set_real32 func_progress_set_position,
                        FPtr_get_real32_enum func_progress_get_thickness,
                        FPtr_set_ptr func_attach_progress_to_panel,
                        FPtr_set_ptr func_detach_progress_from_panel,
                        FPtr_set_bool func_progress_set_visible,
                        FPtr_set_bool func_progress_set_enabled,
                        FPtr_get2_real32 func_progress_get_size,
                        FPtr_get2_real32 func_progress_get_origin,
                        FPtr_set4_real32 func_progress_set_frame);
#define gui_context_append_progress_manager(\
                        context,\
                        func_progress_create,\
                        func_progress_destroy,\
                        func_progress_set_position,\
                        func_progress_get_thickness,\
                        func_attach_progress_to_panel,\
                        func_detach_progress_from_panel,\
                        func_progress_set_visible,\
                        func_progress_set_enabled,\
                        func_progress_get_size,\
                        func_progress_get_origin,\
                        func_progress_set_frame,\
                        progress_type, panel_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_progress_create, progress_type, progress_flag_t),\
        FUNC_CHECK_DESTROY(func_progress_destroy, progress_type),\
        FUNC_CHECK_SET_REAL32(func_progress_set_position, progress_type),\
        FUNC_CHECK_GET_REAL32_ENUM(func_progress_get_thickness, progress_type, fsize_t),\
        FUNC_CHECK_SET_PTR(func_attach_progress_to_panel, progress_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_progress_from_panel, progress_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_progress_set_visible, progress_type),\
        FUNC_CHECK_SET_BOOL(func_progress_set_enabled, progress_type),\
        FUNC_CHECK_GET2_REAL32(func_progress_get_size, progress_type),\
        FUNC_CHECK_GET2_REAL32(func_progress_get_origin, progress_type),\
        FUNC_CHECK_SET4_REAL32(func_progress_set_frame, progress_type),\
        _gui_context_append_progress_manager(\
                        context,\
                        (FPtr_create_enum)func_progress_create,\
                        (FPtr_destroy)func_progress_destroy,\
                        (FPtr_set_real32)func_progress_set_position,\
                        (FPtr_get_real32_enum)func_progress_get_thickness,\
                        (FPtr_set_ptr)func_attach_progress_to_panel,\
                        (FPtr_set_ptr)func_detach_progress_from_panel,\
                        (FPtr_set_bool)func_progress_set_visible,\
                        (FPtr_set_bool)func_progress_set_enabled,\
                        (FPtr_get2_real32)func_progress_get_size,\
                        (FPtr_get2_real32)func_progress_get_origin,\
                        (FPtr_set4_real32)func_progress_set_frame)\
    )

void _gui_context_append_text_manager(
                        GuiContext *context,  
                        FPtr_create_enum func_text_create,
                        FPtr_destroy func_text_destroy,
                        FPtr_set_listener func_text_OnTextChange,
                        FPtr_set_text func_text_insert_text,
                        FPtr_set_text func_text_set_text,
                        FPtr_set_ptr func_text_set_rtf,
                        FPtr_set_property func_text_set_prop,
                        FPtr_set_bool func_text_set_editable,
                        FPtr_get_text func_text_get_text,
                        FPtr_call func_text_set_need_display,
                        FPtr_set_ptr func_attach_text_to_panel,
                        FPtr_set_ptr func_detach_text_from_panel,
                        FPtr_set_bool func_text_set_visible,
                        FPtr_set_bool func_text_set_enabled,
                        FPtr_get2_real32 func_text_get_size,
                        FPtr_get2_real32 func_text_get_origin,
                        FPtr_set4_real32 func_text_set_frame);
#define gui_context_append_text_manager(\
                        context,\
                        func_text_create,\
                        func_text_destroy,\
                        func_text_OnTextChange,\
                        func_text_insert_text,\
                        func_text_set_text,\
                        func_text_set_rtf,\
                        func_text_set_prop,\
                        func_text_set_editable,\
                        func_text_get_text,\
                        func_text_set_need_display,\
                        func_attach_text_to_panel,\
                        func_detach_text_from_panel,\
                        func_text_set_visible,\
                        func_text_set_enabled,\
                        func_text_get_size,\
                        func_text_get_origin,\
                        func_text_set_frame,\
                        text_type, panel_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_text_create, text_type, tview_flag_t),\
        FUNC_CHECK_DESTROY(func_text_destroy, text_type),\
        FUNC_CHECK_SET_LISTENER(func_text_OnTextChange, text_type),\
        FUNC_CHECK_SET_TEXT(func_text_insert_text, text_type),\
        FUNC_CHECK_SET_TEXT(func_text_set_text, text_type),\
        FUNC_CHECK_SET_PTR(func_text_set_rtf, text_type, Stream),\
        FUNC_CHECK_SET_PROPERTY(func_text_set_prop, text_type),\
        FUNC_CHECK_SET_BOOL(func_text_set_editable, text_type),\
        FUNC_CHECK_GET_TEXT(func_text_get_text, text_type),\
        FUNC_CHECK_CALL(func_text_set_need_display, text_type),\
        FUNC_CHECK_SET_PTR(func_attach_text_to_panel, text_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_text_from_panel, text_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_text_set_visible, text_type),\
        FUNC_CHECK_SET_BOOL(func_text_set_enabled, text_type),\
        FUNC_CHECK_GET2_REAL32(func_text_get_size, text_type),\
        FUNC_CHECK_GET2_REAL32(func_text_get_origin, text_type),\
        FUNC_CHECK_SET4_REAL32(func_text_set_frame, text_type),\
        _gui_context_append_text_manager(\
                        context,\
                        (FPtr_create_enum)func_text_create,\
                        (FPtr_destroy)func_text_destroy,\
                        (FPtr_set_listener)func_text_OnTextChange,\
                        (FPtr_set_text)func_text_insert_text,\
                        (FPtr_set_text)func_text_set_text,\
                        (FPtr_set_ptr)func_text_set_rtf,\
                        (FPtr_set_property)func_text_set_prop,\
                        (FPtr_set_bool)func_text_set_editable,\
                        (FPtr_get_text)func_text_get_text,\
                        (FPtr_call)func_text_set_need_display,\
                        (FPtr_set_ptr)func_attach_text_to_panel,\
                        (FPtr_set_ptr)func_detach_text_from_panel,\
                        (FPtr_set_bool)func_text_set_visible,\
                        (FPtr_set_bool)func_text_set_enabled,\
                        (FPtr_get2_real32)func_text_get_size,\
                        (FPtr_get2_real32)func_text_get_origin,\
                        (FPtr_set4_real32)func_text_set_frame)\
    )

void _gui_context_append_split_manager(
                        GuiContext *gui_context,  
                        FPtr_create_enum func_split_create,
                        FPtr_destroy func_split_destroy,
                        FPtr_set_ptr func_split_attach_control,
                        FPtr_set_ptr func_split_detach_control,
                        FPtr_set_listener func_split_OnMoved,
                        FPtr_set4_real32 func_split_track_area,
                        FPtr_set_ptr func_attach_split_to_panel,
                        FPtr_set_ptr func_detach_split_from_panel,
                        FPtr_set_bool func_split_set_visible,
                        FPtr_set_bool func_split_set_enabled,
                        FPtr_get2_real32 func_split_get_size,
                        FPtr_get2_real32 func_split_get_origin,
                        FPtr_set4_real32 func_split_set_frame);
#define gui_context_append_split_manager(\
                        gui_context,\
                        func_split_create,\
                        func_split_destroy,\
                        func_split_attach_control,\
                        func_split_detach_control,\
                        func_split_OnMoved,\
                        func_split_track_area,\
                        func_attach_split_to_panel,\
                        func_detach_split_from_panel,\
                        func_split_set_visible,\
                        func_split_set_enabled,\
                        func_split_get_size,\
                        func_split_get_origin,\
                        func_split_set_frame,\
                        splitv_type, panel_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_split_create, splitv_type, split_flag_t),\
        FUNC_CHECK_DESTROY(func_split_destroy, splitv_type),\
        FUNC_CHECK_SET_PTR(func_split_attach_control, splitv_type, OSControl),\
        FUNC_CHECK_SET_PTR(func_split_detach_control, splitv_type, OSControl),\
        FUNC_CHECK_SET_LISTENER(func_split_OnMoved, splitv_type),\
        FUNC_CHECK_SET4_REAL32(func_split_track_area, splitv_type),\
        FUNC_CHECK_SET_PTR(func_attach_split_to_panel, splitv_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_split_from_panel, splitv_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_split_set_visible, splitv_type),\
        FUNC_CHECK_SET_BOOL(func_split_set_enabled, splitv_type),\
        FUNC_CHECK_GET2_REAL32(func_split_get_size, splitv_type),\
        FUNC_CHECK_GET2_REAL32(func_split_get_origin, splitv_type),\
        FUNC_CHECK_SET4_REAL32(func_split_set_frame, splitv_type),\
        _gui_context_append_split_manager(\
                        gui_context,\
                        (FPtr_create_enum)func_split_create,\
                        (FPtr_destroy)func_split_destroy,\
                        (FPtr_set_ptr)func_split_attach_control,\
                        (FPtr_set_ptr)func_split_detach_control,\
                        (FPtr_set_listener)func_split_OnMoved,\
                        (FPtr_set4_real32)func_split_track_area,\
                        (FPtr_set_ptr)func_attach_split_to_panel,\
                        (FPtr_set_ptr)func_detach_split_from_panel,\
                        (FPtr_set_bool)func_split_set_visible,\
                        (FPtr_set_bool)func_split_set_enabled,\
                        (FPtr_get2_real32)func_split_get_size,\
                        (FPtr_get2_real32)func_split_get_origin,\
                        (FPtr_set4_real32)func_split_set_frame)\
    )

void _gui_context_append_view_manager(
                        GuiContext *context,
                        FPtr_create func_view_create,
                        FPtr_destroy func_view_destroy,
                        FPtr_set_listener func_view_OnDraw,
                        FPtr_set_listener func_view_OnEnter,
                        FPtr_set_listener func_view_OnExit,
                        FPtr_set_listener func_view_OnMoved,
                        FPtr_set_listener func_view_OnDown,
                        FPtr_set_listener func_view_OnUp,
                        FPtr_set_listener func_view_OnClick,
                        FPtr_set_listener func_view_OnDrag,
                        FPtr_set_listener func_view_OnWheel,
                        FPtr_set_listener func_view_OnKeyDown,
                        FPtr_set_listener func_view_OnKeyUp,
                        FPtr_set_listener func_view_OnFocus,
                        FPtr_set_listener func_view_OnNotify,
                        FPtr_set_listener func_view_OnTouchTap,
                        FPtr_set_listener func_view_OnTouchStartDrag,
                        FPtr_set_listener func_view_OnTouchDragging,
                        FPtr_set_listener func_view_OnTouchEndDrag,
                        FPtr_set_listener func_view_OnTouchStartPinch,
                        FPtr_set_listener func_view_OnTouchPinching,
                        FPtr_set_listener func_view_OnTouchEndPinch,
                        FPtr_set2_real32 func_view_scroll,
                        FPtr_get2_real32 func_view_scroll_get,
                        FPtr_get2_real32 func_view_scroller_size,
                        FPtr_set4_real32 func_view_content_size,
                        FPtr_get_real32 func_view_scale_factor,
                        FPtr_call func_view_set_need_display,
                        FPtr_set_bool func_view_set_drawable,
                        FPtr_get_ptr func_view_get_native_view,
                        FPtr_set_ptr func_attach_view_to_panel,
                        FPtr_set_ptr func_detach_view_from_panel,
                        FPtr_set_bool func_view_set_visible,
                        FPtr_set_bool func_view_set_enabled,
                        FPtr_get2_real32 func_view_get_size,
                        FPtr_get2_real32 func_view_get_origin,
                        FPtr_set4_real32 func_view_set_frame);
#define gui_context_append_view_manager(\
                        context,\
                        func_view_create,\
                        func_view_destroy,\
                        func_view_OnDraw,\
                        func_view_OnEnter,\
                        func_view_OnExit,\
                        func_view_OnMoved,\
                        func_view_OnDown,\
                        func_view_OnUp,\
                        func_view_OnClick,\
                        func_view_OnDrag,\
                        func_view_OnWheel,\
                        func_view_OnKeyDown,\
                        func_view_OnKeyUp,\
                        func_view_OnFocus,\
                        func_view_OnNotify,\
                        func_view_OnTouchTap,\
                        func_view_OnTouchStartDrag,\
                        func_view_OnTouchDragging,\
                        func_view_OnTouchEndDrag,\
                        func_view_OnTouchStartPinch,\
                        func_view_OnTouchPinching,\
                        func_view_OnTouchEndPinch,\
                        func_view_scroll,\
                        func_view_scroll_get,\
                        func_view_scroller_size,\
                        func_view_content_size,\
                        func_view_scale_factor,\
                        func_view_set_need_display,\
                        func_view_set_drawable,\
                        func_view_get_native_view,\
                        func_attach_view_to_panel,\
                        func_detach_view_from_panel,\
                        func_view_set_visible,\
                        func_view_set_enabled,\
                        func_view_get_size,\
                        func_view_get_origin,\
                        func_view_set_frame,\
                        view_type, panel_type)\
    (\
        FUNC_CHECK_CREATE(func_view_create, view_type),\
        FUNC_CHECK_DESTROY(func_view_destroy, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnDraw, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnEnter, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnExit, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnMoved, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnDown, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnUp, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnClick, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnDrag, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnWheel, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnKeyDown, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnKeyUp, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnFocus, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnNotify, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnTouchTap, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnTouchStartDrag, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnTouchDragging, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnTouchEndDrag, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnTouchStartPinch, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnTouchPinching, view_type),\
        FUNC_CHECK_SET_LISTENER(func_view_OnTouchEndPinch, view_type),\
        FUNC_CHECK_SET2_REAL32(func_view_scroll, view_type),\
        FUNC_CHECK_GET2_REAL32(func_view_scroll_get, view_type),\
        FUNC_CHECK_GET2_REAL32(func_view_scroller_size, view_type),\
        FUNC_CHECK_SET4_REAL32(func_view_content_size, view_type),\
        FUNC_CHECK_GET_REAL32(func_view_scale_factor, view_type),\
        FUNC_CHECK_CALL(func_view_set_need_display, view_type),\
        FUNC_CHECK_SET_BOOL(func_view_set_drawable, view_type),\
        FUNC_CHECK_GET_PTR(func_view_get_native_view, view_type, void),\
        FUNC_CHECK_SET_PTR(func_attach_view_to_panel, view_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_view_from_panel, view_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_view_set_visible, view_type),\
        FUNC_CHECK_SET_BOOL(func_view_set_enabled, view_type),\
        FUNC_CHECK_GET2_REAL32(func_view_get_size, view_type),\
        FUNC_CHECK_GET2_REAL32(func_view_get_origin, view_type),\
        FUNC_CHECK_SET4_REAL32(func_view_set_frame, view_type),\
        _gui_context_append_view_manager(\
                        context,\
                        (FPtr_create)func_view_create,\
                        (FPtr_destroy)func_view_destroy,\
                        (FPtr_set_listener)func_view_OnDraw,\
                        (FPtr_set_listener)func_view_OnEnter,\
                        (FPtr_set_listener)func_view_OnExit,\
                        (FPtr_set_listener)func_view_OnMoved,\
                        (FPtr_set_listener)func_view_OnDown,\
                        (FPtr_set_listener)func_view_OnUp,\
                        (FPtr_set_listener)func_view_OnClick,\
                        (FPtr_set_listener)func_view_OnDrag,\
                        (FPtr_set_listener)func_view_OnWheel,\
                        (FPtr_set_listener)func_view_OnKeyDown,\
                        (FPtr_set_listener)func_view_OnKeyUp,\
                        (FPtr_set_listener)func_view_OnFocus,\
                        (FPtr_set_listener)func_view_OnNotify,\
                        (FPtr_set_listener)func_view_OnTouchTap,\
                        (FPtr_set_listener)func_view_OnTouchStartDrag,\
                        (FPtr_set_listener)func_view_OnTouchDragging,\
                        (FPtr_set_listener)func_view_OnTouchEndDrag,\
                        (FPtr_set_listener)func_view_OnTouchStartPinch,\
                        (FPtr_set_listener)func_view_OnTouchPinching,\
                        (FPtr_set_listener)func_view_OnTouchEndPinch,\
                        (FPtr_set2_real32)func_view_scroll,\
                        (FPtr_get2_real32)func_view_scroll_get,\
                        (FPtr_get2_real32)func_view_scroller_size,\
                        (FPtr_set4_real32)func_view_content_size,\
                        (FPtr_get_real32)func_view_scale_factor,\
                        (FPtr_call)func_view_set_need_display,\
                        (FPtr_set_bool)func_view_set_drawable,\
                        (FPtr_get_ptr)func_view_get_native_view,\
                        (FPtr_set_ptr)func_attach_view_to_panel,\
                        (FPtr_set_ptr)func_detach_view_from_panel,\
                        (FPtr_set_bool)func_view_set_visible,\
                        (FPtr_set_bool)func_view_set_enabled,\
                        (FPtr_get2_real32)func_view_get_size,\
                        (FPtr_get2_real32)func_view_get_origin,\
                        (FPtr_set4_real32)func_view_set_frame)\
    )

void _gui_context_append_panel_manager(
                        GuiContext *context,    
                        FPtr_create func_panel_create,
                        FPtr_destroy func_panel_destroy,
                        FPtr_area func_panel_area,
                        FPtr_set4_real32 func_panel_content_size,
                        FPtr_call func_panel_set_need_display,
                        FPtr_set_ptr func_attach_panel_to_panel,
                        FPtr_set_ptr func_detach_panel_from_panel,
                        FPtr_set_bool func_panel_set_visible,
                        FPtr_set_bool func_panel_set_enabled,
                        FPtr_get2_real32 func_panel_get_size,
                        FPtr_get2_real32 func_panel_get_origin,
                        FPtr_set4_real32 func_panel_set_frame);
#define gui_context_append_panel_manager(\
                        context,\
                        func_panel_create,\
                        func_panel_destroy,\
                        func_panel_area,\
                        func_panel_content_size,\
                        func_panel_set_need_display,\
                        func_attach_panel_to_panel,\
                        func_detach_panel_from_panel,\
                        func_panel_set_visible,\
                        func_panel_set_enabled,\
                        func_panel_get_size,\
                        func_panel_get_origin,\
                        func_panel_set_frame,\
                        panel_type)\
    (\
        FUNC_CHECK_CREATE(func_panel_create, panel_type),\
        FUNC_CHECK_DESTROY(func_panel_destroy, panel_type),\
        FUNC_CHECK_AREA(func_panel_area, panel_type),\
        FUNC_CHECK_SET4_REAL32(func_panel_content_size, panel_type),\
        FUNC_CHECK_CALL(func_panel_set_need_display, panel_type),\
        FUNC_CHECK_SET_PTR(func_attach_panel_to_panel, panel_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_panel_from_panel, panel_type, panel_type),\
        FUNC_CHECK_SET_BOOL(func_panel_set_visible, panel_type),\
        FUNC_CHECK_SET_BOOL(func_panel_set_enabled, panel_type),\
        FUNC_CHECK_GET2_REAL32(func_panel_get_size, panel_type),\
        FUNC_CHECK_GET2_REAL32(func_panel_get_origin, panel_type),\
        FUNC_CHECK_SET4_REAL32(func_panel_set_frame, panel_type),\
        _gui_context_append_panel_manager(\
                        context,\
                        (FPtr_create)func_panel_create,\
                        (FPtr_destroy)func_panel_destroy,\
                        (FPtr_area)func_panel_area,\
                        (FPtr_set4_real32)func_panel_content_size,\
                        (FPtr_call)func_panel_set_need_display,\
                        (FPtr_set_ptr)func_attach_panel_to_panel,\
                        (FPtr_set_ptr)func_detach_panel_from_panel,\
                        (FPtr_set_bool)func_panel_set_visible,\
                        (FPtr_set_bool)func_panel_set_enabled,\
                        (FPtr_get2_real32)func_panel_get_size,\
                        (FPtr_get2_real32)func_panel_get_origin,\
                        (FPtr_set4_real32)func_panel_set_frame)\
    )

void _gui_context_append_window_manager(
                        GuiContext *context,
                        FPtr_create_enum func_window_create,
                        FPtr_window_managed func_window_managed,
                        FPtr_destroy func_window_destroy,
                        FPtr_set_listener func_window_OnMoved,
                        FPtr_set_listener func_window_OnResize,
                        FPtr_set_listener func_window_OnClose,
                        FPtr_set_text func_window_set_title,
                        FPtr_set_bool func_window_set_edited,
                        FPtr_set_bool func_window_set_movable,
                        FPtr_window_z_order func_window_set_z_order,
                        FPtr_set_real32 func_window_set_alpha,
                        FPtr_set_bool func_window_enable_mouse_events,
                        FPtr_set_ptr func_window_set_taborder,
                        FPtr_set_ptr func_window_set_focus,
                        FPtr_set_ptr func_attach_main_panel_to_window,
                        FPtr_set_ptr func_detach_main_panel_from_window,
                        FPtr_set_ptr func_attach_window_to_window,
                        FPtr_set_ptr func_detach_window_from_window,
                        FPtr_set_ptr func_window_launch,
                        FPtr_set_ptr func_window_hide,
                        FPtr_window_modal func_window_launch_modal,
                        FPtr_set_uint32 func_window_stop_modal,
                        FPtr_get2_real32 func_window_get_origin_in_screen_coordinates,
                        FPtr_set2_real32 func_window_set_origin_in_screen_coordinates,
                        FPtr_get2_real32 func_window_get_size,
                        FPtr_set2_real32 func_window_set_size,
                        FPtr_set_ptr func_window_set_default_pushbutton,
                        FPtr_set_ptr func_window_set_cursor,
                        FPtr_set_property func_window_set_property);
#define gui_context_append_window_manager(\
                        context,\
                        func_window_create,\
                        func_window_managed,\
                        func_window_destroy,\
                        func_window_OnMoved,\
                        func_window_OnResize,\
                        func_window_OnClose,\
                        func_window_set_title,\
                        func_window_set_edited,\
                        func_window_set_movable,\
                        func_window_set_z_order,\
                        func_window_set_alpha,\
                        func_window_enable_mouse_events,\
                        func_window_set_taborder,\
                        func_window_set_focus,\
                        func_attach_main_panel_to_window,\
                        func_detach_main_panel_from_window,\
                        func_attach_window_to_window,\
                        func_detach_window_from_window,\
                        func_window_launch,\
                        func_window_hide,\
                        func_window_launch_modal,\
                        func_window_stop_modal,\
                        func_window_get_origin_in_screen_coordinates,\
                        func_window_set_origin_in_screen_coordinates,\
                        func_window_get_size,\
                        func_window_set_size,\
	                    func_window_set_default_pushbutton,\
                        func_window_set_cursor,\
                        func_window_set_property,\
                        window_type, panel_type, button_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_window_create, window_type, window_flag_t),\
        FUNC_CHECK_WINDOW_MANAGED(func_window_managed, window_type),\
        FUNC_CHECK_DESTROY(func_window_destroy, window_type),\
        FUNC_CHECK_SET_LISTENER(func_window_OnMoved, window_type),\
        FUNC_CHECK_SET_LISTENER(func_window_OnResize, window_type),\
        FUNC_CHECK_SET_LISTENER(func_window_OnClose, window_type),\
        FUNC_CHECK_SET_TEXT(func_window_set_title, window_type),\
        FUNC_CHECK_SET_BOOL(func_window_set_edited, window_type),\
        FUNC_CHECK_SET_BOOL(func_window_set_movable, window_type),\
        FUNC_CHECK_WINDOW_Z_ORDER(func_window_set_z_order, window_type),\
        FUNC_CHECK_SET_REAL32(func_window_set_alpha, window_type),\
        FUNC_CHECK_SET_BOOL(func_window_enable_mouse_events, window_type),\
        FUNC_CHECK_SET_PTR(func_window_set_taborder, window_type, OSControl),\
        FUNC_CHECK_SET_PTR(func_window_set_focus, window_type, OSControl),\
        FUNC_CHECK_SET_PTR(func_attach_main_panel_to_window, window_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_detach_main_panel_from_window, window_type, panel_type),\
        FUNC_CHECK_SET_PTR(func_attach_window_to_window, window_type, window_type),\
        FUNC_CHECK_SET_PTR(func_detach_window_from_window, window_type, window_type),\
        FUNC_CHECK_SET_PTR(func_window_launch, window_type, window_type),\
        FUNC_CHECK_SET_PTR(func_window_hide, window_type, window_type),\
        FUNC_CHECK_WINDOW_MODAL(func_window_launch_modal, window_type),\
        FUNC_CHECK_SET_UINT32(func_window_stop_modal, window_type),\
        FUNC_CHECK_GET2_REAL32(func_window_get_origin_in_screen_coordinates, window_type),\
        FUNC_CHECK_SET2_REAL32(func_window_set_origin_in_screen_coordinates, window_type),\
        FUNC_CHECK_GET2_REAL32(func_window_get_size, window_type),\
        FUNC_CHECK_SET2_REAL32(func_window_set_size, window_type),\
        FUNC_CHECK_SET_PTR(func_window_set_default_pushbutton, window_type, button_type),\
        FUNC_CHECK_SET_PTR(func_window_set_cursor, window_type, Cursor),\
        FUNC_CHECK_SET_PROPERTY(func_window_set_property, window_type),\
        _gui_context_append_window_manager(\
                        context,\
                        (FPtr_create_enum)func_window_create,\
                        (FPtr_window_managed)func_window_managed,\
                        (FPtr_destroy)func_window_destroy,\
                        (FPtr_set_listener)func_window_OnMoved,\
                        (FPtr_set_listener)func_window_OnResize,\
                        (FPtr_set_listener)func_window_OnClose,\
                        (FPtr_set_text)func_window_set_title,\
                        (FPtr_set_bool)func_window_set_edited,\
                        (FPtr_set_bool)func_window_set_movable,\
                        (FPtr_window_z_order)func_window_set_z_order,\
                        (FPtr_set_real32)func_window_set_alpha,\
                        (FPtr_set_bool)func_window_enable_mouse_events,\
                        (FPtr_set_ptr)func_window_set_taborder,\
                        (FPtr_set_ptr)func_window_set_focus,\
                        (FPtr_set_ptr)func_attach_main_panel_to_window,\
                        (FPtr_set_ptr)func_detach_main_panel_from_window,\
                        (FPtr_set_ptr)func_attach_window_to_window,\
                        (FPtr_set_ptr)func_detach_window_from_window,\
                        (FPtr_set_ptr)func_window_launch,\
                        (FPtr_set_ptr)func_window_hide,\
                        (FPtr_window_modal)func_window_launch_modal,\
                        (FPtr_set_uint32)func_window_stop_modal,\
                        (FPtr_get2_real32)func_window_get_origin_in_screen_coordinates,\
                        (FPtr_set2_real32)func_window_set_origin_in_screen_coordinates,\
                        (FPtr_get2_real32)func_window_get_size,\
                        (FPtr_set2_real32)func_window_set_size,\
                        (FPtr_set_ptr)func_window_set_default_pushbutton,\
                        (FPtr_set_ptr)func_window_set_cursor,\
                        (FPtr_set_property)func_window_set_property)\
    )

void _gui_context_append_menu_manager(
                        GuiContext *context,
                        FPtr_create_enum func_menu_create,
                        FPtr_destroy func_menu_destroy,
                        FPtr_set_ptr func_attach_menuitem_to_menu,
                        FPtr_set_ptr func_detach_menuitem_from_menu,
                        FPtr_menu_launch func_menu_launch_popup,
                        FPtr_call func_menu_hide_popup);
#define gui_context_append_menu_manager(\
                        context,\
                        func_menu_create,\
                        func_menu_destroy,\
                        func_attach_menuitem_to_menu,\
                        func_detach_menuitem_from_menu,\
                        func_menu_launch_popup,\
                        func_menu_hide_popup,\
                        menu_type, menuitem_type, window_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_menu_create, menu_type, enum_t),\
        FUNC_CHECK_DESTROY(func_menu_destroy, menu_type),\
        FUNC_CHECK_SET_PTR(func_attach_menuitem_to_menu, menu_type, menuitem_type),\
        FUNC_CHECK_SET_PTR(func_detach_menuitem_from_menu, menu_type, menuitem_type),\
        FUNC_CHECK_MENU_LAUNCH(func_menu_launch_popup, menu_type, window_type),\
        FUNC_CHECK_CALL(func_menu_hide_popup, menu_type),\
        _gui_context_append_menu_manager(\
                        context,\
                        (FPtr_create_enum)func_menu_create,\
                        (FPtr_destroy)func_menu_destroy,\
                        (FPtr_set_ptr)func_attach_menuitem_to_menu,\
                        (FPtr_set_ptr)func_detach_menuitem_from_menu,\
                        (FPtr_menu_launch)func_menu_launch_popup,\
                        (FPtr_call)func_menu_hide_popup)\
    )

void _gui_context_append_menuitem_manager(
                        GuiContext *context,
                        FPtr_create_enum func_menuitem_create,
                        FPtr_destroy func_menuitem_destroy,
                        FPtr_set_listener func_menuitem_OnClick,
                        FPtr_set_bool func_menuitem_set_enabled,
                        FPtr_set_bool func_menuitem_set_visible,
                        FPtr_set_text func_menuitem_set_text,
                        FPtr_set_const_ptr func_menuitem_set_image,
                        FPtr_set_key func_menuitem_set_key_equivalent,
                        FPtr_set_enum func_menuitem_set_state,
                        FPtr_set_ptr func_attach_menu_to_menu_item,
                        FPtr_set_ptr func_detach_menu_from_menu_item);
#define gui_context_append_menuitem_manager(\
                        context,\
                        func_menuitem_create,\
                        func_menuitem_destroy,\
                        func_menuitem_OnClick,\
                        func_menuitem_set_enabled,\
                        func_menuitem_set_visible,\
                        func_menuitem_set_text,\
                        func_menuitem_set_image,\
                        func_menuitem_set_key_equivalent,\
                        func_menuitem_set_state,\
                        func_attach_menu_to_menu_item,\
                        func_detach_menu_from_menu_item,\
                        menuitem_type, menu_type, image_type)\
    (\
        FUNC_CHECK_CREATE_ENUM(func_menuitem_create, menuitem_type, menu_flag_t),\
        FUNC_CHECK_DESTROY(func_menuitem_destroy, menuitem_type),\
        FUNC_CHECK_SET_LISTENER(func_menuitem_OnClick, menuitem_type),\
        FUNC_CHECK_SET_BOOL(func_menuitem_set_enabled, menuitem_type),\
        FUNC_CHECK_SET_BOOL(func_menuitem_set_visible, menuitem_type),\
        FUNC_CHECK_SET_TEXT(func_menuitem_set_text, menuitem_type),\
        FUNC_CHECK_SET_CONST_PTR(func_menuitem_set_image, menuitem_type, image_type),\
        FUNC_CHECK_SET_KEY(func_menuitem_set_key_equivalent, menuitem_type),\
        FUNC_CHECK_SET_ENUM(func_menuitem_set_state, menuitem_type, state_t),\
        FUNC_CHECK_SET_PTR(func_attach_menu_to_menu_item, menuitem_type, menu_type),\
        FUNC_CHECK_SET_PTR(func_detach_menu_from_menu_item, menuitem_type, menu_type),\
        _gui_context_append_menuitem_manager(\
                        context,\
                        (FPtr_create_enum)func_menuitem_create,\
                        (FPtr_destroy)func_menuitem_destroy,\
                        (FPtr_set_listener)func_menuitem_OnClick,\
                        (FPtr_set_bool)func_menuitem_set_enabled,\
                        (FPtr_set_bool)func_menuitem_set_visible,\
                        (FPtr_set_text)func_menuitem_set_text,\
                        (FPtr_set_const_ptr)func_menuitem_set_image,\
                        (FPtr_set_key)func_menuitem_set_key_equivalent,\
                        (FPtr_set_enum)func_menuitem_set_state,\
                        (FPtr_set_ptr)func_attach_menu_to_menu_item,\
                        (FPtr_set_ptr)func_detach_menu_from_menu_item)\
    )

void _gui_context_append_comwin_manager(
                        GuiContext *context,
                        FPtr_comwin_file func_comwin_file,
                        FPtr_comwin_color func_comwin_color);
#define gui_context_append_comwin_manager(\
                        context,\
                        func_comwin_file,\
                        func_comwin_color,\
                        window_type)\
    (\
        FUNC_CHECK_COMWIN_FILE(func_comwin_file, window_type),\
        FUNC_CHECK_COMWIN_COLOR(func_comwin_color, window_type),\
        _gui_context_append_comwin_manager(\
                        context,\
                        (FPtr_comwin_file)func_comwin_file,\
                        (FPtr_comwin_color)func_comwin_color)\
    )

void _gui_context_append_globals_manager(
                        GuiContext *context,
                        FPtr_get_enum func_globals_device,
                        FPtr_get_enum func_globals_color,    
                        FPtr_get2_real32 func_globals_resolution,
                        FPtr_get2_real32 func_globals_mouse_position,
                        FPtr_cursor func_globals_cursor,
                        FPtr_destroy func_globals_cursor_destroy,
                        FPtr_indexed func_globals_value);
#define gui_context_append_globals_manager(\
                        context,\
                        func_globals_device,\
                        func_globals_color,\
                        func_globals_resolution,\
                        func_globals_mouse_position,\
                        func_globals_cursor,\
                        func_globals_cursor_destroy,\
                        func_globals_value,\
                        image_type)\
    (\
        FUNC_CHECK_GET_ENUM(func_globals_device, void, device_t),\
        FUNC_CHECK_GET_ENUM(func_globals_color, syscolor_t, color_t),\
        FUNC_CHECK_GET2_REAL32(func_globals_resolution, void),\
        FUNC_CHECK_GET2_REAL32(func_globals_mouse_position, void),\
        FUNC_CHECK_DESTROY(func_globals_cursor_destroy, Cursor),\
        FUNC_CHECK_INDEXED(func_globals_value),\
        _gui_context_append_globals_manager(\
                        context,\
                        (FPtr_get_enum)func_globals_device,\
                        (FPtr_get_enum)func_globals_color,\
                        (FPtr_get2_real32)func_globals_resolution,\
                        (FPtr_get2_real32)func_globals_mouse_position,\
                        (FPtr_cursor)func_globals_cursor,\
                        (FPtr_destroy)func_globals_cursor_destroy,\
                        (FPtr_indexed)func_globals_value)\
    )

void _gui_context_append_drawctrl_manager(
                        GuiContext *context,
                        FPtr_get_ptr func_drawctrl_font,
                        FPtr_get_uint32 func_drawctrl_row_padding,
                        FPtr_get_uint32 func_drawctrl_check_width,
                        FPtr_get_uint32 func_drawctrl_check_height,
                        FPtr_get_enum2 func_drawctrl_multisel,
                        FPtr_call func_drawctrl_clear,
                        FPtr_rect func_drawctrl_fill,
                        FPtr_rect func_drawctrl_focus,
                        FPtr_text func_drawctrl_text,
                        FPtr_image func_drawctrl_image,
                        FPtr_rect func_drawctrl_checkbox,
                        FPtr_rect func_drawctrl_uncheckbox);
#define gui_context_append_drawctrl_manager(\
                        context,\
                        func_drawctrl_font,\
                        func_drawctrl_row_padding,\
                        func_drawctrl_check_width,\
                        func_drawctrl_check_height,\
                        func_drawctrl_multisel,\
                        func_drawctrl_clear,\
                        func_drawctrl_fill,\
                        func_drawctrl_focus,\
                        func_drawctrl_text,\
                        func_drawctrl_image,\
                        func_drawctrl_checkbox,\
                        func_drawctrl_uncheckbox,\
                        context_type)\
    (\
        FUNC_CHECK_GET_PTR(func_drawctrl_font, context_type, Font),\
        FUNC_CHECK_GET_UINT32(func_drawctrl_row_padding, context_type),\
        FUNC_CHECK_GET_UINT32(func_drawctrl_check_width, context_type),\
        FUNC_CHECK_GET_UINT32(func_drawctrl_check_height, context_type),\
        FUNC_CHECK_GET_ENUM2(func_drawctrl_multisel, context_type, multisel_t, vkey_t),\
        FUNC_CHECK_CALL(func_drawctrl_clear, context_type),\
        FUNC_CHECK_RECT(func_drawctrl_fill, context_type, cstate_t),\
        FUNC_CHECK_RECT(func_drawctrl_focus, context_type, cstate_t),\
        FUNC_CHECK_TEXT(func_drawctrl_text, context_type, cstate_t),\
        FUNC_CHECK_IMAGE(func_drawctrl_image, context_type, cstate_t),\
        FUNC_CHECK_RECT(func_drawctrl_checkbox, context_type, cstate_t),\
        FUNC_CHECK_RECT(func_drawctrl_uncheckbox, context_type, cstate_t),\
        _gui_context_append_drawctrl_manager(\
                        context,\
                        (FPtr_get_ptr)func_drawctrl_font,\
                        (FPtr_get_uint32)func_drawctrl_row_padding,\
                        (FPtr_get_uint32)func_drawctrl_check_width,\
                        (FPtr_get_uint32)func_drawctrl_check_height,\
                        (FPtr_get_enum2)func_drawctrl_multisel,\
                        (FPtr_call)func_drawctrl_clear,\
                        (FPtr_rect)func_drawctrl_fill,\
                        (FPtr_rect)func_drawctrl_focus,\
                        (FPtr_text)func_drawctrl_text,\
                        (FPtr_image)func_drawctrl_image,\
                        (FPtr_rect)func_drawctrl_checkbox,\
                        (FPtr_rect)func_drawctrl_uncheckbox)\
    )

__END_C

