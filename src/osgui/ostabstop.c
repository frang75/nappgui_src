/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabstop.c
 *
 */

/* Tabstops functionality */

#include "ostabstop.inl"
#include "oscontrol.inl"
#include "oscombo.inl"
#include "osedit.inl"
#include "ospanel.inl"
#include "ostext.inl"
#include "osview.inl"
#include "oswindow.inl"
#include <core/arrpt.h>
#include <sewer/cassert.h>
#include <sewer/bstd.h>

/*---------------------------------------------------------------------------*/

static int32_t i_SCROLL_OFFSET = 10;

/*---------------------------------------------------------------------------*/

static void i_default(OSTabStop *tabstop)
{
    cassert_no_null(tabstop);
    tabstop->controls = NULL;
    tabstop->tablist = NULL;
    tabstop->tabindex = UINT32_MAX;
    tabstop->motion = ENUM_MAX(gui_tab_t);
    tabstop->window = NULL;
    tabstop->current = NULL;
    tabstop->transient = NULL;
    tabstop->next = NULL;
    tabstop->defbutton = NULL;
    tabstop->cycle = TRUE;
}

/*---------------------------------------------------------------------------*/

void _ostabstop_init(OSTabStop *tabstop, OSWindow *window)
{
    cassert_no_null(tabstop);
    i_default(tabstop);
    tabstop->controls = arrpt_create(OSControl);
    tabstop->tablist = arrpt_create(OSControl);
    tabstop->window = window;
}

/*---------------------------------------------------------------------------*/

void _ostabstop_remove(OSTabStop *tabstop)
{
    cassert_no_null(tabstop);
    arrpt_destroy(&tabstop->controls, NULL, OSControl);
    arrpt_destroy(&tabstop->tablist, NULL, OSControl);
    i_default(tabstop);
}

/*---------------------------------------------------------------------------*/

static ___INLINE uint32_t i_tabindex(const OSTabStop *tabstop, const OSControl *control)
{
    cassert_no_null(tabstop);
    cassert(arrpt_find(tabstop->controls, control, OSControl) != UINT32_MAX);
    return arrpt_find(tabstop->tablist, control, OSControl);
}

/*---------------------------------------------------------------------------*/

static bool_t i_in_tablist(const OSTabStop *tabstop, const OSControl *control)
{
    return (bool_t)(i_tabindex(tabstop, control) != UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

static bool_t i_resign_focus(OSTabStop *tabstop, const OSControl *control)
{
    gui_type_t type = _oscontrol_type(control);
    cassert_no_null(tabstop);
    cassert(arrpt_find(tabstop->controls, control, OSControl) != UINT32_MAX);
    if (type == ekGUI_TYPE_EDITBOX)
        return _osedit_resign_focus(cast(control, OSEdit));
    else if (type == ekGUI_TYPE_COMBOBOX)
        return _oscombo_resign_focus(cast(control, OSCombo));
    else if (type == ekGUI_TYPE_CUSTOMVIEW)
        return _osview_resign_focus(cast(control, OSView));
    else if (type == ekGUI_TYPE_TEXTVIEW)
        return _ostext_resign_focus(cast(control, OSText));
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static OSControl *i_get_focus(OSTabStop *tabstop)
{
    cassert_no_null(tabstop);
    if (tabstop->current != NULL)
    {
        cassert(arrpt_find(tabstop->controls, tabstop->current, OSControl) != UINT32_MAX);
        return tabstop->current;
    }

    /* Unknown focus control */
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool_t _ostabstop_can_close_window(OSTabStop *tabstop)
{
    OSControl *focus = i_get_focus(tabstop);
    if (focus != NULL)
        return i_resign_focus(tabstop, focus);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

bool_t _ostabstop_capture_return(OSTabStop *tabstop)
{
    OSControl *control = i_get_focus(tabstop);
    if (control != NULL)
    {
        gui_type_t type = _oscontrol_type(control);
        cassert(arrpt_find(tabstop->controls, control, OSControl) != UINT32_MAX);
        if (type == ekGUI_TYPE_TEXTVIEW)
            return _ostext_capture_return(cast_const(control, OSText));
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t _ostabstop_capture_tab(OSTabStop *tabstop)
{
    OSControl *control = i_get_focus(tabstop);
    if (control != NULL)
    {
        gui_type_t type = _oscontrol_type(control);
        cassert(arrpt_find(tabstop->controls, control, OSControl) != UINT32_MAX);
        if (type == ekGUI_TYPE_CUSTOMVIEW)
            return _osview_capture_tab(cast_const(control, OSView));
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

#if defined __ASSERTS__

static bool_t i_dump_controls(const ArrPt(OSControl) *controls, const bool_t print)
{
    if (print == TRUE)
        bstd_printf("Dump controls: %d\n", arrpt_size(controls, OSControl));

    arrpt_foreach_const(control, controls, OSControl)
        {
            gui_type_t type = _oscontrol_type(control);
            const char_t *str = _oscontrol_type_str(type);
            if (print == TRUE)
                bstd_printf("  - %d: %s\n", control_i, str);
        }
    arrpt_end()

    return TRUE;
}

#endif

/*---------------------------------------------------------------------------*/

void _ostabstop_list_add(OSTabStop *tabstop, OSControl *control)
{
    cassert_no_null(tabstop);
    if (control != NULL)
    {
        gui_type_t type = _oscontrol_type(control);
        cassert_unref(type != ekGUI_TYPE_PANEL, type);
        arrpt_append(tabstop->tablist, control, OSControl);
    }
    else
    {
        /* Reset the tablist */
        arrpt_clear(tabstop->tablist, NULL, OSControl);
        arrpt_clear(tabstop->controls, NULL, OSControl);
        tabstop->tabindex = UINT32_MAX;
        tabstop->transient = NULL;

        _oswindow_find_all_controls(tabstop->window, tabstop->controls);
        if (tabstop->current != NULL)
        {
            if (arrpt_find(tabstop->controls, tabstop->current, OSControl) == UINT32_MAX)
                tabstop->current = NULL;
        }

        cassert(i_dump_controls(tabstop->controls, FALSE) == TRUE);
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_focus(OSTabStop *tabstop, OSControl *control)
{
    OSWidget *widget = _oscontrol_focus_widget(control);
    OSControl *parent = _oscontrol_parent(control);
    gui_type_t ptype = _oscontrol_type(parent);
    uint32_t tabindex = UINT32_MAX;
    cassert_no_null(tabstop);
    cassert(arrpt_find(tabstop->controls, control, OSControl) != UINT32_MAX);

    if (tabstop->transient != NULL)
        _ostabstop_release_transient(tabstop, tabstop->transient);

    _oswindow_widget_set_focus(tabstop->window, widget);
    tabstop->current = control;

    /* We remember the last tablist focused control */
    tabindex = arrpt_find(tabstop->tablist, control, OSControl);
    if (tabindex != UINT32_MAX)
        tabstop->tabindex = tabindex;

    /* Automatic panel scrolling if focused control is not completely visible */
    if (ptype == ekGUI_TYPE_PANEL)
    {
        OSPanel *panel = cast(parent, OSPanel);
        if (_ospanel_with_scroll(panel) == TRUE)
        {
            OSFrame prect, crect;
            int32_t scroll_x = INT32_MAX, scroll_y = INT32_MAX;
            _ospanel_scroll_frame(panel, &prect);
            _oscontrol_frame(control, &crect);

            if (prect.left > crect.left)
                scroll_x = (crect.left - i_SCROLL_OFFSET);
            else if (prect.right < crect.right)
                scroll_x = (crect.right + i_SCROLL_OFFSET) - (prect.right - prect.left);

            if (prect.top > crect.top)
                scroll_y = (crect.top - i_SCROLL_OFFSET);
            else if (prect.bottom < crect.bottom)
                scroll_y = (crect.bottom + i_SCROLL_OFFSET) - (prect.bottom - prect.top);

            if (scroll_x != INT32_MAX || scroll_y != INT32_MAX)
                _ospanel_scroll(panel, scroll_x, scroll_y);
        }
    }
}

/*---------------------------------------------------------------------------*/

static OSControl *i_effective_focus(OSTabStop *tabstop, OSControl *control, const bool_t forward)
{
    OSWidget *widget = _oscontrol_focus_widget(control);
    cassert_no_null(tabstop);

    if (_oscontrol_widget_visible(widget) == TRUE && _oscontrol_widget_enable(widget) == TRUE)
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
                OSWidget *twidget = _oscontrol_focus_widget(tcontrol);
                if (_oscontrol_widget_visible(twidget) == TRUE && _oscontrol_widget_enable(twidget) == TRUE)
                    return tcontrol;
            }
        }
    }

    /* No control is 'focusable' */
    return NULL;
}

/*---------------------------------------------------------------------------*/

static void i_on_focus(OSControl *control, const bool_t focused)
{
    gui_type_t type = _oscontrol_type(control);
    if (type == ekGUI_TYPE_EDITBOX)
        _osedit_focus(cast(control, OSEdit), focused);
    else if (type == ekGUI_TYPE_COMBOBOX)
        _oscombo_focus(cast(control, OSCombo), focused);
    else if (type == ekGUI_TYPE_CUSTOMVIEW)
        _osview_focus(cast(control, OSView), focused);
    else if (type == ekGUI_TYPE_TEXTVIEW)
        _ostext_focus(cast(control, OSText), focused);
}

/*---------------------------------------------------------------------------*/

static bool_t i_is_next_transient(const OSTabStop *tabstop, OSControl *focus, OSControl *next)
{
    if (next != NULL && focus != next)
    {
        /* If button is not in the tablist, can't get the focus */
        gui_type_t type = _oscontrol_type(next);
        if (type == ekGUI_TYPE_BUTTON && i_in_tablist(tabstop, next) == FALSE)
        {
            _oscontrol_set_can_focus(next, FALSE);
            return TRUE;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_accept_focus(OSTabStop *tabstop, const OSControl *control)
{
    gui_type_t type = _oscontrol_type(control);
    unref(tabstop);
    if (type == ekGUI_TYPE_CUSTOMVIEW)
        return _osview_accept_focus(cast(control, OSView));
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gui_focus_t i_try_change_focus(OSTabStop *tabstop, OSControl *control, const gui_tab_t motion, const bool_t forward)
{
    gui_focus_t fstate = ENUM_MAX(gui_focus_t);
    OSControl *focus = NULL;
    OSControl *next_control = NULL;
    bool_t resign = TRUE;
    cassert_no_null(tabstop);
    cassert(arrpt_find(tabstop->controls, control, OSControl) != UINT32_MAX);
    cassert(tabstop->motion == ENUM_MAX(gui_tab_t));
    cassert(tabstop->next == NULL);

    tabstop->motion = motion;
    focus = i_get_focus(tabstop);
    next_control = i_effective_focus(tabstop, control, forward);
    tabstop->next = next_control;

    if (i_is_next_transient(tabstop, focus, next_control) == TRUE)
        resign = FALSE;

    /*
     * 'focus' and 'next_control' can be the same but,
     * we force validation events in non-cycle-one-control tablists
     */
    if (resign == TRUE && focus != NULL)
        resign = i_resign_focus(tabstop, focus);

    /* The current focused control resign the focus */
    if (resign == TRUE)
    {
        if (next_control != NULL)
        {
            /* Here we don't continue if 'next_control' is the current 'focused' control */
            if (focus != next_control)
            {
                /* The next_control must explicitly accept focus */
                if (i_accept_focus(tabstop, next_control) == TRUE)
                {
                    i_set_focus(tabstop, next_control);

                    if (focus != next_control)
                    {
                        if (focus != NULL)
                            i_on_focus(focus, FALSE);

                        if (next_control != NULL)
                            i_on_focus(next_control, TRUE);
                    }

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
                OSWidget *widget = _oscontrol_focus_widget(next_control);
                _oswindow_widget_set_focus(tabstop->window, widget);
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
    tabstop->motion = ENUM_MAX(gui_tab_t);
    tabstop->next = NULL;
    return fstate;
}

/*---------------------------------------------------------------------------*/

gui_focus_t _ostabstop_next(OSTabStop *tabstop, const bool_t from_key)
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
        gui_tab_t motion = from_key ? ekGUI_TAB_KEY : ekGUI_TAB_NEXT;
        return i_try_change_focus(tabstop, next_control, motion, TRUE);
    }

    /* We don't know what the next control is */
    return ekGUI_FOCUS_NO_NEXT;
}

/*---------------------------------------------------------------------------*/

gui_focus_t _ostabstop_prev(OSTabStop *tabstop, const bool_t from_key)
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
        gui_tab_t motion = from_key ? ekGUI_TAB_BACKKEY : ekGUI_TAB_PREV;
        return i_try_change_focus(tabstop, prev_control, motion, FALSE);
    }

    /* We don't know what the previous control is */
    return ekGUI_FOCUS_NO_NEXT;
}

/*---------------------------------------------------------------------------*/

gui_focus_t _ostabstop_move(OSTabStop *tabstop, OSControl *control)
{
    return i_try_change_focus(tabstop, control, ekGUI_TAB_MOVE, TRUE);
}

/*---------------------------------------------------------------------------*/

gui_focus_t _ostabstop_restore(OSTabStop *tabstop)
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
    {
        i_set_focus(tabstop, control);
        i_on_focus(control, TRUE);
        return ekGUI_FOCUS_KEEP;
    }

    return ekGUI_FOCUS_NO_NEXT;
}

/*---------------------------------------------------------------------------*/

gui_tab_t _ostabstop_info_focus(const OSTabStop *tabstop, void **next_ctrl)
{
    cassert_no_null(tabstop);
    cassert_no_null(next_ctrl);
    *next_ctrl = tabstop->next;
    return tabstop->motion;
}

/*---------------------------------------------------------------------------*/

bool_t _ostabstop_mouse_down(OSTabStop *tabstop, OSControl *control)
{
    cassert_no_null(tabstop);
    /* Click over focused control allways is accepted */
    if (tabstop->current != control)
    {
        gui_focus_t fstate = i_try_change_focus(tabstop, control, ekGUI_TAB_CLICK, TRUE);
        switch (fstate)
        {
        case ekGUI_FOCUS_NO_RESIGN:
        {
            gui_type_t type = _oscontrol_type(control);
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

void _ostabstop_release_transient(OSTabStop *tabstop, OSControl *control)
{
    cassert_no_null(tabstop);
    if (tabstop->transient != NULL)
    {
        cassert_unref(tabstop->transient == control, control);
        _oscontrol_set_can_focus(tabstop->transient, TRUE);
        tabstop->transient = NULL;
        if (tabstop->current != NULL)
        {
            OSWidget *widget = _oscontrol_focus_widget(tabstop->current);
            _oswindow_widget_set_focus(tabstop->window, widget);
        }
    }
}
