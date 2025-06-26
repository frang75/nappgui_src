/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: splitview.c
 *
 */

/* Split view */

#include "splitview.h"
#include "splitview.inl"
#include "component.inl"
#include "gui.inl"
#include "panel.inl"
#include <draw2d/guictx.h>
#include <geom2d/r2d.h>
#include <geom2d/s2d.h>
#include <geom2d/v2d.h>
#include <core/event.h>
#include <core/objh.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/types.h>

struct _splitview_t
{
    GuiComponent component;
    S2Df size;
    uint32_t flags;
    split_mode_t divider_mode;
    real32_t natural_divpos;
    real32_t user_divpos;
    real32_t drag_divpos;
    real32_t minsize0;
    real32_t minsize1;
    real32_t mindrag0;
    real32_t mindrag1;
    GuiComponent *child0;
    GuiComponent *child1;
    bool_t child0_visible;
    bool_t child1_visible;
    bool_t child0_tabstop;
    bool_t child1_tabstop;
    real32_t chid0_natural_dim[2];
    real32_t chid1_natural_dim[2];
    bool_t dim_expand[2];
    R2Df child0_rect;
    R2Df child1_rect;
};

static const real32_t i_DIVIDER_THICKNESS = 10;

/*---------------------------------------------------------------------------*/

static real32_t i_convert_clamp_divpos(const split_mode_t from_mode, const split_mode_t to_mode, const real32_t divpos, const real32_t size)
{
    cassert(size > 0);
    switch (from_mode)
    {
    case ekSPLIT_NORMAL:
        cassert(divpos >= 0 && divpos <= 1);
        switch (to_mode)
        {
        case ekSPLIT_NORMAL:
            return divpos;
        case ekSPLIT_FIXED0:
            return bmath_clampf(bmath_ceilf(divpos * size), 0, size);
        case ekSPLIT_FIXED1:
            return bmath_clampf(bmath_ceilf((1 - divpos) * size), 0, size);
            cassert_default();
        }
        break;

    case ekSPLIT_FIXED0:
    {
        real32_t pos = bmath_clampf(divpos, 0, size);
        switch (to_mode)
        {
        case ekSPLIT_NORMAL:
            return pos / size;
        case ekSPLIT_FIXED0:
            return pos;
        case ekSPLIT_FIXED1:
            return size - pos;
            cassert_default();
        }
        break;
    }

    case ekSPLIT_FIXED1:
    {
        real32_t pos = bmath_clampf(divpos, 0, size);
        switch (to_mode)
        {
        case ekSPLIT_NORMAL:
            return 1 - (pos / size);
        case ekSPLIT_FIXED0:
            return size - pos;
        case ekSPLIT_FIXED1:
            return pos;
            cassert_default();
        }
        break;
    }

        cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static real32_t i_divpos_to_px(SplitView *split, const real32_t size)
{
    cassert_no_null(split);
    /* Mouse-moved divider position has high priority */
    if (split->drag_divpos >= 0)
        return i_convert_clamp_divpos(split->divider_mode, ekSPLIT_FIXED0, split->drag_divpos, size);

    /* splitview_pos() */
    if (split->user_divpos >= 0)
        return i_convert_clamp_divpos(split->divider_mode, ekSPLIT_FIXED0, split->user_divpos, size);

    /* Divider position calculated from children sizes */
    cassert(split->natural_divpos >= 0);
    return i_convert_clamp_divpos(ekSPLIT_NORMAL, ekSPLIT_FIXED0, split->natural_divpos, size);
}

/*---------------------------------------------------------------------------*/

static bool_t i_child0_displayed(const SplitView *split)
{
    cassert_no_null(split);
    if (split->child0 == NULL)
        return FALSE;

    if (split->child0_visible == FALSE)
        return FALSE;

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_child1_displayed(const SplitView *split)
{
    cassert_no_null(split);
    if (split->child1 == NULL)
        return FALSE;

    if (split->child1_visible == FALSE)
        return FALSE;

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_adjust_minimum(const real32_t min0, const real32_t min1, real32_t *div_pos, real32_t *size, const bool_t can_expand)
{
    cassert_no_null(div_pos);
    cassert_no_null(size);
    cassert(min0 >= 0);
    cassert(min1 >= 0);
    cassert(*div_pos >= 0);

    if (min0 + min1 > *size)
    {
        if (can_expand == TRUE)
            *size = min0 + min1;
    }

    if (min0 > 0 && *div_pos < min0)
        *div_pos = min0;

    if (min1 > 0 && *size - *div_pos < min1)
        *div_pos = *size - min1;

    *div_pos = bmath_clampf(*div_pos, 0, *size);
}

/*---------------------------------------------------------------------------*/

static R2Df i_rect_track(SplitView *split, const S2Df *size)
{
    bool_t display0 = i_child0_displayed(split);
    bool_t display1 = i_child1_displayed(split);
    R2Df rect_track = kR2D_ZEROf;
    cassert_no_null(split);
    cassert_no_null(size);
    if (display0 == TRUE && display1 == TRUE)
    {
        if (split_get_type(split->flags) == ekSPLIT_HORZ)
        {
            real32_t divider_y = i_divpos_to_px(split, size->height);
            rect_track.pos.x = 0;
            rect_track.pos.y = bmath_ceilf(divider_y - .5f * i_DIVIDER_THICKNESS);
            rect_track.size.width = size->width;
            rect_track.size.height = i_DIVIDER_THICKNESS;
        }
        else
        {
            real32_t divider_x = i_divpos_to_px(split, size->width);
            rect_track.pos.x = bmath_ceilf(divider_x - .5f * i_DIVIDER_THICKNESS);
            rect_track.pos.y = 0;
            rect_track.size.width = i_DIVIDER_THICKNESS;
            rect_track.size.height = size->height;
        }
    }
    /* The child0 will fill all the splitview area */
    else if (display0 == TRUE)
    {
        /* If child1 is hidden by user, no rect_track will be defined */
        if (split->child1_visible == TRUE)
        {
            /* The rect_track at bottom */
            if (split_get_type(split->flags) == ekSPLIT_HORZ)
            {
                rect_track.pos.x = 0;
                rect_track.pos.y = size->height - i_DIVIDER_THICKNESS;
                rect_track.size.width = size->width;
                rect_track.size.height = i_DIVIDER_THICKNESS;
            }
            /* The rect_track at right */
            else
            {
                rect_track.pos.x = size->width - i_DIVIDER_THICKNESS;
                rect_track.pos.y = 0;
                rect_track.size.width = i_DIVIDER_THICKNESS;
                rect_track.size.height = size->height;
            }
        }
        else
        {
            rect_track = kR2D_ZEROf;
        }
    }
    /* The child1 will fill all the splitview area */
    else if (display1 == TRUE)
    {
        /* If child0 is hidden by user, no rect_track will be defined */
        if (split->child0_visible == TRUE)
        {
            /* The rect_track at top */
            if (split_get_type(split->flags) == ekSPLIT_HORZ)
            {
                rect_track.pos = kV2D_ZEROf;
                rect_track.size.width = size->width;
                rect_track.size.height = i_DIVIDER_THICKNESS;
            }
            /* The rect_track at left */
            else
            {
                rect_track.pos = kV2D_ZEROf;
                rect_track.size.width = i_DIVIDER_THICKNESS;
                rect_track.size.height = size->height;
            }
        }
        else
        {
            rect_track = kR2D_ZEROf;
        }
    }
    /* No child displayed (unusual) */
    else
    {
        rect_track = kR2D_ZEROf;
    }

    return rect_track;
}

/*---------------------------------------------------------------------------*/

static real32_t i_frame_dimension(SplitView *split, const uint32_t di, const real32_t total_size, real32_t *pos0, real32_t *size0, real32_t *pos1, real32_t *size1, const bool_t can_expand)
{
    bool_t display0 = i_child0_displayed(split);
    bool_t display1 = i_child1_displayed(split);
    cassert_no_null(split);
    cassert_no_null(pos0);
    cassert_no_null(pos1);
    cassert_no_null(size0);
    cassert_no_null(size1);
    if (display0 == TRUE && display1 == TRUE)
    {
        if ((split_get_type(split->flags) == ekSPLIT_HORZ && di == 1) || (split_get_type(split->flags) == ekSPLIT_VERT && di == 0))
        {
            real32_t divider_px = i_divpos_to_px(split, total_size);
            real32_t total = total_size;
            i_adjust_minimum(max_r32(split->minsize0, split->mindrag0), max_r32(split->minsize1, split->mindrag1), &divider_px, &total, can_expand);
            *pos0 = 0;
            *pos1 = divider_px;
            *size0 = divider_px;
            *size1 = total - divider_px;
            return divider_px;
        }
        else
        {
            *pos0 = 0;
            *pos1 = 0;
            *size0 = total_size;
            *size1 = total_size;
            return -1;
        }
    }
    else if (display0 == TRUE)
    {
        *pos0 = 0;
        *pos1 = 0;
        *size0 = total_size;
        *size1 = 0;
        return -1;
    }
    else if (display1 == TRUE)
    {
        *pos0 = 0;
        *pos1 = 0;
        *size0 = 0;
        *size1 = total_size;
        return -1;
    }
    else
    {
        *pos0 = 0;
        *pos1 = 0;
        *size0 = 0;
        *size1 = 0;
        return -1;
    }
}

/*---------------------------------------------------------------------------*/

static void i_recompute_dimension(SplitView *split, const uint32_t di, const real32_t required_size, real32_t *final_size, const bool_t can_expand)
{
    real32_t pos0 = 0, pos1 = 0;
    real32_t fsize0 = 0, fsize1 = 0;
    bool_t recompute = TRUE;
    cassert_no_null(split);

    while (recompute == TRUE)
    {
        real32_t size0 = 0, size1 = 0;
        real32_t divpos_px = i_frame_dimension(split, di, required_size, &pos0, &size0, &pos1, &size1, can_expand);

        if (size0 > 0)
            _component_expand(split->child0, di, split->chid0_natural_dim[di], size0, &fsize0);

        if (size1 > 0)
            _component_expand(split->child1, di, split->chid1_natural_dim[di], size1, &fsize1);

        /* The expansion affect to divider dimension */
        if ((split_get_type(split->flags) == ekSPLIT_HORZ && di == 1) || (split_get_type(split->flags) == ekSPLIT_VERT && di == 0))
        {
            real32_t tsize = size0 + size1;
            real32_t fsize = fsize0 + fsize1;

            /* The divider constraint cannot be met */
            if (bmath_absf(tsize - fsize) >= 1)
            {
                cassert(fsize > tsize);
                if (bmath_absf(size0 - fsize0) < 1)
                {
                    split->drag_divpos = i_convert_clamp_divpos(ekSPLIT_FIXED0, split->divider_mode, tsize - fsize1, tsize);
                    split->mindrag1 = fsize1;
                }
                else
                {
                    split->drag_divpos = i_convert_clamp_divpos(ekSPLIT_FIXED1, split->divider_mode, tsize - fsize0, tsize);
                    split->mindrag0 = fsize0;
                }
            }
            /* Constraint can be met */
            else
            {
                if (divpos_px >= 0)
                    split->drag_divpos = i_convert_clamp_divpos(ekSPLIT_FIXED0, split->divider_mode, divpos_px, tsize);
                ptr_assign(final_size, fsize);
                recompute = FALSE;
            }
        }
        /* The expansion affect to no-divider dimension */
        else
        {
            ptr_assign(final_size, max_r32(fsize0, fsize1));
            recompute = FALSE;
        }
    }

    cassert(sizeof(split->child0_rect.pos) == 2 * sizeof(real32_t));
    cassert(sizeof(split->child0_rect.size) == 2 * sizeof(real32_t));
    cast(&split->child0_rect.pos, real32_t)[di] = pos0;
    cast(&split->child0_rect.size, real32_t)[di] = fsize0;
    cast(&split->child1_rect.pos, real32_t)[di] = pos1;
    cast(&split->child1_rect.size, real32_t)[di] = fsize1;
}

/*---------------------------------------------------------------------------*/

static void i_resize_child(const R2Df *rect, GuiComponent *child)
{
    cassert_no_null(rect);
    if (rect->size.width > 0 && rect->size.height > 0)
    {
        if (child != NULL)
        {
            _component_set_frame(child, &rect->pos, &rect->size);
            _component_visible(child, TRUE);
        }
    }
    else if (child != NULL)
    {
        _component_visible(child, FALSE);
    }
}

/*---------------------------------------------------------------------------*/

static void i_recompute_dragging(SplitView *split)
{
    cassert_no_null(split);
    i_recompute_dimension(split, 0, split->size.width, NULL, FALSE);
    i_recompute_dimension(split, 1, split->size.height, NULL, FALSE);

    if (split->child0_rect.size.width > 0 && split->child0_rect.size.height > 0)
        _component_locate(split->child0);

    if (split->child1_rect.size.width > 0 && split->child1_rect.size.height > 0)
        _component_locate(split->child1);

    i_resize_child(&split->child0_rect, split->child0);
    i_resize_child(&split->child1_rect, split->child1);
}

/*---------------------------------------------------------------------------*/

static void i_recompute_rect_track(SplitView *split)
{
    R2Df rect_track;
    cassert_no_null(split);
    rect_track = i_rect_track(split, &split->size);
    if (rect_track.size.width > 0 && rect_track.size.height > 0)
        split->component.context->func_split_track_area(split->component.ositem, rect_track.pos.x, rect_track.pos.y, rect_track.size.width, rect_track.size.height);
    else
        split->component.context->func_split_track_area(split->component.ositem, 0, 0, 0, 0);
}

/*---------------------------------------------------------------------------*/

static void i_OnDrag(SplitView *split, Event *e)
{
    const EvMouse *params = event_params(e, EvMouse);

    if (event_type(e) == ekGUI_EVENT_DRAG)
    {
        real32_t mouse_pos = 0, size = 0;
        cassert_no_null(split);
        cassert(params->button == ekGUI_MOUSE_LEFT);
        if (split_get_type(split->flags) == ekSPLIT_HORZ)
        {
            mouse_pos = params->y;
            size = split->size.height;
        }
        else
        {
            mouse_pos = params->x;
            size = split->size.width;
        }

        split->drag_divpos = i_convert_clamp_divpos(ekSPLIT_FIXED0, split->divider_mode, mouse_pos, size);
        i_recompute_dragging(split);
    }
    else
    {
        /* End dragging */
        cassert(event_type(e) == ekGUI_EVENT_UP);
        i_recompute_rect_track(split);
        split->mindrag0 = 0;
        split->mindrag1 = 0;
    }
}

/*---------------------------------------------------------------------------*/

static SplitView *i_create(const uint32_t flags)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = context->func_create[ekGUI_TYPE_SPLITVIEW](flags);
    SplitView *split = obj_new0(SplitView);
    _component_init(&split->component, context, ekGUI_TYPE_SPLITVIEW, &ositem);
    split->flags = flags;
    split->divider_mode = ekSPLIT_NORMAL;
    split->natural_divpos = -1;
    split->user_divpos = -1;
    split->drag_divpos = -1;
    context->func_split_OnDrag(split->component.ositem, obj_listener(split, i_OnDrag, SplitView));
    return split;
}

/*---------------------------------------------------------------------------*/

SplitView *splitview_horizontal(void)
{
    return i_create(ekSPLIT_HORZ);
}

/*---------------------------------------------------------------------------*/

SplitView *splitview_vertical(void)
{
    return i_create(ekSPLIT_VERT);
}

/*---------------------------------------------------------------------------*/

static void i_add_child(SplitView *split, GuiComponent *component, const bool_t tabstop)
{
    cassert_no_null(split);
    cassert_no_null(component);
    if (split->child0 == NULL)
    {
        split->child0 = component;
        split->child0_tabstop = tabstop;
        split->child0_visible = TRUE;
    }
    else
    {
        cassert(split->child1 == NULL);
        split->child1 = component;
        split->child1_tabstop = tabstop;
        split->child1_visible = TRUE;
    }

    split->component.context->func_split_attach_control(split->component.ositem, component->ositem);
}

/*---------------------------------------------------------------------------*/

void splitview_view(SplitView *split, View *view, const bool_t tabstop)
{
    i_add_child(split, cast(view, GuiComponent), tabstop);
}

/*---------------------------------------------------------------------------*/

void splitview_textview(SplitView *split, TextView *view, const bool_t tabstop)
{
    i_add_child(split, cast(view, GuiComponent), tabstop);
}

/*---------------------------------------------------------------------------*/

void splitview_webview(SplitView *split, WebView *view, const bool_t tabstop)
{
    i_add_child(split, cast(view, GuiComponent), tabstop);
}

/*---------------------------------------------------------------------------*/

void splitview_tableview(SplitView *split, TableView *view, const bool_t tabstop)
{
    i_add_child(split, cast(view, GuiComponent), tabstop);
}

/*---------------------------------------------------------------------------*/

void splitview_splitview(SplitView *split, SplitView *view)
{
    i_add_child(split, cast(view, GuiComponent), TRUE);
}

/*---------------------------------------------------------------------------*/

void splitview_panel(SplitView *split, Panel *panel)
{
    i_add_child(split, cast(panel, GuiComponent), TRUE);
}

/*---------------------------------------------------------------------------*/

void splitview_pos(SplitView *split, const split_mode_t mode, const real32_t pos)
{
    cassert_no_null(split);
    if (pos >= 0)
    {
        real32_t size = split_get_type(split->flags) == ekSPLIT_HORZ ? split->size.height : split->size.width;
        if (split->user_divpos > 0)
            split->user_divpos = i_convert_clamp_divpos(split->divider_mode, mode, split->user_divpos, size);
        else
            split->user_divpos = pos;

        split->drag_divpos = -1;
    }

    split->divider_mode = mode;
}

/*---------------------------------------------------------------------------*/

real32_t splitview_get_pos(const SplitView *split, const split_mode_t mode)
{
    real32_t size = 0;
    cassert_no_null(split);
    if (split_get_type(split->flags) == ekSPLIT_HORZ)
        size = split->size.height;
    else
        size = split->size.width;

    if (split->drag_divpos >= 0)
        return i_convert_clamp_divpos(split->divider_mode, mode, split->drag_divpos, size);
    else if (split->user_divpos >= 0)
        return i_convert_clamp_divpos(split->divider_mode, mode, split->user_divpos, size);
    else
        return i_convert_clamp_divpos(ekSPLIT_NORMAL, mode, split->natural_divpos, size);
}

/*---------------------------------------------------------------------------*/

void splitview_visible0(SplitView *split, const bool_t visible)
{
    cassert_no_null(split);
    if (split->child0_visible != visible)
        split->child0_visible = visible;
}

/*---------------------------------------------------------------------------*/

void splitview_visible1(SplitView *split, const bool_t visible)
{
    cassert_no_null(split);
    if (split->child1_visible != visible)
        split->child1_visible = visible;
}

/*---------------------------------------------------------------------------*/

void splitview_minsize0(SplitView *split, const real32_t size)
{
    cassert_no_null(split);
    cassert(size >= 0);
    split->minsize0 = size;
}

/*---------------------------------------------------------------------------*/

void splitview_minsize1(SplitView *split, const real32_t size)
{
    cassert_no_null(split);
    cassert(size >= 0);
    split->minsize1 = size;
}

/*---------------------------------------------------------------------------*/

void _splitview_destroy(SplitView **split)
{
    cassert_no_null(split);
    cassert_no_null(*split);
    if ((*split)->child0 != NULL)
    {
        (*split)->component.context->func_split_detach_control((*split)->component.ositem, (*split)->child0->ositem);
        _component_destroy(&(*split)->child0);
    }

    if ((*split)->child1 != NULL)
    {
        (*split)->component.context->func_split_detach_control((*split)->component.ositem, (*split)->child1->ositem);
        _component_destroy(&(*split)->child1);
    }

    _component_destroy_imp(&(*split)->component);
    obj_delete(split, SplitView);
}

/*---------------------------------------------------------------------------*/

void _splitview_natural(SplitView *split, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    real32_t dim = 0;
    cassert_no_null(split);
    cassert_no_null(dim0);
    cassert_no_null(dim1);

    if (split->child0 != NULL && split->child0_visible == TRUE)
        _component_natural(split->child0, di, &split->chid0_natural_dim[0], &split->chid0_natural_dim[1]);
    else
        split->chid0_natural_dim[di] = 0;

    if (split->child1 != NULL && split->child1_visible == TRUE)
        _component_natural(split->child1, di, &split->chid1_natural_dim[0], &split->chid1_natural_dim[1]);
    else
        split->chid1_natural_dim[di] = 0;

    if ((split_get_type(split->flags) == ekSPLIT_HORZ && di == 1) || (split_get_type(split->flags) == ekSPLIT_VERT && di == 0))
    {
        dim = split->chid0_natural_dim[di] + split->chid1_natural_dim[di];
        split->natural_divpos = split->chid0_natural_dim[di] / dim;
    }
    else
    {
        dim = max_r32(split->chid0_natural_dim[di], split->chid1_natural_dim[di]);
    }

    if (di == 0)
    {
        *dim0 = dim;
        split->mindrag0 = 0;
        split->mindrag1 = 0;
        split->dim_expand[0] = FALSE;
        split->dim_expand[1] = FALSE;
    }
    else
    {
        *dim1 = dim;
    }
}

/*---------------------------------------------------------------------------*/

void _splitview_expand(SplitView *split, const uint32_t di, const real32_t current_size, const real32_t required_size, real32_t *final_size)
{
    cassert_no_null(split);
    unref(current_size);
    i_recompute_dimension(split, di, required_size, final_size, TRUE);
    split->dim_expand[di] = TRUE;
}

/*---------------------------------------------------------------------------*/

void _splitview_taborder(const SplitView *split, Window *window)
{
    bool_t display0 = FALSE;
    bool_t display1 = FALSE;
    cassert_no_null(split);
    display0 = i_child0_displayed(cast(split, SplitView));
    display1 = i_child1_displayed(cast(split, SplitView));
    if (display0 == TRUE && split->child0_tabstop == TRUE)
        _component_taborder(split->child0, window);
    if (display1 == TRUE && split->child1_tabstop == TRUE)
        _component_taborder(split->child1, window);
}

/*---------------------------------------------------------------------------*/

void _splitview_OnResize(SplitView *split, const S2Df *size)
{
    cassert_no_null(split);
    cassert_no_null(size);
    split->size = *size;

    /*
     * The split view is computed during expansion. It's possible that some dimensions
     * don't expand if the layout composer detects that the natural dimension is correct.
     */
    if (split->dim_expand[0] == FALSE)
    {
        i_recompute_dimension(split, 0, size->width, NULL, FALSE);
        split->dim_expand[0] = TRUE;
    }

    if (split->dim_expand[1] == FALSE)
    {
        i_recompute_dimension(split, 1, size->height, NULL, FALSE);
        split->dim_expand[1] = TRUE;
    }

    if (split->child0_rect.size.width > 0 && split->child0_rect.size.height > 0)
        _component_locate(split->child0);

    if (split->child1_rect.size.width > 0 && split->child1_rect.size.height > 0)
        _component_locate(split->child1);

    i_resize_child(&split->child0_rect, split->child0);
    i_resize_child(&split->child1_rect, split->child1);
    i_recompute_rect_track(split);
}

/*---------------------------------------------------------------------------*/

static void i_accum_panels(const SplitView *split, uint32_t *num_panels, Panel **panels);

/*---------------------------------------------------------------------------*/

static void i_accum_child_panels(const GuiComponent *component, uint32_t *num_panels, Panel **panels)
{
    cassert_no_null(num_panels);
    cassert_no_null(panels);
    if (component != NULL)
    {
        if (component->type == ekGUI_TYPE_PANEL)
        {
            panels[*num_panels] = cast(component, Panel);
            *num_panels += 1;
            cassert(*num_panels < GUI_COMPONENT_MAX_PANELS);
        }
        else if (component->type == ekGUI_TYPE_SPLITVIEW)
        {
            i_accum_panels(cast(component, SplitView), num_panels, panels);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_accum_panels(const SplitView *split, uint32_t *num_panels, Panel **panels)
{
    cassert_no_null(split);
    if (i_child0_displayed(split) == TRUE)
        i_accum_child_panels(split->child0, num_panels, panels);
    if (i_child1_displayed(split) == TRUE)
        i_accum_child_panels(split->child1, num_panels, panels);
}

/*---------------------------------------------------------------------------*/

void _splitview_panels(const SplitView *split, uint32_t *num_panels, Panel **panels)
{
    *num_panels = 0;
    i_accum_panels(split, num_panels, panels);
}
