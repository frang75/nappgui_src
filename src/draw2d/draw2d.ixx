/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw2d.ixx
 *
 */

/* Operating system 2D drawing support */

#ifndef __OSDRAW_IXX__
#define __OSDRAW_IXX__

#include "draw2d.hxx"
#include "draw2dh.ixx"

typedef struct _measurestr_t MeasureStr;
typedef struct _ccursor_t Cursor;

#define label_type(flags)       ((flags) & ekLBTYPE)
#define button_type(flags)      ((flags) & ekBTTYPE)
#define edit_type(flags)        ((flags) & ekEDTYPE)
#define slider_type(flags)      ((flags) & ekSLTYPE)
#define progress_type(flags)    ((flags) & ekPGTYPE)
#define split_type(flags)		((flags) & ekSPTYPE)

enum font_family_t
{
    ekFONT_FAMILY_SYSTEM            = 0,
    ekFONT_FAMILY_MONOSPACE         = 1
};

typedef enum _guitype_t
{
	/* Basic controls */
    ekGUI_COMPONENT_LABEL           = 0,
    ekGUI_COMPONENT_BUTTON          = 1,
    ekGUI_COMPONENT_POPUP           = 2,
    ekGUI_COMPONENT_EDITBOX         = 3,
    ekGUI_COMPONENT_COMBOBOX        = 4,
    ekGUI_COMPONENT_SLIDER          = 5,
    ekGUI_COMPONENT_UPDOWN          = 6,
    ekGUI_COMPONENT_PROGRESS        = 7,

	/* View Controls */
    ekGUI_COMPONENT_TEXTVIEW        = 8,
    ekGUI_COMPONENT_TABLEVIEW       = 9,
    ekGUI_COMPONENT_TREEVIEW        = 10,
	ekGUI_COMPONENT_BOXVIEW         = 11,
    ekGUI_COMPONENT_SPLITVIEW       = 12,
    ekGUI_COMPONENT_CUSTOMVIEW      = 13,

    /* Others */
    ekGUI_COMPONENT_PANEL           = 14,
    ekGUI_COMPONENT_LINE            = 15,
    ekGUI_COMPONENT_HEADER          = 16,

    /* Non-Components */
    ekGUI_COMPONENT_WINDOW          = 17,
    ekGUI_COMPONENT_TOOLBAR         = 18
} guitype_t;

#define GUI_CONTEXT_NUM_COMPONENTS  17

typedef enum _gui_role_t
{
    ekGUI_ROLE_MAIN             = 0,
    ekGUI_ROLE_OVERLAY          = 1,
    ekGUI_ROLE_MODAL            = 2,
    ekGUI_ROLE_MANAGED          = 3
} gui_role_t;

typedef enum _cstate_t
{
    ekCSTATE_NORMAL,
    ekCSTATE_HOT,
    ekCSTATE_PRESSED,
    ekCSTATE_BKNORMAL,
    ekCSTATE_BKHOT,
    ekCSTATE_BKPRESSED,
    ekCSTATE_DISABLED
} cstate_t;

typedef enum _multisel_t
{
    ekMULTISEL_NO,
    ekMULTISEL_SINGLE,
    ekMULTISEL_BURST
} multisel_t;

typedef struct _osfont_t OSFont;
typedef struct _osimage_t OSImage;
typedef struct _higram_t Higram;
typedef struct _btext_t BText;

typedef void(*FPtr_word_extents)(void *data, const char_t *word, real32_t *width, real32_t *height);
#define FUNC_CHECK_WORD_EXTENTS(func, type)\
    (void)((void(*)(type*, const char_t*, real32_t*, real32_t*))func == func)

typedef void(*FPtr_call)(void *item);
#define FUNC_CHECK_CALL(func, type)\
    (void)((void(*)(type*))func == func)

typedef void*(*FPtr_create)(const uint32_t flags);
#define FUNC_CHECK_CREATE(func, type)\
    (void)((type*(*)(const uint32_t))func == func)

typedef void*(*FPtr_create_enum)(const enum_t value);
#define FUNC_CHECK_CREATE_ENUM(func, type, enum_type)\
    (void)((type*(*)(const enum_type))func == func)

typedef void(*FPtr_set_bool)(void *item, const bool_t value);
#define FUNC_CHECK_SET_BOOL(func, type)\
    (void)((void(*)(type*, const bool_t))func == func)

typedef void(*FPtr_set_uint32)(void *item, const uint32_t value);
#define FUNC_CHECK_SET_UINT32(func, type)\
    (void)((void(*)(type*, const uint32_t))func == func)

typedef void(*FPtr_set4_uint32)(void *item, const uint32_t value1, const uint32_t value2, const uint32_t value3, const uint32_t value4);
#define FUNC_CHECK_SET4_UINT32(func, type)\
    (void)((void(*)(type*, const uint32_t, const uint32_t, const uint32_t, const uint32_t))func == func)

typedef uint32_t(*FPtr_get_uint32)(const void *item);
#define FUNC_CHECK_GET_UINT32(func, type)\
    (void)((uint32_t(*)(const type*))func == func)

typedef enum_t(*FPtr_get_enum)(const void *item);
#define FUNC_CHECK_GET_ENUM(func, type, type_enum)\
    (void)((type_enum(*)(const type*))func == func)

typedef enum_t(*FPtr_get_enum2)(const void *item, const enum_t);
#define FUNC_CHECK_GET_ENUM2(func, type, type_enum, type_enum2)\
    (void)((type_enum(*)(const type*, const type_enum2))func == func)

typedef void(*FPtr_set_enum)(void *item, const enum_t value);
#define FUNC_CHECK_SET_ENUM(func, type, type_enum)\
    (void)((void(*)(type*, const type_enum))func == func)

typedef void(*FPtr_set_real32)(void *item, const real32_t value);
#define FUNC_CHECK_SET_REAL32(func, type)\
    (void)((void(*)(type*, const real32_t))func == func)

typedef void(*FPtr_set2_real32)(void *item, const real32_t value1, const real32_t value2);
#define FUNC_CHECK_SET2_REAL32(func, type)\
    (void)((void(*)(type*, const real32_t, const real32_t))func == func)

typedef void(*FPtr_set4_real32)(void *item, const real32_t value1, const real32_t value2, const real32_t value3, const real32_t value4);
#define FUNC_CHECK_SET4_REAL32(func, type)\
    (void)((void(*)(type*, const real32_t, const real32_t, const real32_t, const real32_t))func == func)

typedef real32_t(*FPtr_get_real32)(const void *item);
#define FUNC_CHECK_GET_REAL32(func, type)\
    (void)((real32_t(*)(const type*))func == func)

typedef void(*FPtr_get2_real32)(const void *item, real32_t *value1, real32_t *value2);
#define FUNC_CHECK_GET2_REAL32(func, type)\
    (void)((void(*)(const type*, real32_t*, real32_t*))func == func)

typedef void(*FPtr_set_text)(void *item, const char_t *text);
#define FUNC_CHECK_SET_TEXT(func, type)\
    (void)((void(*)(type*, const char_t*))func == func)

typedef const char_t*(*FPtr_get_text)(const void *item);
#define FUNC_CHECK_GET_TEXT(func, type)\
    (void)((const char_t*(*)(const type*))func == func)

typedef const void*(*FPtr_get_const_ptr)(const void *item);
#define FUNC_CHECK_GET_CONST_PTR(func, type, ptr_type)\
    (void)((const ptr_type*(*)(const type*))func == func)

typedef void*(*FPtr_get_ptr)(const void *item);
#define FUNC_CHECK_GET_PTR(func, type, ptr_type)\
    (void)((ptr_type*(*)(const type*))func == func)

typedef void(*FPtr_set_ptr)(void *item, void *ptr);
#define FUNC_CHECK_SET_PTR(func, type, ptr_type)\
    (void)((void(*)(type*, ptr_type*))func == func)

typedef void(*FPtr_set_const_ptr)(void *item, const void *ptr);
#define FUNC_CHECK_SET_CONST_PTR(func, type, ptr_type)\
    (void)((void(*)(type*, const ptr_type*))func == func)

typedef void(*FPtr_set_listener)(void *item, Listener *listener);
#define FUNC_CHECK_SET_LISTENER(func, type)\
    (void)((void(*)(type*, Listener*))func == func)

typedef void(*FPtr_set_elem)(void *item, const op_t op, const uint32_t index, const char_t *text, const Image *data);
#define FUNC_CHECK_SET_ELEM(func, type)\
    (void)((void(*)(type*, const op_t, const uint32_t, const char_t*, const Image*))func == func)

typedef void(*FPtr_bounds1)(const void *item, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height);
#define FUNC_CHECK_BOUNDS1(func, type)\
    (void)((void(*)(const type*, const char_t*, const real32_t, real32_t*, real32_t*))func == func)

typedef void(*FPtr_bounds2)(const void *item, const char_t *text, const real32_t refwidth, const real32_t refheight, real32_t *width, real32_t *height);
#define FUNC_CHECK_BOUNDS2(func, type)\
    (void)((void(*)(const type*, const char_t*, const real32_t, const real32_t, real32_t*, real32_t*))func == func)

typedef void(*FPtr_bounds3)(const void *item, const char_t *text, real32_t *width, real32_t *height);
#define FUNC_CHECK_BOUNDS3(func, type)\
    (void)((void(*)(const type*, const char_t*, real32_t*, real32_t*))func == func)

typedef void(*FPtr_bounds4)(const void *item, const real32_t refwidth, const uint32_t lines, real32_t *width, real32_t *height);
#define FUNC_CHECK_BOUNDS4(func, type)\
    (void)((void(*)(const type*, const real32_t, const uint32_t, real32_t*, real32_t*))func == func)

typedef void(*FPtr_bounds5)(const void *item, const real32_t refwidth, real32_t *width, real32_t *height);
#define FUNC_CHECK_BOUNDS5(func, type)\
    (void)((void(*)(const type*, const real32_t, real32_t*, real32_t*))func == func)

typedef void(*FPtr_bounds6)(const void *item, const real32_t length, const fsize_t size, real32_t *width, real32_t *height);
#define FUNC_CHECK_BOUNDS6(func, type)\
    (void)((void(*)(const type*, const real32_t, const fsize_t , real32_t*, real32_t*))func == func)

typedef void(*FPtr_area)(void *item, void *obj, const color_t bgcolor, const color_t skcolor, const real32_t x, const real32_t y, const real32_t width, const real32_t height);
#define FUNC_CHECK_AREA(func, type)\
    (void)((void(*)(type*, void*, const color_t, const color_t, const real32_t, const real32_t, const real32_t, const real32_t))func == func)

typedef void(*FPtr_bar_tickmarks)(void *item, const uint32_t num_tickmarks, const bool_t tickmarks_at_left_top);
#define FUNC_CHECK_BAR_TICKMARKS(func, type)\
    (void)((void(*)(type*, const uint32_t, const bool_t))func == func)

typedef real32_t(*FPtr_get_real32_enum)(const void *item, const enum_t value);
#define FUNC_CHECK_GET_REAL32_ENUM(func, type, type_enum)\
    (void)((real32_t(*)(const type*, const type_enum))func == func)

typedef void*(*FPtr_window_managed)(void*);
#define FUNC_CHECK_WINDOW_MANAGED(func, type)\
    (void)((type*(*)(void*))func == func)

typedef void(*FPtr_window_z_order)(void*, void*);
#define FUNC_CHECK_WINDOW_Z_ORDER(func, type)\
    (void)((void(*)(type*, type*))func == func)

typedef uint32_t(*FPtr_window_modal)(void*, void*);
#define FUNC_CHECK_WINDOW_MODAL(func, type)\
    (void)((uint32_t(*)(type*, type*))func == func)

typedef void(*FPtr_set_property)(void *item, const guiprop_t property, const void *value);
#define FUNC_CHECK_SET_PROPERTY(func, type)\
    (void)((void(*)(type*, const guiprop_t, const void*))func == func)

typedef void(*FPtr_menu_launch)(void *item, void *window, const real32_t x, const real32_t y);
#define FUNC_CHECK_MENU_LAUNCH(func, type, window_type)\
    (void)((void(*)(type*, window_type*, const real32_t, const real32_t))func == func)

typedef void(*FPtr_set_key)(void *item, const uint32_t key, const uint32_t modifiers);
#define FUNC_CHECK_SET_KEY(func, type)\
    (void)((void(*)(type*, const uint32_t, const uint32_t))func == func)

typedef const char_t*(*FPtr_comwin_file)(void *parent, const char_t **ftypes, const uint32_t size, const char_t *start_dir, const bool_t open);
#define FUNC_CHECK_COMWIN_FILE(func, type)\
    (void)((const char_t*(*)(type*, const char_t**, const uint32_t, const char_t*, const bool_t))func == func)

typedef void(*FPtr_comwin_color)(void *parent, const char_t *title, const real32_t x, const real32_t y, const align_t halign, const align_t valign, color_t current, color_t *colors, const uint32_t n, Listener *OnChange);
#define FUNC_CHECK_COMWIN_COLOR(func, type)\
    (void)((void(*)(type*, const char_t*, const real32_t, const real32_t, const align_t, const align_t, const color_t, color_t*, const uint32_t, Listener*))func == func)

typedef Cursor*(*FPtr_cursor)(const enum_t, const Image*, const real32_t, const real32_t);

typedef void(*FPtr_rect)(void*, const uint32_t, const uint32_t, const uint32_t, const uint32_t, const enum_t);
#define FUNC_CHECK_RECT(func, type, enum_type)\
    (void)((void(*)(type*, const uint32_t, const uint32_t, const uint32_t, const uint32_t, const enum_type))func == func)

typedef void(*FPtr_text)(void*, const char_t*, const uint32_t, const uint32_t, const enum_t);
#define FUNC_CHECK_TEXT(func, type, enum_type)\
    (void)((void(*)(type*, const char_t*, const uint32_t, const uint32_t, const enum_type))func == func)

typedef void(*FPtr_image)(void*, const Image*, const uint32_t, const uint32_t, const enum_t);
#define FUNC_CHECK_IMAGE(func, type, enum_type)\
    (void)((void(*)(type*, const Image*, const uint32_t, const uint32_t, const enum_type))func == func)

typedef void(*FPtr_indexed)(const uint32_t, void*);
#define FUNC_CHECK_INDEXED(func)\
    (void)((void(*)(const uint32_t, void*))func == func)

struct _gui_context_t
{
    uint32_t retain_count;

    /*! <Commons> */
    FPtr_set_text func_set_tooltip[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_destroy func_destroy[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_set_ptr func_attach_to_panel[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_set_ptr func_detach_from_panel[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_set_bool func_set_visible[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_set_bool func_set_enabled[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_get2_real32 func_get_size[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_get2_real32 func_get_origin[GUI_CONTEXT_NUM_COMPONENTS];
    FPtr_set4_real32 func_set_frame[GUI_CONTEXT_NUM_COMPONENTS];

    /*! <Label> */
    FPtr_create_enum func_label_create;
    FPtr_set_listener func_label_OnClick;
    FPtr_set_listener func_label_OnMouseEnter;
    FPtr_set_listener func_label_OnMouseExit;
    FPtr_set_text func_label_set_text;
    FPtr_set_const_ptr func_label_set_font;
    FPtr_set_enum func_label_set_align;
    FPtr_set_enum func_label_set_ellipsis;
    FPtr_set_uint32 func_label_set_text_color;
    FPtr_set_uint32 func_label_set_bg_color;
    FPtr_bounds1 func_label_bounds;

    /*! <Button> */
    FPtr_create_enum func_button_create;
    FPtr_set_listener func_button_OnClick;
    FPtr_set_text func_button_set_text;
    FPtr_set_const_ptr func_button_set_font;
    FPtr_set_enum func_button_set_align;
    FPtr_set_const_ptr func_button_set_image;
    FPtr_set_enum func_button_set_state;
    FPtr_get_enum func_button_get_state;
    FPtr_bounds2 func_button_bounds;

    /*! <PopUp> */
    FPtr_create_enum func_popup_create;
    FPtr_set_listener func_popup_OnChange;
    FPtr_set_elem func_popup_set_elem;
    FPtr_set_const_ptr func_popup_set_font;
    FPtr_set_uint32 func_popup_list_height;
    FPtr_set_uint32 func_popup_set_selected;
    FPtr_get_uint32 func_popup_get_selected;
    FPtr_bounds3 func_popup_bounds;

    /*! <Edit> */
    FPtr_create_enum func_edit_create;
    FPtr_set_listener func_edit_OnFilter;
    FPtr_set_listener func_edit_OnChange;
    FPtr_set_listener func_edit_OnFocus;
    FPtr_set_text func_edit_set_text;
    FPtr_set_const_ptr func_edit_set_font;
    FPtr_set_enum func_edit_set_align;
    FPtr_set_bool func_edit_set_passmode;
    FPtr_set_bool func_edit_set_editable;
    FPtr_set_bool func_edit_set_autoselect;
    FPtr_set_uint32 func_edit_set_text_color;
    FPtr_set_uint32 func_edit_set_bg_color;
    FPtr_bounds4 func_edit_bounds;

    /*! <Combo> */
    FPtr_create_enum func_combo_create;
    FPtr_set_listener func_combo_OnFilter;
    FPtr_set_listener func_combo_OnChange;
    FPtr_set_listener func_combo_OnFocus;
    FPtr_set_listener func_combo_OnSelect;
    FPtr_set_text func_combo_set_text;
    FPtr_set_const_ptr func_combo_set_font;
    FPtr_set_enum func_combo_set_align;
    FPtr_set_bool func_combo_set_passmode;
    FPtr_set_uint32 func_combo_set_text_color;
    FPtr_set_uint32 func_combo_set_bg_color;
    FPtr_set_elem func_combo_set_elem;
    FPtr_set_uint32 func_combo_set_selected;
    FPtr_get_uint32 func_combo_get_selected;
    FPtr_bounds5 func_combo_bounds;

    /*! <Slider> */
    FPtr_create_enum func_slider_create;
    FPtr_set_listener func_slider_OnMoved;
    FPtr_bar_tickmarks func_slider_set_tickmarks;
    FPtr_get_real32 func_slider_get_position;
    FPtr_set_real32 func_slider_set_position;
    FPtr_bounds6 func_slider_bounds;

    /* UpDown */
	FPtr_create_enum func_updown_create;
    FPtr_set_listener func_updown_OnClick;

    /*! <Progress> */
    FPtr_create_enum func_progress_create;
    FPtr_set_real32 func_progress_set_position;
    FPtr_get_real32_enum func_progress_get_thickness;

    /*! <Text view> */
    FPtr_create_enum func_text_create;
    FPtr_set_listener func_text_OnTextChange;
    FPtr_set_text func_text_insert_text;
    FPtr_set_text func_text_set_text;
    FPtr_set_ptr func_text_set_rtf;
    FPtr_set_property func_text_set_prop;
    FPtr_set_bool func_text_set_editable;
    FPtr_get_text func_text_get_text;
    FPtr_call func_text_set_need_display;

    /*! <Split view> */
    FPtr_create_enum func_split_create;
    FPtr_set_ptr func_split_attach_control;
    FPtr_set_ptr func_split_detach_control;
    FPtr_set_listener func_split_OnDrag;
    FPtr_set4_real32 func_split_track_area;

    /*! <View> */
    FPtr_create func_view_create;
    FPtr_set_listener func_view_OnDraw;
    FPtr_set_listener func_view_OnEnter;
    FPtr_set_listener func_view_OnExit;
    FPtr_set_listener func_view_OnMoved;
    FPtr_set_listener func_view_OnDown;
    FPtr_set_listener func_view_OnUp;
    FPtr_set_listener func_view_OnClick;
    FPtr_set_listener func_view_OnDrag;
    FPtr_set_listener func_view_OnWheel;
    FPtr_set_listener func_view_OnKeyDown;
    FPtr_set_listener func_view_OnKeyUp;
    FPtr_set_listener func_view_OnFocus;
    FPtr_set_listener func_view_OnNotify;
    FPtr_set_listener func_view_OnTouchTap;
    FPtr_set_listener func_view_OnTouchStartDrag;
    FPtr_set_listener func_view_OnTouchDragging;
    FPtr_set_listener func_view_OnTouchEndDrag;
    FPtr_set_listener func_view_OnTouchStartPinch;
    FPtr_set_listener func_view_OnTouchPinching;
    FPtr_set_listener func_view_OnTouchEndPinch;
    FPtr_set2_real32 func_view_scroll;
    FPtr_get2_real32 func_view_scroll_get;
    FPtr_get2_real32 func_view_scroller_size;
    FPtr_set4_real32 func_view_content_size;
    FPtr_get_real32 func_view_scale_factor;
    FPtr_call func_view_set_need_display;
    FPtr_set_bool func_view_set_drawable;
    FPtr_get_ptr func_view_get_native_view;
    
	/*! <Panels> */
    FPtr_create func_panel_create;
    FPtr_area func_panel_area;
    FPtr_get2_real32 func_panel_scroller_size;
    FPtr_set4_real32 func_panel_content_size;
    FPtr_call func_panel_set_need_display;

    /*! <Menus> */
    FPtr_create_enum func_menu_create;
    FPtr_destroy func_menu_destroy;
    FPtr_set_ptr func_attach_menuitem_to_menu;
    FPtr_set_ptr func_detach_menuitem_from_menu;
    FPtr_menu_launch func_menu_launch_popup;
    FPtr_call func_menu_hide_popup;

    /*! <MenuItems> */
    FPtr_create_enum func_menuitem_create;
    FPtr_destroy func_menuitem_destroy;
    FPtr_set_listener func_menuitem_OnClick;
    FPtr_set_bool func_menuitem_set_enabled;
    FPtr_set_bool func_menuitem_set_visible;
    FPtr_set_text func_menuitem_set_text;
    FPtr_set_const_ptr func_menuitem_set_image;
    FPtr_set_key func_menuitem_set_key_equivalent;
    FPtr_set_enum func_menuitem_set_state;
    FPtr_set_ptr func_attach_menu_to_menu_item;
    FPtr_set_ptr func_detach_menu_from_menu_item;

    /*! <Windows> */
    FPtr_create_enum func_window_create;
    FPtr_window_managed func_window_managed;
    FPtr_destroy func_window_destroy;
    FPtr_set_listener func_window_OnMoved;
    FPtr_set_listener func_window_OnResize;
    FPtr_set_listener func_window_OnClose;
    FPtr_set_text func_window_set_title;
    FPtr_set_bool func_window_set_edited;
    FPtr_set_bool func_window_set_movable;
    FPtr_window_z_order func_window_set_z_order;
    FPtr_set_real32 func_window_set_alpha;
    FPtr_set_bool func_window_enable_mouse_events;
    FPtr_set_ptr func_window_set_taborder;
    FPtr_set_ptr func_window_set_focus;
    FPtr_set_ptr func_attach_main_panel_to_window;
    FPtr_set_ptr func_detach_main_panel_from_window;
    FPtr_set_ptr func_attach_window_to_window;
    FPtr_set_ptr func_detach_window_from_window;
    FPtr_set_ptr func_window_launch;
    FPtr_set_ptr func_window_hide;
    FPtr_window_modal func_window_launch_modal;
    FPtr_set_uint32 func_window_stop_modal;
    FPtr_get2_real32 func_window_get_origin_in_screen_coordinates;
    FPtr_set2_real32 func_window_set_origin_in_screen_coordinates;
    FPtr_get2_real32 func_window_get_size;
    FPtr_set2_real32 func_window_set_size;
	FPtr_set_ptr func_window_set_default_pushbutton;
    FPtr_set_ptr func_window_set_cursor;
	FPtr_set_property func_window_set_property;

    /*! <Common Windows> */
    FPtr_comwin_file func_comwin_file;
    FPtr_comwin_color func_comwin_color;

    /*! <Globals> */
    FPtr_get_enum func_globals_device;
    FPtr_get_enum func_globals_color;
    FPtr_get2_real32 func_globals_resolution;
    FPtr_get2_real32 func_globals_mouse_position;
    FPtr_cursor func_globals_cursor;   
    FPtr_destroy func_globals_cursor_destroy;
    FPtr_indexed func_globals_value;

    /*! <Draw Custom Controls> */
    FPtr_get_ptr func_drawctrl_font;
    FPtr_get_uint32 func_drawctrl_row_padding;
    FPtr_get_uint32 func_drawctrl_check_width;
    FPtr_get_uint32 func_drawctrl_check_height;
    FPtr_get_enum2 func_drawctrl_multisel;
    FPtr_call func_drawctrl_clear;
    FPtr_rect func_drawctrl_fill;
    FPtr_rect func_drawctrl_focus;
    FPtr_text func_drawctrl_text;
    FPtr_image func_drawctrl_image;
    FPtr_rect func_drawctrl_checkbox;
    FPtr_rect func_drawctrl_uncheckbox;
};

#endif
