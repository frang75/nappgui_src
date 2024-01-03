/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabstop.c
 *
 */

/* Tabstops functionality */

#include "ostabstop.inl"
#include "oscontrol.inl"
#include "osedit.inl"
#include "ospanel.inl"
#include "osview.inl"
#include "oswindow.inl"
#include <core/arrpt.h>
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

static int32_t i_SCROLL_OFFSET = 10;

/*---------------------------------------------------------------------------*/

void ostabstop_init(OSTabStop *tabstop, OSWindow *window)
{
    cassert_no_null(tabstop);
    tabstop->tablist = arrpt_create(OSControl);
    tabstop->tabindex = UINT32_MAX;
    tabstop->window = window;
    tabstop->current = NULL;
    tabstop->transient = NULL;
    tabstop->defbutton = NULL;
    tabstop->cycle = TRUE;
}

/*---------------------------------------------------------------------------*/

void ostabstop_remove(OSTabStop *tabstop)
{
    cassert_no_null(tabstop);
    arrpt_destroy(&tabstop->tablist, NULL, OSControl);
    tabstop->tabindex = UINT32_MAX;
    tabstop->current = NULL;
    tabstop->transient = NULL;
    tabstop->defbutton = NULL;
    tabstop->window = NULL;
}

/*---------------------------------------------------------------------------*/

static __INLINE uint32_t i_tabindex(const OSTabStop *tabstop, const OSControl *control)
{
    cassert_no_null(tabstop);
    return arrpt_find(tabstop->tablist, control, OSControl);
}

/*---------------------------------------------------------------------------*/

bool_t ostabstop_in_tablist(const OSTabStop *tabstop, const OSControl *control)
{
    return (bool_t)(i_tabindex(tabstop, control) != UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

bool_t ostabstop_resign_focus(OSTabStop *tabstop, const OSControl *control, const OSControl *next_control)
{
    gui_type_t type = oscontrol_type(control);
    unref(tabstop);
    if (type == ekGUI_TYPE_EDITBOX)
        return osedit_resign_focus((OSEdit *)control, next_control);
    else if (type == ekGUI_TYPE_CUSTOMVIEW)
        return osview_resign_focus((OSView *)control, next_control);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

bool_t ostabstop_accept_focus(OSTabStop *tabstop, const OSControl *control)
{
    gui_type_t type = oscontrol_type(control);
    unref(tabstop);
    if (type == ekGUI_TYPE_CUSTOMVIEW)
        return osview_accept_focus((OSView *)control);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static OSControl *i_get_focus(OSTabStop *tabstop)
{
    cassert_no_null(tabstop);
    if (tabstop->current != NULL)
    {
        return tabstop->current;
    }
    else
    {
        OSWidget *widget = oswindow_widget_get_focus(tabstop->window);

        /* Try to find the current window focus in the tablist */
        if (widget != NULL)
        {
            arrpt_foreach(control, tabstop->tablist, OSControl) if (oscontrol_focus_widget(control) == widget) return control;
            arrpt_end();
        }

        /* Unknown focus control */
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

bool_t ostabstop_can_close_window(OSTabStop *tabstop)
{
    OSControl *focus = i_get_focus(tabstop);
    if (focus != NULL)
        return ostabstop_resign_focus(tabstop, focus, NULL);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

void ostabstop_list_add(OSTabStop *tabstop, OSControl *control)
{
    cassert_no_null(tabstop);
    if (control != NULL)
    {
        gui_type_t type = oscontrol_type(control);
        cassert_unref(type != ekGUI_TYPE_PANEL, type);
        arrpt_append(tabstop->tablist, control, OSControl);
    }
    else
    {
        /* Reset the tablist */
        arrpt_clear(tabstop->tablist, NULL, OSControl);
        tabstop->tabindex = UINT32_MAX;
        tabstop->current = NULL;
        tabstop->transient = NULL;
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_focus(OSTabStop *tabstop, OSControl *control)
{
    OSWidget *widget = oscontrol_focus_widget(control);
    OSControl *parent = oscontrol_parent(control);
    gui_type_t ptype = oscontrol_type(parent);
    uint32_t tabindex = UINT32_MAX;
    cassert_no_null(tabstop);

    oswindow_widget_set_focus(tabstop->window, widget);
    tabstop->current = control;
    cassert(tabstop->transient == NULL);

    /* We remember the last tablist focused control */
    tabindex = arrpt_find(tabstop->tablist, control, OSControl);
    if (tabindex != UINT32_MAX)
        tabstop->tabindex = tabindex;

    /* Automatic panel scrolling if focused control is not completely visible */
    if (ptype == ekGUI_TYPE_PANEL)
    {
        OSPanel *panel = OSPanelPtr(parent);
        if (ospanel_with_scroll(panel) == TRUE)
        {
            OSFrame prect, crect;
            int32_t scroll_x = INT32_MAX, scroll_y = INT32_MAX;
            ospanel_scroll_frame(panel, &prect);
            oscontrol_frame(control, &crect);

            if (prect.left > crect.left)
                scroll_x = (crect.left - i_SCROLL_OFFSET);
            else if (prect.right < crect.right)
                scroll_x = (crect.right + i_SCROLL_OFFSET) - (prect.right - prect.left);

            if (prect.top > crect.top)
                scroll_y = (crect.top - i_SCROLL_OFFSET);
            else if (prect.bottom < crect.bottom)
                scroll_y = (crect.bottom + i_SCROLL_OFFSET) - (prect.bottom - prect.top);

            if (scroll_x != INT32_MAX || scroll_y != INT32_MAX)
                ospanel_scroll(panel, scroll_x, scroll_y);
        }
    }
}

/*---------------------------------------------------------------------------*/

static OSControl *i_effective_focus(OSTabStop *tabstop, OSControl *control, const bool_t forward)
{
    OSWidget *widget = oscontrol_focus_widget(control);
    cassert_no_null(tabstop);

    if (oscontrol_widget_visible(widget) == TRUE && oscontrol_widget_enable(widget) == TRUE)
    {
        return control;
    }
    /* If proposed control can't be 'focusable', we look for a candidate in the tablist */
    else
    {
        uint32_t idx = i_tabindex(tabstop, control);
        OSControl **controls = arrpt_all(tabstop->tablist, OSControl);
        uint32_t size = arrpt_size(tabstop->tablist, OSControl);
        uint32_t i = 0;

        if (idx == UINT32_MAX)
            idx = 0;

        for (i = 0; i < size; ++i)
        {
            /* Move to next control in tablist */
            if (forward == TRUE)
            {
                if (idx == size - 1)
                    idx = 0;
                else
                    idx += 1;
            }
            else
            {
                if (idx == 0)
                    idx = size - 1;
                else
                    idx -= 1;
            }

            {
                OSControl *tcontrol = controls[idx];
                OSWidget *twidget = oscontrol_focus_widget(tcontrol);
                if (oscontrol_widget_visible(twidget) == TRUE && oscontrol_widget_enable(twidget) == TRUE)
                    return tcontrol;
            }
        }
    }

    /* No control is 'focusable' */
    return NULL;
}

/*---------------------------------------------------------------------------*/

static gui_focus_t i_try_change_focus(OSTabStop *tabstop, OSControl *control, const bool_t forward)
{
    gui_focus_t fstate = ENUM_MAX(gui_focus_t);
    OSControl *focus = i_get_focus(tabstop);
    OSControl *next_control = i_effective_focus(tabstop, control, forward);
    bool_t resign = TRUE;
    cassert_no_null(tabstop);

    /*
     * 'focus' and 'next_control' can be the same but,
     * we force validation events in non-cycle-one-control tablists
     */
    if (focus != NULL)
        resign = ostabstop_resign_focus(tabstop, focus, next_control);

    /* The current focused control resign the focus */
    if (resign == TRUE)
    {
        if (next_control != NULL)
        {
            /* Here we don't continue if 'next_control' is the current 'focused' control */
            if (focus != next_control)
            {
                /* The next_control must explicitly accept focus */
                if (ostabstop_accept_focus(tabstop, next_control) == TRUE)
                {
                    i_set_focus(tabstop, next_control);
                    fstate = ekGUI_FOCUS_CHANGED;
                }
                else
                {
                    fstate = ekGUI_FOCUS_NO_ACCEPT;
                }
            }
            /* The focus remains in the same control, but the window may have been disabled. */
            else
            {
                OSWidget *widget = oscontrol_focus_widget(next_control);
                oswindow_widget_set_focus(tabstop->window, widget);
                fstate = ekGUI_FOCUS_KEEP;
            }
        }
        else
        {
            fstate = ekGUI_FOCUS_NO_NEXT;
        }
    }
    else
    {
        fstate = ekGUI_FOCUS_NO_RESIGN;
    }

    cassert(fstate != ENUM_MAX(gui_focus_t));
    return fstate;
}

/*---------------------------------------------------------------------------*/

gui_focus_t ostabstop_next(OSTabStop *tabstop)
{
    OSControl *focus = i_get_focus(tabstop);
    uint32_t tabindex = focus ? i_tabindex(tabstop, focus) : UINT32_MAX;
    uint32_t next_tabindex = UINT32_MAX;
    uint32_t size = 0;
    cassert_no_null(tabstop);
    size = arrpt_size(tabstop->tablist, OSControl);

    if (tabindex != UINT32_MAX)
    {
        next_tabindex = tabindex;

        if (next_tabindex == size - 1)
        {
            if (tabstop->cycle == TRUE)
                next_tabindex = 0;
        }
        else
        {
            next_tabindex += 1;
        }
    }
    /* The focused control is not in the tablist */
    else
    {
        /* We try to use the last focused tablist control */
        if (tabstop->tabindex != UINT32_MAX)
            next_tabindex = tabstop->tabindex;
        /* We finally use the first control in the tablist */
        else if (size > 0)
            next_tabindex = 0;
    }

    if (next_tabindex != UINT32_MAX)
    {
        OSControl *next_control = arrpt_get(tabstop->tablist, next_tabindex, OSControl);
        return i_try_change_focus(tabstop, next_control, TRUE);
    }

    /* We don't know what the next control is */
    return ekGUI_FOCUS_NO_NEXT;
}

/*---------------------------------------------------------------------------*/

gui_focus_t ostabstop_prev(OSTabStop *tabstop)
{
    OSControl *focus = i_get_focus(tabstop);
    uint32_t tabindex = focus ? i_tabindex(tabstop, focus) : UINT32_MAX;
    uint32_t prev_tabindex = UINT32_MAX;
    uint32_t size = 0;
    cassert_no_null(tabstop);
    size = arrpt_size(tabstop->tablist, OSControl);

    if (tabindex != UINT32_MAX)
    {
        prev_tabindex = tabindex;

        if (prev_tabindex == 0)
        {
            if (tabstop->cycle == TRUE)
                prev_tabindex = size - 1;
        }
        else
        {
            prev_tabindex -= 1;
        }
    }
    /* The focused control is not in the tablist */
    else
    {
        /* We try to use the last focused tablist control */
        if (tabstop->tabindex != UINT32_MAX)
            prev_tabindex = tabstop->tabindex;
        /* We finally use the last control in the tablist */
        else if (size > 0)
            prev_tabindex = size - 1;
    }

    if (prev_tabindex != UINT32_MAX)
    {
        OSControl *prev_control = arrpt_get(tabstop->tablist, prev_tabindex, OSControl);
        return i_try_change_focus(tabstop, prev_control, FALSE);
    }

    /* We don't know what the previous control is */
    return ekGUI_FOCUS_NO_NEXT;
}

/*---------------------------------------------------------------------------*/

gui_focus_t ostabstop_move(OSTabStop *tabstop, OSControl *control)
{
    return i_try_change_focus(tabstop, control, TRUE);
}

/*---------------------------------------------------------------------------*/

gui_focus_t ostabstop_restore(OSTabStop *tabstop)
{
    OSControl *control = NULL;
    cassert_no_null(tabstop);
    control = tabstop->current;
    if (control == NULL)
    {
        if (arrpt_size(tabstop->tablist, OSControl) > 0)
            control = arrpt_first(tabstop->tablist, OSControl);
    }

    if (control != NULL)
        return i_try_change_focus(tabstop, control, TRUE);

    return ekGUI_FOCUS_NO_NEXT;
}

/*---------------------------------------------------------------------------*/

bool_t ostabstop_mouse_down(OSTabStop *tabstop, OSControl *control)
{
    cassert_no_null(tabstop);
    /* Click over focused control allways is accepted */
    if (tabstop->current != control)
    {
        gui_focus_t fstate = i_try_change_focus(tabstop, control, TRUE);
        switch (fstate)
        {
        case ekGUI_FOCUS_NO_RESIGN: {
            gui_type_t type = oscontrol_type(control);
            if (type == ekGUI_TYPE_BUTTON)
            {
                cassert(tabstop->transient == NULL || tabstop->transient == control);
                tabstop->transient = control;
                return TRUE;
            }

            return FALSE;
        }

        case ekGUI_FOCUS_NO_ACCEPT:
        case ekGUI_FOCUS_CHANGED:
        case ekGUI_FOCUS_KEEP:
        case ekGUI_FOCUS_NO_NEXT:
            return TRUE;
            cassert_default();
        }
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

void ostabstop_force(OSTabStop *tabstop)
{
    cassert_no_null(tabstop);
    if (tabstop->current != NULL)
    {
        OSWidget *widget = oscontrol_focus_widget(tabstop->current);
        oswindow_widget_set_focus(tabstop->window, widget);
    }
}

/*---------------------------------------------------------------------------*/

void ostabstop_release_transient(OSTabStop *tabstop, OSControl *control)
{
    cassert_no_null(tabstop);
    if (tabstop->transient != NULL)
    {
        cassert_unref(tabstop->transient == control, control);
        tabstop->transient = NULL;
        if (tabstop->current != NULL)
        {
            OSWidget *widget = oscontrol_focus_widget(tabstop->current);
            oswindow_widget_set_focus(tabstop->window, widget);
        }
    }
}
