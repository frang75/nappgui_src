/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: split.c
 *
 */

/* Split view */

#include "split.h"
#include "split.inl"
#include "component.inl"
#include "gui.inl"
#include "guicontext.inl"
#include "guicontexth.inl"
#include "obj.inl"
#include "panel.inl"

#include "bmath.h"
#include "cassert.h"
#include "event.h"
#include "ptr.h"
#include "s2d.h"
#include "v2d.h"

struct _splitview_t
{
    GuiComponent component;
    S2Df init_size;
    S2Df real_size;
    split_flag_t flags;
    bool_t is_dragging;
    real32_t div_min;
    real32_t div_pos;
    real32_t div_thick;
    real32_t div_start;
    real32_t div_drag;
    GuiComponent *child1;
    GuiComponent *child2;
};

/*---------------------------------------------------------------------------*/

static void i_horizontal_frame(const real32_t div_pos, const real32_t div_thick, const S2Df *split_size, R2Df *rect1, R2Df *rect2, R2Df *rect_track)
{
    real32_t divider_y;
    cassert_no_null(split_size);
    cassert_no_null(rect1);
    cassert_no_null(rect2);
    cassert_no_null(rect_track);
    divider_y = bmath_roundf(div_pos * split_size->height);
    rect1->pos = kV2D_ZEROf;
    rect1->size.width = split_size->width;
    rect1->size.height = divider_y;
    rect2->pos.x = 0;
    rect2->pos.y = rect1->size.height;
    rect2->size.width = split_size->width;
    rect2->size.height = split_size->height - rect1->size.height;
    rect_track->pos.x = 0;
    rect_track->pos.y = bmath_roundf(divider_y - .5f * div_thick);
    rect_track->size.width = split_size->width;
    rect_track->size.height = div_thick;
}

/*---------------------------------------------------------------------------*/

static void i_vertical_frame(const real32_t div_pos, const real32_t div_thick, const S2Df *split_size, R2Df *rect1, R2Df *rect2, R2Df *rect_track)
{
    real32_t divider_x;
    cassert_no_null(split_size);
    cassert_no_null(rect1);
    cassert_no_null(rect2);
    cassert_no_null(rect_track);
    divider_x = bmath_roundf(div_pos * split_size->width);
    rect1->pos = kV2D_ZEROf;
    rect1->size.width = divider_x;
    rect1->size.height = split_size->height;
    rect2->pos.x = rect1->size.width;
    rect2->pos.y = 0;
    rect2->size.width = split_size->width - rect1->size.width;
    rect2->size.height = split_size->height;
    rect_track->pos.x = bmath_roundf(divider_x - .5f * div_thick);
    rect_track->pos.y = 0;
    rect_track->size.width = div_thick;
    rect_track->size.height = split_size->height;
}

/*---------------------------------------------------------------------------*/

static void i_update_divider(
                        const GuiComponent *component,
                        const real32_t new_div_pos,
                        const real32_t div_min,
                        GuiComponent *component1,
                        GuiComponent *component2,
                        real32_t *div_pos)
{
    bool_t is_visible1;
    bool_t is_visible2;
    bool_t should_visible1;
    bool_t should_visible2;
    cassert_no_null(component);
    cassert_no_null(component->context);
    cassert_no_null(div_pos);
    cassert_no_null(component1);
    cassert_no_null(component2);
    if (*div_pos == 0)
    {
        is_visible1 = FALSE;
        is_visible2 = TRUE;
    }
    else if (*div_pos == 1)
    {
        is_visible1 = TRUE;
        is_visible2 = FALSE;
    }
    else
    {
        is_visible1 = TRUE;
        is_visible2 = TRUE;
    }

    if (new_div_pos < div_min)
        *div_pos = 0;
    else if (new_div_pos > 1 - div_min)
        *div_pos = 1;
    else
        *div_pos = new_div_pos;

    if (*div_pos == 0)
    {
        should_visible1 = FALSE;
        should_visible2 = TRUE;
    }
    else if (*div_pos == 1)
    {
        should_visible1 = TRUE;
        should_visible2 = FALSE;
    }
    else
    {
        should_visible1 = TRUE;
        should_visible2 = TRUE;
    }

    if (is_visible1 != should_visible1)
        _component_visible(component1, should_visible1);

    if (is_visible2 != should_visible2)
        _component_visible(component2, should_visible2);
}   

/*---------------------------------------------------------------------------*/

static void i_OnDivider(SplitView *split, Event *e)
{
    const EvMouse *params = event_params(e, EvMouse);
    cassert_no_null(split);
    switch (event_type(e)) {
        case ekEVDRAG:
            cassert(params->button == ekMLEFT);
            if (split->is_dragging == TRUE)
            {
                real32_t new_div_pos;
                if (split_type(split->flags) == ekSPHORZ)
                    new_div_pos = split->div_drag + (params->y - split->div_start) / split->real_size.height;
                else
                    new_div_pos = split->div_drag + (params->x - split->div_start) / split->real_size.width;
                i_update_divider(&split->component, new_div_pos, split->div_min, split->child1, split->child2, &split->div_pos);
                _splitview_OnResize(split, &split->real_size);
            }
            else
            {
                cassert(split->div_start == 0);
                cassert(split->div_drag == 0);
                split->is_dragging = TRUE;
                split->div_start = split_type(split->flags) == ekSPHORZ ? params->y : params->x;
                split->div_drag = split->div_pos;
            }

            break;

        case ekEVUP:
            cassert(params->button == ekMLEFT);
            cassert_no_null(split->component.context);
            cassert_no_nullf(split->component.context->func_split_track_area);
            split->is_dragging = FALSE;
            split->div_start = 0;
            split->div_drag = 0;
            break;

        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static SplitView *i_create(const split_flag_t flags)
{
    const GuiContext *context = gui_context_get_current();
    SplitView *split = obj_new0(SplitView);
    void *ositem = context->func_split_create((enum_t)flags);
    _component_init(&split->component, context, ekGUI_COMPONENT_SPLITVIEW, &ositem);
    split->init_size = s2df(128, 128);
    split->real_size = kS2D_ZEROf;
    split->flags = flags;
    split->is_dragging = FALSE;
    split->div_min = 0.01f;
    split->div_pos = .5f;
    split->div_thick = 8;
    split->div_start = 0;
    split->div_drag = 0;
    context->func_split_OnMoved(split->component.ositem, obj_listener(split, i_OnDivider, SplitView));
    return split;
}

/*---------------------------------------------------------------------------*/

SplitView *splitview_horizontal(void)
{
    return i_create(ekSPHORZ);
}

/*---------------------------------------------------------------------------*/

SplitView *splitview_vertical(void)
{
    return i_create(ekSPVERT);
}

/*---------------------------------------------------------------------------*/

static void i_add_child(SplitView *split, GuiComponent *component)
{
    cassert_no_null(split);
    cassert_no_null(component);
    if (split->child1 == NULL)
    {
        split->child1 = component;
    }
    else
    {
        cassert(split->child2 == NULL);
        split->child2 = component;
    }

    split->component.context->func_split_attach_control(split->component.ositem, component->ositem);
}

/*---------------------------------------------------------------------------*/

void splitview_view(SplitView *split, View *view)
{
    i_add_child(split, (GuiComponent*)view);
}

/*---------------------------------------------------------------------------*/

void splitview_text(SplitView *split, TextView *view)
{
    i_add_child(split, (GuiComponent*)view);
}

/*---------------------------------------------------------------------------*/

void splitview_split(SplitView *split, SplitView *child)
{
    i_add_child(split, (GuiComponent*)child);
}

/*---------------------------------------------------------------------------*/

void splitview_panel(SplitView *split, Panel *panel)
{
    i_add_child(split, (GuiComponent*)panel);
}

/*---------------------------------------------------------------------------*/

void splitview_divider(SplitView *split, const real32_t pos)
{
    cassert_no_null(split);
    split->div_pos = pos;
}

/*---------------------------------------------------------------------------*/

void _splitview_destroy(SplitView **split)
{
    cassert_no_null(split);
    cassert_no_null(*split);
    if ((*split)->child1 != NULL)
    {
        (*split)->component.context->func_split_detach_control((*split)->component.ositem, (*split)->child1->ositem);
        _component_destroy(&(*split)->child1);

        if ((*split)->child2 != NULL)
        {
            (*split)->component.context->func_split_detach_control((*split)->component.ositem, (*split)->child2->ositem);
            _component_destroy(&(*split)->child2);
        }
    }
    else
    {
        cassert((*split)->child2 != NULL);
    }

    _component_destroy_imp(&(*split)->component);
    obj_delete(split, SplitView);
}

/*---------------------------------------------------------------------------*/

void _splitview_dimension(SplitView *split, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(split);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        *dim0 = split->init_size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = split->init_size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _splitview_OnResize(SplitView *split, const S2Df *size)
{
    cassert_no_null(split);
    cassert_no_null(size);
    if (split->child1 != NULL && split->child2 != NULL)
    {    
        R2Df r1, r2, rtrack;
        if (split_type(split->flags) == ekSPHORZ)
            i_horizontal_frame(split->div_pos, split->div_thick, size, &r1, &r2, &rtrack);
        else
            i_vertical_frame(split->div_pos, split->div_thick, size, &r1, &r2, &rtrack);

        if (split->div_pos > 0)
            _component_set_frame(split->child1, &r1.pos, &r1.size);

        if (split->div_pos < 1)
            _component_set_frame(split->child2, &r2.pos, &r2.size);

        split->component.context->func_split_track_area(split->component.ositem, rtrack.pos.x, rtrack.pos.y, rtrack.size.width, rtrack.size.height);
    }
    else if (split->child1 != NULL)
    {
        _component_set_frame(split->child1, &kV2D_ZEROf, size);
        split->component.context->func_split_track_area(split->component.ositem, 0, 0, 0, 0);
    }

    split->real_size = *size;
}
