/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gui.ixx
 *
 */

/* Graphics User Interface */

#ifndef __GUI_IXX__
#define __GUI_IXX__

#include "gui.hxx"
#include <core/coreh.hxx>

enum _gievent_t
{
    ekGUI_EVENT_ANIMATION
};

typedef struct _line_t Line;
typedef union _tag_t Tag;
typedef struct _gui_component_t GuiComponent;
typedef struct _vctrltbl_t VCtrlTbl;
typedef struct _editimp_t EditImp;
typedef struct _items_t Items;
typedef struct _scrollview_t ScrollView;
typedef struct _colitem_t ColItem;
typedef struct _evtransition_t EvTransition;
typedef struct _evbind_t EvBind;

typedef void (*FPtr_set_size)(void *component, const S2Df *size);
#define FUNC_CHECK_SET_SIZE(func, type) \
    (void)((void (*)(type *, const S2Df *))func == func)

typedef void (*FPtr_get_size)(void *component, S2Df *size);
#define FUNC_CHECK_GET_SIZE(func, type) \
    (void)((void (*)(type *, S2Df *))func == func)

typedef void (*FPtr_natural)(void *component, const uint32_t di, real32_t *dim0, real32_t *dim1);
#define FUNC_CHECK_DIMENSION(func, type) \
    (void)((void (*)(type *, const uint32_t, real32_t *, real32_t *))func == func)

typedef void (*FPtr_expand)(void *component, const uint32_t di, const real32_t current_size, const real32_t required_size, real32_t *final_size);
#define FUNC_CHECK_EXPAND(func, type) \
    (void)((void (*)(type *, const uint32_t, const real32_t, const real32_t, real32_t *))func == func)

typedef void (*FPtr_panels)(const void *component, uint32_t *num_panels, Panel **panels);
#define FUNC_CHECK_PANELS(func, type) \
    (void)((void (*)(const type *, uint32_t *, Panel **))func == func)

typedef void (*FPtr_set_image)(void *item, const Image *image);

union _tag_t
{
    uint32_t tag_uint32;
};

struct _gui_component_t
{
    Object object;
    GuiCtx *context;
    gui_type_t type;
    Panel *panel;
    Tag tag;
    void *ositem;
};

struct _vctrltbl_t
{
    const char_t *type;
    FPtr_event_handler OnDraw;
    FPtr_event_handler OnOverlay;
    FPtr_event_handler OnResize;
    FPtr_event_handler OnEnter;
    FPtr_event_handler OnExit;
    FPtr_event_handler OnMoved;
    FPtr_event_handler OnDown;
    FPtr_event_handler OnUp;
    FPtr_event_handler OnClick;
    FPtr_event_handler OnDrag;
    FPtr_event_handler OnWheel;
    FPtr_event_handler OnKeyDown;
    FPtr_event_handler OnKeyUp;
    FPtr_event_handler OnFocus;
    FPtr_event_handler OnResignFocus;
    FPtr_event_handler OnAcceptFocus;
    FPtr_event_handler OnScroll;
    FPtr_destroy func_destroy_data;
    FPtr_gctx_call func_locale;
    FPtr_natural func_natural;
    FPtr_gctx_call func_empty;
    FPtr_gctx_set_uint32 func_uint32;
    FPtr_set_image func_image;
};

struct _colitem_t
{
    ctrl_op_t op;
    uint32_t index;
    const char_t *text;
    align_t align;
    real32_t width;
};

struct _evtransition_t
{
    real64_t prtime;
    real64_t crtime;
    uint32_t cframe;
    uint32_t empty;
};

struct _evbind_t
{
    void *obj_main;
    void *obj_edit;
    const char_t *objtype_main;
    const char_t *objtype_edit;
    uint16_t offset_edit;
    uint16_t size_main;
    uint16_t size_edit;
};

DeclPt(Cell);
DeclPt(Layout);
DeclPt(GuiComponent);
DeclPt(MenuItem);

#endif
