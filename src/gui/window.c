/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: window.c
 *
 */

/* Windows */

#include "window.h"
#include "window.inl"
#include "gui.inl"
#include "guicontexth.inl"
#include "button.inl"
#include "component.inl"
#include "obj.inl"
#include "panel.inl"
#include "panel.h"
#include "layout.inl"
#include "layout.h"

#include "cassert.h"
#include "event.h"
#include "ptr.h"
#include "s2d.h"
#include "v2d.h"

struct _window_t
{
    Object object;
    const GuiContext *context;
    bool_t in_will_close_event;
    bool_t is_destroyed;
    bool_t visible;
    uint32_t flags;
    gui_role_t role;
    void *ositem;
    ResId titleid;
    Layout *main_layout;
    Listener *OnMoved;
    Listener *OnResize;
    Listener *OnClose;
};

/*---------------------------------------------------------------------------*/

static void i_detach_main_panel(Panel *main_panel, void *window_renderable_item, FPtr_set_ptr func_detach_main_panel_from_window)
{
    GuiComponent *panel_component;
    cassert_no_nullf(func_detach_main_panel_from_window);
    panel_component = _panel_get_component(main_panel);
    cassert_no_null(panel_component);
    func_detach_main_panel_from_window(window_renderable_item, panel_component->ositem);
    _panel_detach_components(main_panel);
    _panel_window(main_panel, NULL);
}

/*---------------------------------------------------------------------------*/

static void i_destroy(Window **window)
{
    cassert_no_null(window);
    cassert_no_null(*window);
    cassert((*window)->in_will_close_event == FALSE);
    cassert_no_null((*window)->context);
    cassert_no_nullf((*window)->context->func_window_destroy);
    
    if ((*window)->visible == TRUE)
        window_hide(*window);

    if ((*window)->role == ekGUI_ROLE_MODAL)
        window_stop_modal(*window, 0);
        
    if ((*window)->main_layout != NULL)
    {
        Panel *main_panel = layout_get_panel((*window)->main_layout, 0, 0);

        // Prevent flickering in Windows because the main panel new parent will be 
        // set to NULL (Desktop HWND) when is detached from this window.
        #if defined(__WINDOWS__)
        _panel_hide_all(main_panel);
        #endif

        i_detach_main_panel(main_panel, (*window)->ositem, (*window)->context->func_detach_main_panel_from_window);
        _layout_destroy(&(*window)->main_layout);
        _panel_destroy(&main_panel);
    }
    
    (*window)->context->func_window_destroy(&(*window)->ositem);
    listener_destroy(&(*window)->OnMoved);
    listener_destroy(&(*window)->OnResize);
    listener_destroy(&(*window)->OnClose);    
    gui_context_release((GuiContext**)(&(*window)->context));
    obj_delete(window, Window);
}

/*---------------------------------------------------------------------------*/

void window_destroy(Window **window)
{
    cassert_no_null(window);
    cassert_no_null(*window);
    cassert((*window)->is_destroyed == FALSE);
    _gui_delete_window(*window);
    if ((*window)->in_will_close_event == TRUE)
    {
        (*window)->is_destroyed = TRUE;
        *window = NULL;
    }
    else
    {
        i_destroy(window);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnWindowMoved(Window *window, Event *event)
{
    cassert_no_null(window);
    cassert_no_null(event);
    cassert(event_type(event) == ekEVWNDMOVED);
    cassert(event_sender_imp(event, NULL) == window->ositem);
    if (window->visible == TRUE && window->OnMoved != NULL)
        listener_pass_event(window->OnMoved, event, window, Window);
}

/*---------------------------------------------------------------------------*/

static void i_OnWindowResize(Window *window, Event *e)
{
    const EvSize *params = NULL;
    cassert_no_null(window);
    cassert(window->visible == TRUE);
    cassert_no_null(window->context);
    cassert(event_sender_imp(e, NULL) == window->ositem);
    params = event_params(e, EvSize);
    cassert_no_null(params);

	switch (event_type(e)) {
	case ekEVWNDSIZING:
	{
		S2Df reqsize;
		S2Df finsize;
		EvSize* result = event_result(e, EvSize);
		cassert_no_null(result);
		reqsize.width = params->width;
		reqsize.height = params->height;
		_layout_compose(window->main_layout, &reqsize, &finsize);
		result->width = finsize.width;
		result->height = finsize.height;
		break;
	}

	case ekEVWNDSIZE:
	{
		//S2Df size = s2df(params->width, params->height);
		//_component_set_frame(_panel_get_component(window->main_panel), &kV2D_ZEROf, &size);
		_layout_locate(window->main_layout);
		if (window->OnResize != NULL)
			listener_pass_event(window->OnResize, e, window, Window);
		break;
	}

	cassert_default();
	}
}

/*---------------------------------------------------------------------------*/

static void i_OnWindowClose(Window *window, Event *event)
{
    const EvWinClose *params = event_params(event, EvWinClose);
    bool_t closed = TRUE;
    cassert_no_null(window);
    cassert(window->in_will_close_event == FALSE);
    cassert(window->is_destroyed == FALSE);
    cassert_no_null(event);
    cassert(event_type(event) == ekEVWNDCLOSE);
    cassert(event_sender_imp(event, NULL) == window->ositem);
    
    window->in_will_close_event = TRUE;

    switch (params->origin) {
    case ekCLBUTTON:
        if (window->OnClose != NULL)
        {
            listener_pass_event(window->OnClose, event, window, Window);
            closed = *event_result(event, bool_t);
        }
        else
        {
            switch (window->role)
            {
                case ekGUI_ROLE_MAIN:
                    closed = FALSE;
                    break;
                case ekGUI_ROLE_OVERLAY:
                case ekGUI_ROLE_MODAL:
                    closed = TRUE;
                    break;
                case ekGUI_ROLE_MANAGED:
                    cassert(FALSE);
                    closed = FALSE;
                    break;
                cassert_default();
            }
        }

        break;

    case ekCLESC:
        if (window->flags & ekWNESC)
        {
            *event_result(event, bool_t) = TRUE;
            if (window->OnClose != NULL)
                listener_pass_event(window->OnClose, event, window, Window);
            closed = *event_result(event, bool_t);
        }
        else
        {
            closed = FALSE;
        }
        break;

    case ekCLINTRO:
        if (window->flags & ekWNRETURN)
        {
            *event_result(event, bool_t) = TRUE;
            if (window->OnClose != NULL)
                listener_pass_event(window->OnClose, event, window, Window);
            closed = *event_result(event, bool_t);
        }
        else
        {
            closed = FALSE;
        }
        break;

    case ekCLDEACT:
        closed = FALSE;
        break;

    cassert_default();
    }
    
    if (closed == TRUE)
    {
        if (window->visible == TRUE)
            window_hide(window);

        if (window->role == ekGUI_ROLE_MODAL)
            window_stop_modal(window, (uint32_t)params->origin);

        cassert(window->visible == FALSE);
        window->in_will_close_event = FALSE;
    
        if (window->is_destroyed == TRUE)
            i_destroy(&window);
    }
    else
    {
        window->in_will_close_event = FALSE;
    }

    *event_result(event, bool_t) = closed;
}

/*---------------------------------------------------------------------------*/

static Window *i_create_window(const uint32_t flags)
{
    Window *window = obj_new0(Window);
    window->context = gui_context_retain(gui_context_get_current());
    window->ositem = window->context->func_window_create((enum_t)flags);
    window->flags = flags;
    window->role = ENUM_MAX(gui_role_t);
    window->context->func_window_OnResize(window->ositem, obj_listener(window, i_OnWindowResize, Window));
    window->context->func_window_OnClose(window->ositem, obj_listener(window, i_OnWindowClose, Window));
    window->context->func_window_set_property(window->ositem, ekGUI_PROPERTY_CHILDREN, NULL);
    _gui_add_window(window);
    return window;
}

/*---------------------------------------------------------------------------*/

//static Window *i_create_window_managed(void *native_ptr)
//{
//    Window *window = obj_new0(Window);
//    window->context = gui_context_retain(gui_context_get_current());
//    window->ositem = window->context->func_window_managed(native_ptr);
//    window->flags = UINT32_MAX;
//    window->role = ENUM_MAX(gui_role_t);
//    _gui_add_window(window);
//    return window;
//}

/*---------------------------------------------------------------------------*/

static void i_main_layout_compose(Window *window, const S2Df *content_required_size)
{
    S2Df main_layout_size;
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_set_taborder);
    cassert_no_nullf(window->context->func_window_set_size);
    _layout_compose(window->main_layout, content_required_size, &main_layout_size);
    window->context->func_window_set_taborder(window->ositem, NULL);
    _layout_taborder(window->main_layout, window);
    window->context->func_window_set_size(window->ositem, main_layout_size.width, main_layout_size.height);
    //_component_set_frame(_panel_get_component(window->main_panel), &kV2D_ZEROf, &main_panel_size);
    _layout_locate(window->main_layout);
}

/*---------------------------------------------------------------------------*/

static void i_attach_main_layout(Window *window, const S2Df *content_size, Layout **layout)
{
    Panel *main_panel = NULL;
    GuiComponent *panel_component = NULL;
    cassert_no_null(window);
    cassert(window->main_layout == NULL);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_attach_main_panel_to_window);
    cassert_no_null(layout);
    window->main_layout = *layout;
    main_panel = layout_get_panel(window->main_layout, 0, 0);
    _panel_window(main_panel, window);
    panel_component = _panel_get_component(main_panel);
    cassert_no_null(panel_component);
    window->context->func_attach_main_panel_to_window(window->ositem, panel_component->ositem);
    i_main_layout_compose(window, content_size);
    _component_visible(panel_component, TRUE);
}

/*---------------------------------------------------------------------------*/

Window *window_create(const uint32_t flags)
{
    Window *window = i_create_window(flags);
    window->context->func_window_set_size(window->ositem, 320, 200);
    return window;
}

/*---------------------------------------------------------------------------*/

void window_panel(Window *window, Panel *panel)
{
    Layout *layout = layout_create(1, 1);
    layout_panel(layout, panel, 0, 0);
    i_attach_main_layout(window, NULL, &layout);
}

/*---------------------------------------------------------------------------*/

//Window *window_create(const uint32_t flags, Panel **main_panel);
//Window *window_create(const uint32_t flags, Panel **main_panel)
//{
//    Window *window = i_create_window(flags);
//    i_attach_main_panel(window, NULL, main_panel);
//    return window;
//}

/*---------------------------------------------------------------------------*/

//func(window_create_sized).Crea una nueva ventana, indicando el tama�o del �rea de cliente.
//fret(Window*).La ventana reci�n creada.
//fpar(const uint32_t,flags).Combinaci�n de valores <lt>window_flag_t</lt>.
//fpar(const S2Df,content_size).Tama�o del panel principal.
//fpar(Panel**,main_panel).Panel principal, que integra el contenido de la ventana.
//fnote.El panel principal se expandir� hasta alcanzar el tama�o <c>content_size</c>. Ver <lh>Expansi�n de celdas</lh>.

//Window *window_create_sized(const uint32_t flags, const S2Df content_size, Panel **main_panel);
//Window *window_create_sized(const uint32_t flags, const S2Df content_size, Panel **main_panel)
//{
//    Window *window = i_create_window(flags);
//    i_attach_main_panel(window, (content_size.width > 0 && content_size.height > 0) ? &content_size : NULL, main_panel);
//    return window;
//}

/*---------------------------------------------------------------------------*/

//Window *window_managed(void *native_ptr);
//Window *window_managed(void *native_ptr)
//{
//    return i_create_window_managed(native_ptr);
//}

/*---------------------------------------------------------------------------*/

void window_OnMoved(Window *window, Listener *listener)
{
    component_update_listener(
                    window, &window->OnMoved, listener, i_OnWindowMoved, 
                    window->context->func_window_OnMoved,
                    Window);
}

/*---------------------------------------------------------------------------*/

void window_OnResize(Window *window, Listener *listener)
{
    cassert_no_null(window);
    listener_update(&window->OnResize, listener);
}

/*---------------------------------------------------------------------------*/

void window_OnClose(Window *window, Listener *listener)
{
    cassert_no_null(window);
    listener_update(&window->OnClose, listener);
}

/*---------------------------------------------------------------------------*/

/*
Panel *window_get_main_panel(Window *window);
Panel *window_get_main_panel(Window *window)
{
    cassert_no_null(window);
    return window->main_panel;
}*/

/*---------------------------------------------------------------------------*/

/*
void window_set_edited(Window *window, const bool_t is_edited);
void window_set_edited(Window *window, const bool_t is_edited)
{
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_set_edited);
    window->context->func_window_set_edited(window->ositem, is_edited);
}
*/

/*---------------------------------------------------------------------------*/

/*
void window_set_alpha(Window *window, const real32_t alpha);
void window_set_alpha(Window *window, const real32_t alpha)
{
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_set_alpha);
    window->context->func_window_set_alpha(window->ositem, alpha);
}*/

/*---------------------------------------------------------------------------*/

/*void window_enabled_mouse_events(Window *window, const bool_t enabled);
void window_enabled_mouse_events(Window *window, const bool_t enabled)
{
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_enable_mouse_events);
    window->context->func_window_enable_mouse_events(window->ositem, enabled);
}*/

/*---------------------------------------------------------------------------*/

void window_title(Window *window, const char_t *text)
{
    const char_t *ltext;
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_set_title);
    ltext = _gui_respack_text(text, &window->titleid);
    window->context->func_window_set_title(window->ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void window_show(Window *window)
{
    cassert_no_null(window);
    if (window->visible == FALSE)
    {
        cassert_no_null(window->context);
        cassert_no_nullf(window->context->func_window_launch);
        window->visible = TRUE;
        if (window->role == ENUM_MAX(gui_role_t))
            window->role = ekGUI_ROLE_MAIN;
        window->context->func_window_launch(window->ositem, NULL);
    }
}

/*---------------------------------------------------------------------------*/

void window_hide(Window *window)
{
    cassert_no_null(window);
    cassert(window->visible == TRUE);
    if (window->visible == TRUE)
    {
        cassert_no_null(window->context);
        cassert_no_nullf(window->context->func_window_hide);
        window->context->func_window_hide(window->ositem, NULL);
        window->visible = FALSE;
    }
}

/*---------------------------------------------------------------------------*/

void window_launch_overlay(Window *window, Window *parent_window)
{
    if (window->visible == FALSE)
    {
        void *renderable_window = NULL;
        cassert_no_null(window->context);
        cassert_no_nullf(window->context->func_window_launch);
        cassert_no_null(parent_window);
        renderable_window = _window_ositem(parent_window);
        window->visible = TRUE;
        window->role = ekGUI_ROLE_OVERLAY;
        window->context->func_window_launch(window->ositem, renderable_window);
    }
}

/*---------------------------------------------------------------------------*/

uint32_t window_modal(Window *window, Window *parent)
{
    cassert_no_null(window);
    cassert(window->visible == FALSE);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_launch_modal);
    if (window->visible == FALSE)
    {
        window->role = ekGUI_ROLE_MODAL;
        window->visible = TRUE;
        return window->context->func_window_launch_modal(window->ositem, (parent != NULL) ? parent->ositem : NULL);
    }
    else
    {
        cassert(FALSE);
        return UINT32_MAX;
    }
}

/*---------------------------------------------------------------------------*/

void window_stop_modal(Window *window, const uint32_t return_value)
{
    cassert_no_null(window);
    cassert(window->role == ekGUI_ROLE_MODAL);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_stop_modal);
    if (window->role == ekGUI_ROLE_MODAL)
    {
        window->context->func_window_stop_modal(window->ositem, return_value);
        window->visible = FALSE;
        window->role = ENUM_MAX(gui_role_t);
    }
}

/*---------------------------------------------------------------------------*/

void window_update(Window *window)
{
    _window_update(window);
}

/*---------------------------------------------------------------------------*/

/*
void window_launch_sheet(Window *window, Window *owner_window);
void window_launch_sheet(Window *window, Window *owner_window)
{
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_null(owner_window);
    cassert_no_nullf(window->context->func_window_launch_sheet);
    cassert_no_nullf(window->context->func_window_set_z_order);
    cassert(FALSE);
    window->context->func_window_launch_sheet(window->ositem, owner_window->ositem);
    window->context->func_window_set_z_order(window->ositem, NULL);
}*/

/*---------------------------------------------------------------------------*/

/*
void window_stop_sheet(Window *window, Window *owner_window);
void window_stop_sheet(Window *window, Window *owner_window)
{
    cassert_no_null(window);
    cassert_no_null(owner_window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_stop_sheet);
    cassert(FALSE);
    window->context->func_window_stop_sheet(window->ositem, owner_window->ositem);
}
*/

/*---------------------------------------------------------------------------*/

/*
void window_set_origin_in_screen_center(Window *window);
void window_set_origin_in_screen_center(Window *window)
{
    real32_t screen_width, screen_height; 
    real32_t window_width, window_height;
    real32_t x, y;
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_globals_resolution);
    cassert_no_nullf(window->context->func_window_get_size);
    cassert_no_nullf(window->context->func_window_set_origin_in_screen_coordinates);
    window->context->func_globals_resolution(NULL, &screen_width, &screen_height);
    window->context->func_window_get_size(window->ositem, &window_width, &window_height);
    x = .5f * (screen_width - window_width);
    y = .5f * (screen_height - window_height);
    window->context->func_window_set_origin_in_screen_coordinates(window->ositem, x, y);
}*/

/*---------------------------------------------------------------------------*/

/*
void window_set_origin_in_screen_width_center(Window *window, const real32_t y);
void window_set_origin_in_screen_width_center(Window *window, const real32_t y)
{
    real32_t screen_width, screen_height; 
    real32_t window_width, window_height;
    real32_t x;
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_globals_resolution);
    cassert_no_nullf(window->context->func_window_get_size);
    cassert_no_nullf(window->context->func_window_set_origin_in_screen_coordinates);
    window->context->func_globals_resolution(NULL, &screen_width, &screen_height);
    window->context->func_window_get_size(window->ositem, &window_width, &window_height);
    x = .5f * (screen_width - window_width);
    window->context->func_window_set_origin_in_screen_coordinates(window->ositem, x, y);
}*/

/*---------------------------------------------------------------------------*/

/*
void window_set_origin_in_screen_width_right(Window *window, const real32_t offset_x, const real32_t y);
void window_set_origin_in_screen_width_right(Window *window, const real32_t offset_x, const real32_t y)
{
    real32_t screen_width, screen_height; 
    real32_t window_width, window_height;
    real32_t x;
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_globals_resolution);
    cassert_no_nullf(window->context->func_window_get_size);
    cassert_no_nullf(window->context->func_window_set_origin_in_screen_coordinates);
    window->context->func_globals_resolution(NULL, &screen_width, &screen_height);
    window->context->func_window_get_size(window->ositem, &window_width, &window_height);
    x = screen_width - window_width - offset_x;
    window->context->func_window_set_origin_in_screen_coordinates(window->ositem, x, y);
}*/

/*---------------------------------------------------------------------------*/

void window_origin(Window *window, const V2Df origin)
{
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_set_origin_in_screen_coordinates);
    window->context->func_window_set_origin_in_screen_coordinates(window->ositem, origin.x, origin.y);
}

/*---------------------------------------------------------------------------*/

void window_size(Window *window, const S2Df size)
{
    cassert_no_null(window);
    cassert(window->flags & ekWNRES);
    i_main_layout_compose(window, &size);
}

/*---------------------------------------------------------------------------*/

V2Df window_get_origin(const Window *window)
{
    V2Df origin;
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_get_origin_in_screen_coordinates);
    window->context->func_window_get_origin_in_screen_coordinates(window->ositem, &origin.x, &origin.y);
    return origin;
}

/*---------------------------------------------------------------------------*/

V2Df _window_get_origin(const Window *window)
{
    return window_get_origin(window);
}

/*---------------------------------------------------------------------------*/

S2Df window_get_size(const Window *window)
{
    S2Df size;
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_get_size);
    window->context->func_window_get_size(window->ositem, &size.width, &size.height);
    return size;
}

/*---------------------------------------------------------------------------*/

S2Df _window_get_size(const Window *window)
{
    return window_get_size(window);
}

/*---------------------------------------------------------------------------*/

S2Df window_get_client_size(const Window *window)
{
    Panel *panel = NULL;
    GuiComponent *component = NULL;
    S2Df size;
    cassert_no_null(window);
    panel = layout_get_panel(window->main_layout, 0, 0);
    component = _panel_get_component(panel);
    window->context->func_get_size[ekGUI_COMPONENT_PANEL](component->ositem, &size.width, &size.height);
    return size;
}

/*---------------------------------------------------------------------------*/

void window_defbutton(Window *window, Button *button)
{
    cassert_no_null(window);
    if (button != NULL)
    {
        cassert(_button_is_pushbutton(button) == TRUE);
        window->context->func_window_set_default_pushbutton(window->ositem, ((GuiComponent*)button)->ositem);
    }
    else
    {
        window->context->func_window_set_default_pushbutton(window->ositem, NULL);
    }
}

/*---------------------------------------------------------------------------*/

void window_cursor(Window *window, const cursor_t cursor, const Image *image, const real32_t hot_x, const real32_t hot_y)
{
    const Cursor *oscursor = _gui_cursor(cursor, image, hot_x, hot_y);
    cassert_no_null(window);
    window->context->func_window_set_cursor(window->ositem, (Cursor*)oscursor);
}

/*---------------------------------------------------------------------------*/

/*
//void* window_imp_imp(Window *window);
//#define window_imp(window, type)\
//    (type*)window_imp_imp(window)
//void* window_imp_imp(Window *window)
//{
//    cassert_no_null(window);
//    return window->ositem;
//}
*/

/*---------------------------------------------------------------------------*/

void _window_locale(Window *window)
{
    Panel *main_panel = NULL;
    cassert_no_null(window);
    main_panel = layout_get_panel(window->main_layout, 0, 0);
    _panel_locale(main_panel);
    if (window->titleid != NULL)
    {
        const char_t *text = _gui_respack_text(window->titleid, NULL);
        window->context->func_window_set_title(window->ositem, text);
    }

    _window_update(window);
}

/*---------------------------------------------------------------------------*/

gui_role_t _window_role(const Window *window)
{
    cassert_no_null(window);
    return window->role;
}

/*---------------------------------------------------------------------------*/

/*
//void *window_get_object_by_tag_imp(const Window *window, const uint32_t tag, const char_t *type);
//#define window_get_object_by_tag(window, tag, type)\
//    (type*)window_get_object_by_tag_imp(window, tag, #type)
//void *window_get_object_by_tag_imp(const Window *window, const uint32_t tag, const char_t *type)
//{
//    cassert_no_null(window);
//    return _panel_get_object_by_tag(window->main_panel, tag, type);
//}
*/

/*---------------------------------------------------------------------------*/

/*
//void *window_set_property_imp(Window *window, const gui_property_t property, void *value);
//#define window_set_property(window, property, value, type_property, type_return)\
//    (\
//        ((type_property*)value == value),\
//        (type_return*)window_set_property_imp(window, property, (void*)value)\
//    )
//void* window_set_property_imp(Window *window, const gui_property_t property, void *value)
//{
//    cassert_no_null(window);
//    cassert_no_null(window->context);
//    cassert_no_nullf(window->context->func_window_set_property);
//    return window->context->func_window_set_property(window->ositem, property, value);
//}
*/

/*---------------------------------------------------------------------------*/

void _window_update(Window *window)
{
    cassert_no_null(window);
    if (window->flags & ekWNRES)
    {
        S2Df current_panel_size;
        Panel *main_panel = layout_get_panel(window->main_layout, 0, 0);
        GuiComponent *component = _panel_get_component(main_panel);
        window->context->func_get_size[ekGUI_COMPONENT_PANEL](component->ositem, &current_panel_size.width, &current_panel_size.height);
        i_main_layout_compose(window, &current_panel_size);
    }
    else
    {
        i_main_layout_compose(window, NULL);
    }
}

/*---------------------------------------------------------------------------*/

void *_window_ositem(Window *window)
{
    cassert_no_null(window);
    return window->ositem;
}

/*---------------------------------------------------------------------------*/

bool_t _window_is_visible(const Window *window)
{
    cassert_no_null(window);
    return (bool_t)window->visible;
}

/*---------------------------------------------------------------------------*/

void _window_taborder(Window *window, void *ositem)
{
    cassert_no_null(window);
    cassert_no_null(window->context);
    cassert_no_nullf(window->context->func_window_set_taborder);
    window->context->func_window_set_taborder(window->ositem, ositem);
}
