/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: panel.c
 *
 */

/* Panel */

#include "panel.h"
#include "panel.inl"
#include "component.inl"
#include "gui.inl"
#include "layout.inl"
#include "window.inl"
#include "guictx.h"

#include "arrpt.h"
#include "cassert.h"
#include "ptr.h"
#include "objh.h"
#include "s2d.h"
#include "strings.h"
#include "v2d.h"

struct _panel_t
{
    GuiComponent component;
    Window *window;
    ArrPt(Layout) *layouts;
    uint32_t visible_layout;
    uint32_t active_layout;
    uint32_t flags;
    S2Df natural_size;
    S2Df control_size;
    S2Df content_size;
    void *data;
    FPtr_destroy func_destroy_data;
    ArrPt(GuiComponent) *visible_children;
    ArrPt(GuiComponent) *children;
};

/*---------------------------------------------------------------------------*/

void _panel_destroy(Panel **panel)
{
    GuiComponent *component;
    cassert_no_null(panel);
    cassert_no_null(*panel);
    component = &(*panel)->component;
    _component_destroy(&component);
}

/*---------------------------------------------------------------------------*/

void _panel_destroy_all(Panel **panel)
{
    cassert_no_null(panel);
    cassert_no_null(*panel);

    if ((*panel)->data != NULL)
    {
        if ((*panel)->func_destroy_data != NULL)
            (*panel)->func_destroy_data(&(*panel)->data);
    }

    _component_destroy_imp(&(*panel)->component);
    arrpt_destroy(&(*panel)->layouts, _layout_destroy, Layout);
    arrpt_destroy(&(*panel)->children, _component_destroy, GuiComponent);
    arrpt_destroy(&(*panel)->visible_children, NULL, GuiComponent);
    obj_delete(panel, Panel);
}

/*---------------------------------------------------------------------------*/

static Panel *i_create(const uint32_t flags)
{
    const GuiCtx *context = guictx_get_current();
    Panel *panel = obj_new0(Panel);
    void *ositem = context->func_create[ekGUI_TYPE_PANEL](flags);
    _component_init(&panel->component, context, PARAM(type, ekGUI_TYPE_PANEL), &ositem);
    panel->layouts = arrpt_create(Layout);
    panel->visible_layout = UINT32_MAX;
    panel->active_layout = UINT32_MAX;
    panel->natural_size = s2df(-1.f, -1.f);
    panel->control_size = s2df((flags & ekVIEW_HSCROLL) ? 256.f : -1.f, (flags & ekVIEW_VSCROLL) ? 256.f : -1.f);
    panel->content_size = s2df(-1.f, -1.f);
    panel->flags = flags;
    panel->children = arrpt_create(GuiComponent);
    panel->visible_children = arrpt_create(GuiComponent);
    return panel;
}

/*---------------------------------------------------------------------------*/

Panel *panel_create(void)
{
    return i_create(0);
}

/*---------------------------------------------------------------------------*/

Panel *panel_scroll(const bool_t hscroll, const bool_t vscroll)
{
    uint32_t flags = 0;
    flags |= (hscroll == TRUE) ? ekVIEW_HSCROLL : 0;
    flags |= (vscroll == TRUE) ? ekVIEW_VSCROLL : 0;
    return i_create(flags);
}

/*---------------------------------------------------------------------------*/

void panel_data_imp(Panel *panel, void **data, FPtr_destroy func_destroy_data)
{
    cassert_no_null(panel);
    cassert(panel->data == NULL);
    cassert(panel->func_destroy_data == NULL);
    panel->data = ptr_dget_no_null(data, void);
    panel->func_destroy_data = func_destroy_data;
}

/*---------------------------------------------------------------------------*/

void *panel_get_data_imp(const Panel *panel)
{
    cassert_no_null(panel);
    return panel->data;
}

/*---------------------------------------------------------------------------*/

void panel_size(Panel *panel, const S2Df size)
{
    cassert_no_null(panel);
    panel->control_size = size;
}

/*---------------------------------------------------------------------------*/

uint32_t panel_layout(Panel *panel, Layout *layout)
{
    uint32_t layout_index;
    cassert_no_null(panel);
    cassert_no_null(layout);
    layout_index = arrpt_size(panel->layouts, Layout);
    arrpt_append(panel->layouts, layout, Layout);
    _layout_attach_to_panel(layout, panel);
    if (layout_index == 0)
    {
        cassert(panel->active_layout == UINT32_MAX);
        panel->active_layout = 0;
    }
    else
    {
        cassert(panel->active_layout != UINT32_MAX);
    }

    return layout_index;
}

/*---------------------------------------------------------------------------*/

Layout *panel_get_layout(Panel *panel, const uint32_t index)
{
    cassert_no_null(panel);
    return arrpt_get(panel->layouts, index, Layout);
}

/*---------------------------------------------------------------------------*/

void panel_visible_layout(Panel *panel, const uint32_t index)
{
    cassert_no_null(panel);
    cassert(index < arrpt_size(panel->layouts, Layout));
    if (panel->active_layout != index)
        panel->active_layout = index;
}

/*---------------------------------------------------------------------------*/

void panel_update(Panel *panel)
{
    cassert_no_null(panel);
    cassert_no_null(panel->component.context);
    cassert_no_nullf(panel->component.context->func_panel_set_need_display);
    if (panel->window != NULL)
    {
        panel->component.context->func_panel_set_need_display(panel->component.ositem);
        _window_update(panel->window);
    }
}

/*---------------------------------------------------------------------------*/

real32_t panel_scroll_width(const Panel *panel)
{
    real32_t v = 0;
    cassert_no_null(panel);
    cassert_no_null(panel->component.context);
    cassert_no_nullf(panel->component.context->func_panel_scroller_size);
    panel->component.context->func_panel_scroller_size(panel->component.ositem, &v, NULL);
    return v;
}

/*---------------------------------------------------------------------------*/

real32_t panel_scroll_height(const Panel *panel)
{
    real32_t v = 0;
    cassert_no_null(panel);
    cassert_no_null(panel->component.context);
    cassert_no_nullf(panel->component.context->func_panel_scroller_size);
    panel->component.context->func_panel_scroller_size(panel->component.ositem, NULL, &v);
    return v;
}

/*---------------------------------------------------------------------------*/

static void i_detach_component(GuiComponent *panel_component, GuiComponent *component)
{
    Panel *panels[GUI_COMPONENT_MAX_PANELS];
    uint32_t i, num_panels;
    _component_panels(component, &num_panels, panels);
    for (i = 0; i < num_panels; ++i)
        _panel_detach_components(panels[i]);

    _component_detach_from_panel(panel_component, component);
}

/*---------------------------------------------------------------------------*/

void _panel_attach_component(Panel *panel, GuiComponent *component)
{
    cassert_no_null(panel);
    cassert_no_null(component);
    if (arrpt_find(panel->children, component, GuiComponent) == UINT32_MAX)
    {
        arrpt_append(panel->children, component, GuiComponent);
        _component_attach_to_panel((GuiComponent*)panel, component);
    }
}

/*---------------------------------------------------------------------------*/

void _panel_detach_components(Panel *panel)
{
    cassert_no_null(panel);
    arrpt_foreach(child, panel->children, GuiComponent)
        i_detach_component(&panel->component, child);
    arrpt_end();
}

/*---------------------------------------------------------------------------*/

static void i_hide_component(GuiComponent *component)
{
    Panel *panels[GUI_COMPONENT_MAX_PANELS];
    uint32_t i, n;
    _component_visible(component, FALSE);
    _component_panels(component, &n, panels);

    for(i = 0; i < n; ++i)
    {
        arrpt_foreach(child, panels[i]->children, GuiComponent)
            i_hide_component(child);
        arrpt_end();
    }
}

/*---------------------------------------------------------------------------*/

void _panel_destroy_component(Panel *panel, GuiComponent *component)
{
    bool_t exists = FALSE;
    cassert_no_null(panel);
    cassert_no_null(component);

    /* Check if component exists in any panel layout */
    /*arrpt_foreach(layout, panel->layouts, Layout)
        if (_layout_search_component(layout, component) != NULL)
        {
            exists = TRUE;
            break;
        }
    arrpt_end()*/

    if (exists == FALSE)
    {
        register uint32_t index;

        /* Prevent flickering in Windows because destroyed component new parent
        will be set to NULL (Desktop HWND) when is detached from this panel. */
        #if defined(__WINDOWS__)
        i_hide_component(component);
        #endif

        _component_set_parent_window(component, NULL);
        i_detach_component(&panel->component, component);
        index = arrpt_find(panel->children, component, GuiComponent);
        cassert(index != UINT32_MAX);
        arrpt_delete(panel->children, index, _component_destroy, GuiComponent);
        index = arrpt_find(panel->visible_children, component, GuiComponent);
        if (index != UINT32_MAX)
            arrpt_delete(panel->visible_children, index, NULL, GuiComponent);
    }
}

/*---------------------------------------------------------------------------*/

void _panel_hide_all(Panel *panel)
{
    cassert_no_null(panel);
    i_hide_component(&panel->component);
}

/*---------------------------------------------------------------------------*/

void _panel_invalidate_layout(Panel *panel, Layout *layout)
{
    cassert_no_null(panel);
    cassert_no_null(panel->component.context);
    cassert_no_nullf(panel->component.context->func_panel_set_need_display);
    if (panel->visible_layout != UINT32_MAX)
    {
        Layout *main_layout = arrpt_get(panel->layouts, panel->visible_layout, Layout);
        if (_layout_search_layout(main_layout, layout) == TRUE)
            panel->visible_layout = UINT32_MAX;
    }

    /* panel->component.context->func_panel_set_need_display(panel->component.ositem); */
}

/*---------------------------------------------------------------------------*/

GuiComponent *_panel_get_component(Panel *panel)
{
    cassert_no_null(panel);
    return &(panel)->component;
}

/*---------------------------------------------------------------------------*/

/*
void *_panel_get_object_by_tag(const Panel *panel, const uint32_t tag, const char_t *type);
void *_panel_get_object_by_tag(const Panel *panel, const uint32_t tag, const char_t *type)
{
    cassert_no_null(panel);
    if (panel->component.tag.tag_uint32 == tag)
    {
        cassert(str_equ_c(type, "Panel") == TRUE);
        return (void*)panel;
    }

    if (panel->active_layout != UINT32_MAX)
    {
        const Layout *layout = arrpt_get(panel->layouts, panel->active_layout, Layout);
        void *component = _layout_get_object_by_tag(layout, tag, type);
        if (component != NULL)
            return component;
    }

    arrpt_foreach(layout, panel->layouts, Layout)
        if (layout_i != panel->active_layout)
        {
            void *component = _layout_get_object_by_tag(layout, tag, type);
            if (component != NULL)
                return component;
        }
    arrpt_end();

    return NULL;
}*/

/*---------------------------------------------------------------------------*/

void _panel_panels(const Panel *panel, uint32_t *num_panels, Panel **panels)
{
    cassert_no_null(panel);
    cassert_no_null(num_panels);
    cassert_no_null(panels);
    *num_panels = 1;
    panels[0] = (Panel*)panel;
}

/*---------------------------------------------------------------------------*/

void _panel_window(Panel *panel, Window *window)
{
    cassert_no_null(panel);

    if (window != NULL)
    {
        cassert(panel->window == NULL);
        panel->window = obj_retain(window, Window);
    }
    else
    {
        obj_release(&panel->window, Window);
    }

    {
        register GuiComponent **child;
        register uint32_t i, num_elems;
        child = arrpt_all(panel->children, GuiComponent);
        num_elems = arrpt_size(panel->children, GuiComponent);
        for (i = 0; i < num_elems; ++i, ++child)
        {
            cassert_no_null(child);
            _component_set_parent_window(*child, panel->window);
        }
    }
}

/*---------------------------------------------------------------------------*/

Window *_panel_get_window(Panel *panel)
{
    cassert_no_null(panel);
    return panel->window;
}

/*---------------------------------------------------------------------------*/

static void i_activate_layout(
                        ArrPt(Layout) *layouts,
                        const uint32_t active_layout,
                        uint32_t *visible_layout,
                        ArrPt(GuiComponent) **visible_children)
{
    Layout *layout = NULL;
    ArrPt(GuiComponent) *layout_components = NULL;
    cassert_no_null(visible_layout);
    cassert_no_null(visible_children);
    cassert(*visible_layout != active_layout);
    layout = arrpt_get(layouts, active_layout, Layout);
    cassert_no_null(layout);
    layout_components = arrpt_create(GuiComponent);
    _layout_components(layout, layout_components);

    /*! <Hidden components that's become visible> */
    arrpt_foreach(component, layout_components, GuiComponent)
        if (arrpt_find(*visible_children, component, GuiComponent) == UINT32_MAX)
            _component_visible(component, TRUE);
    arrpt_end()

    /*! <Visible components that's become hidden> */
    arrpt_foreach(component, *visible_children, GuiComponent)
        if (arrpt_find(layout_components, component, GuiComponent) == UINT32_MAX)
            _component_visible(component, FALSE);
    arrpt_end()

    arrpt_destroy(visible_children, NULL, GuiComponent);
    *visible_children = layout_components;
    *visible_layout = active_layout;
}

/*---------------------------------------------------------------------------*/

void _panel_compose(Panel *panel, const S2Df *required_size, S2Df *final_size)
{
    Layout *layout = NULL;

    cassert_no_null(panel);

    if (panel->visible_layout != panel->active_layout)
        i_activate_layout(panel->layouts, panel->active_layout, &panel->visible_layout, &panel->visible_children);

    layout = arrpt_get(panel->layouts, panel->active_layout, Layout);
    _layout_compose(layout, required_size, final_size);

    cassert(panel->control_size.width <= 0 || final_size->width <= panel->control_size.width);
    cassert(panel->control_size.height <= 0 || final_size->height <= panel->control_size.height);
}

/*---------------------------------------------------------------------------*/

void _panel_dimension(Panel *panel, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    Layout *layout = NULL;
    cassert_no_null(panel);
    if (panel->visible_layout != panel->active_layout)
        i_activate_layout(panel->layouts, panel->active_layout, &panel->visible_layout, &panel->visible_children);

    layout = arrpt_get(panel->layouts, panel->active_layout, Layout);
    _layout_dimension(layout, di, dim0, dim1);

    if (di == 0)
    {
        panel->natural_size.width = *dim0;
        if (panel->control_size.width > 0)
            *dim0 = panel->control_size.width;
    }
    else
    {
        cassert(di == 1);
        panel->natural_size.height = *dim1;
        if (panel->control_size.height > 0)
            *dim1 = panel->control_size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _panel_expand(Panel *panel, const uint32_t di, const real32_t current_size, const real32_t required_size, real32_t *final_size)
{
    Layout *layout = NULL;
    real32_t cursize = 0;
    real32_t reqsize = 0;
    real32_t laysize = 0;
    cassert_no_null(panel);
    cassert_no_null(final_size);
    cassert(panel->active_layout == panel->visible_layout);
    layout = arrpt_get(panel->layouts, panel->active_layout, Layout);

    if (di == 0)
    {
        cursize = panel->natural_size.width;

        if (panel->control_size.width > 0)
        {
            cassert_unref(current_size == panel->control_size.width, current_size);
            if (required_size > panel->control_size.width)
                reqsize = required_size;
            else
                reqsize = panel->control_size.width;
        }
        else
        {
            reqsize = required_size;
        }
    }
    else
    {
        cassert(di == 1);
        cursize = panel->natural_size.height;

        if (panel->control_size.height > 0)
        {
            cassert(current_size == panel->control_size.height);
            if (required_size > panel->control_size.height)
                reqsize = required_size;
            else
                reqsize = panel->control_size.height;
        }
        else
        {
            reqsize = required_size;
        }
    }

    _layout_expand(layout, di, cursize, reqsize, &laysize);

    if (di == 0 && panel->control_size.width > 0)
        *final_size = required_size;
    else if (di == 1 && panel->control_size.height > 0)
        *final_size = required_size;
    else
        *final_size = laysize;

    if (panel->flags & ekVIEW_HSCROLL || panel->flags & ekVIEW_VSCROLL)
    {
        if (di == 0)
        {
            panel->content_size.width = laysize;
        }
        else
        {
            cassert(di == 1);
            panel->content_size.height = laysize;
            panel->component.context->func_panel_content_size(panel->component.ositem, panel->content_size.width, panel->content_size.height, 10, 10);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _panel_taborder(const Panel *panel, Window *window)
{
    Layout *layout = NULL;
    cassert_no_null(panel);
    cassert(panel->visible_layout == panel->active_layout);
    cassert_unref(panel->window == window, window);
    layout = arrpt_get(panel->layouts, panel->active_layout, Layout);
    _layout_taborder(layout, panel->window);
}

/*---------------------------------------------------------------------------*/

void _panel_locate(Panel *panel)
{
    Layout *layout = NULL;
    cassert_no_null(panel);
    cassert(panel->visible_layout == panel->active_layout);
    layout = arrpt_get(panel->layouts, panel->active_layout, Layout);
    _layout_locate(layout);
}

/*---------------------------------------------------------------------------*/

void _panel_locale(Panel *panel)
{
    cassert_no_null(panel);
    arrpt_foreach(child, panel->children, GuiComponent)
        _component_locale(child);
    arrpt_end();
}

/*---------------------------------------------------------------------------*/

ArrPt(Layout) *_panel_layouts(const Panel *panel)
{
    cassert_no_null(panel);
    return panel->layouts;
}

/*---------------------------------------------------------------------------*/

bool_t _panel_with_scroll(const Panel *panel)
{
    cassert_no_null(panel);
    return (bool_t)((panel->flags & ekVIEW_HSCROLL) != 0 || (panel->flags & ekVIEW_VSCROLL) != 0);
}
