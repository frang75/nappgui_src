/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oslistener.c
 *
 */

/* View listeners */

#include "oslistener.inl"
#include "oscontrol.inl"
#include "osgui.inl"
#include "osgui_gtk.inl"
#include "cassert.h"
#include "event.h"
#include "bmem.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

/*---------------------------------------------------------------------------*/

void _oslistener_init(ViewListeners *listeners)
{
    cassert_no_null(listeners);
    bmem_zero(listeners, ViewListeners);
    listeners->is_enabled = TRUE;
    listeners->button = ENUM_MAX(mouse_t);
}

/*---------------------------------------------------------------------------*/

void _oslistener_remove(ViewListeners *listeners)
{
    cassert_no_null(listeners);
    listener_destroy(&listeners->OnDraw);
    listener_destroy(&listeners->OnEnter);
    listener_destroy(&listeners->OnExit);
    listener_destroy(&listeners->OnMoved);
    listener_destroy(&listeners->OnDown);
    listener_destroy(&listeners->OnUp);
    listener_destroy(&listeners->OnClick);
    listener_destroy(&listeners->OnDrag);
    listener_destroy(&listeners->OnWheel);
    listener_destroy(&listeners->OnKeyDown);
    listener_destroy(&listeners->OnKeyUp);
}

/*---------------------------------------------------------------------------*/

void _oslistener_signal(GtkWidget *widget, bool_t add, gint *signal_id, gint signal_mask, const gchar *signal_name, GCallback callback, gpointer callback_data)
{
    if (add == TRUE)
    {
        if (*signal_id == 0)
        {
            gtk_widget_add_events(widget, signal_mask);
            *signal_id = g_signal_connect(G_OBJECT(widget), signal_name, callback, callback_data);
        }
    }
    else
    {
        if (*signal_id != 0)
        {
            gint emask = gtk_widget_get_events(widget);
            emask &= ~signal_mask;
            gtk_widget_set_events(widget, emask);
            g_signal_handler_disconnect(G_OBJECT(widget), *signal_id);
            signal_id = 0;
        }
    }
}

/*---------------------------------------------------------------------------*/

//void _oslistener_set_enabled(ViewListeners *listeners, bool_t is_enabled)
//{
//    cassert_no_null(listeners);
//    listeners->is_enabled = is_enabled;
//}

/*---------------------------------------------------------------------------*/

void _oslistener_redraw(OSControl *sender, EvDraw *params, ViewListeners *listeners)
{
    cassert_no_null(sender);
    cassert(sender->type == ekGUI_COMPONENT_CUSTOMVIEW);
    cassert_no_null(listeners);
    if (listeners->OnDraw != NULL)
        listener_event(listeners->OnDraw, ekEVDRAW, sender, params, NULL, OSControl, EvDraw, void);
}

/*---------------------------------------------------------------------------*/

void _oslistener_mouse_enter(OSControl *sender, GdkEventCrossing *event, GtkAdjustment *hadjust, GtkAdjustment *vadjust, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    if (listeners->OnEnter != NULL)
    {
        EvMouse params;
        params.x = (real32_t)event->x;
        params.y = (real32_t)event->y;
        params.button = ENUM_MAX(mouse_t);
        params.count = 0;

        if (hadjust != NULL)
            params.x += (real32_t)gtk_adjustment_get_value(hadjust);

        if (vadjust != NULL)
            params.y += (real32_t)gtk_adjustment_get_value(vadjust);

        listener_event(listeners->OnEnter, ekEVENTER, sender, &params, NULL, OSControl, EvMouse, void);
    }
}

/*---------------------------------------------------------------------------*/

void _oslistener_mouse_exit(OSControl *sender, GdkEventCrossing *event, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    unref(event);
    if (listeners->OnExit != NULL)
        listener_event(listeners->OnExit, ekEVEXIT, sender, NULL, NULL, OSControl, void, void);
}

/*---------------------------------------------------------------------------*/

void _oslistener_mouse_moved(OSControl *sender, GdkEventMotion *event, GtkAdjustment *hadjust, GtkAdjustment *vadjust, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    cassert_no_null(sender);
    if (listeners->is_enabled == TRUE)
    {
        if (listeners->button != ENUM_MAX(mouse_t))
        {
            if (listeners->OnDrag != NULL)
            {
                EvMouse params;
                params.x = (real32_t)event->x;
                params.y = (real32_t)event->y;
                params.button = listeners->button;
                params.count = 0;

                if (hadjust != NULL)
                    params.x += (real32_t)gtk_adjustment_get_value(hadjust);

                if (vadjust != NULL)
                    params.y += (real32_t)gtk_adjustment_get_value(vadjust);

                listener_event(listeners->OnDrag, ekEVDRAG, sender, &params, NULL, OSControl, EvMouse, void);
            }
        }
        else
        {
            if (listeners->OnMoved != NULL)
            {
                EvMouse params;
                params.x = (real32_t)event->x;
                params.y = (real32_t)event->y;
                params.button = ENUM_MAX(mouse_t);
                params.count = 0;

                if (hadjust != NULL)
                    params.x += (real32_t)gtk_adjustment_get_value(hadjust);

                if (vadjust != NULL)
                    params.y += (real32_t)gtk_adjustment_get_value(vadjust);

                listener_event(listeners->OnMoved, ekEVMOVED, sender, &params, NULL, OSControl, EvMouse, void);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void _oslistener_mouse_down(OSControl *sender, GdkEventButton *event, GtkAdjustment *hadjust, GtkAdjustment *vadjust, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    if (listeners->is_enabled == TRUE)
    {
        switch(event->button) {
        case 1:
            listeners->button = ekMLEFT;
            break;
        case 2:
            listeners->button = ekMMIDDLE;
            break;
        case 3:
            listeners->button = ekMRIGHT;
            break;
        default:
            listeners->button = ekMLEFT;
            break;
        }

        if (listeners->OnDown != NULL)
        {
            EvMouse params;
            params.x = (real32_t)event->x;
            params.y = (real32_t)event->y;
            params.button = listeners->button;
            params.count = 0;

            if (hadjust != NULL)
                params.x += (real32_t)gtk_adjustment_get_value(hadjust);

            if (vadjust != NULL)
                params.y += (real32_t)gtk_adjustment_get_value(vadjust);

            listener_event(listeners->OnDown, ekEVDOWN, sender, &params, NULL, OSControl, EvMouse, void);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _oslistener_mouse_up(OSControl *sender, GdkEventButton *event, GtkAdjustment *hadjust, GtkAdjustment *vadjust, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    cassert_no_null(sender);
    if (listeners->is_enabled == TRUE && listeners->button != ENUM_MAX(mouse_t))
    {
        if (listeners->OnUp != NULL)
        {
            EvMouse params;
            params.x = (real32_t)event->x;
            params.y = (real32_t)event->y;
            params.button = listeners->button;
            params.count = 0;

            if (hadjust != NULL)
                params.x += (real32_t)gtk_adjustment_get_value(hadjust);

            if (vadjust != NULL)
                params.y += (real32_t)gtk_adjustment_get_value(vadjust);

            listener_event(listeners->OnUp, ekEVUP, sender, &params, NULL, OSControl, EvMouse, void);
        }

        if (listeners->OnClick != NULL)
        {
            EvMouse params;
            params.x = (real32_t)event->x;
            params.y = (real32_t)event->y;
            params.button = listeners->button;

            switch(event->type) {
            case GDK_BUTTON_PRESS:
                params.count = 1;
                break;
            case GDK_2BUTTON_PRESS:
                params.count = 2;
                break;
            case GDK_3BUTTON_PRESS:
                params.count = 3;
                break;
            default:
                params.count = 1;
                break;
            }

            if (hadjust != NULL)
                params.x += (real32_t)gtk_adjustment_get_value(hadjust);

            if (vadjust != NULL)
                params.y += (real32_t)gtk_adjustment_get_value(vadjust);

            listener_event(listeners->OnClick, ekEVCLICK, sender, &params, NULL, OSControl, EvMouse, void);
        }

        listeners->button = ENUM_MAX(mouse_t);
    }
}

/*---------------------------------------------------------------------------*/

void _oslistener_scroll_whell(OSControl *sender, GdkEventScroll *event, GtkAdjustment *hadjust, GtkAdjustment *vadjust, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    cassert_no_null(sender);
    if (listeners->is_enabled == TRUE)
    {
        if (listeners->OnWheel != NULL)
        {
            EvWheel params;
            params.x = (real32_t)event->x;
            params.y = (real32_t)event->y;
            params.dx = 0;
            params.dy = event->direction == GDK_SCROLL_DOWN ? -1 : 1;
            params.dz = 0;

            if (hadjust != NULL)
                params.x += (real32_t)gtk_adjustment_get_value(hadjust);

            if (vadjust != NULL)
                params.y += (real32_t)gtk_adjustment_get_value(vadjust);

            listener_event(listeners->OnWheel, ekEVWHEEL, sender, &params, NULL, OSControl, EvWheel, void);
        }
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_key_event(OSControl *sender, const uint32_t type, GdkEventKey *event, Listener *listener)
{
    cassert_no_null(sender);
    cassert_unref(sender->type == ekGUI_COMPONENT_CUSTOMVIEW, sender);
    if (listener != NULL)
    {
        vkey_t key = ENUM_MAX(vkey_t);
        register guint kval = event->keyval;
        register uint32_t i, n = kNUM_VKEYS;
        register const guint *keys = kVIRTUAL_KEY;

        // Letter events as uppercase
        if (kval >= 97 && kval <= 122)
        {
            kval -= 32;
        }
        else switch(kval)
        {
            case GDK_KEY_KP_Home:
                kval = GDK_KEY_Home;
                break;
            case GDK_KEY_KP_Left:
                kval = GDK_KEY_Left;
                break;
            case GDK_KEY_KP_Up:
                kval = GDK_KEY_Up;
                break;
            case GDK_KEY_KP_Right:
                kval = GDK_KEY_Right;
                break;
            case GDK_KEY_KP_Down:
                kval = GDK_KEY_Down;
                break;
            case GDK_KEY_KP_Page_Up:
                kval = GDK_KEY_Page_Up;
                break;
            case GDK_KEY_KP_Page_Down:
                kval = GDK_KEY_Page_Down;
                break;
            case GDK_KEY_KP_End:
                kval = GDK_KEY_End;
                break;
            case GDK_KEY_KP_Begin:
                kval = GDK_KEY_Begin;
                break;
            case GDK_KEY_KP_Insert:
                kval = GDK_KEY_Insert;
                break;
            case GDK_KEY_KP_Delete:
                kval = GDK_KEY_Delete;
                break;
        }

        for (i = 0; i < n; ++i)
        {
            if (keys[i] == kval)
            {
                key = (vkey_t)i;
                break;
            }
        }

        if (key != ENUM_MAX(vkey_t))
        {
            EvKey params;
            params.key = key;
            listener_event(listener, type, sender, &params, NULL, OSControl, EvKey, void);
            return TRUE;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t _oslistener_key_down(OSControl *sender, GdkEventKey *event, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    return i_key_event(sender, ekEVKEYDOWN, event, listeners->OnKeyDown);
}

/*---------------------------------------------------------------------------*/

bool_t _oslistener_key_up(OSControl *sender, GdkEventKey *event, ViewListeners *listeners)
{
    cassert_no_null(listeners);
    return i_key_event(sender, ekEVKEYUP, event, listeners->OnKeyUp);
}

/*---------------------------------------------------------------------------*/

//void _oslistener_key_flags_changed(const NSView *view, NSEvent *theEvent, ViewListeners *listeners)
//{
//    cassert_no_null(listeners);
//    cassert_no_null(theEvent);
//    if (listeners->is_enabled == YES)
//    {
//        NSUInteger flags;
//        BOOL alt_down, control_down;
//        flags = [theEvent modifierFlags];
//        alt_down = (BOOL)((flags & NSAlternateKeyMask) == NSAlternateKeyMask);
//        control_down = (BOOL)((flags & NSControlKeyMask) == NSControlKeyMask);
//        if (alt_down == YES && listeners->OnKeyDown_listener.object != NULL)
//            i_launch_key_event(view, ekGUI_EVENT_KEY_DOWN, ekGUI_KEY_ALT, &listeners->OnKeyDown_listener);
//        if (alt_down == NO && listeners->OnKeyUp_listener.object != NULL)
//            i_launch_key_event(view, ekGUI_EVENT_KEY_UP, ekGUI_KEY_ALT, &listeners->OnKeyUp_listener);
//        if (control_down == YES && listeners->OnKeyDown_listener.object != NULL)
//            i_launch_key_event(view, ekGUI_EVENT_KEY_DOWN, ekGUI_KEY_CONTROL, &listeners->OnKeyDown_listener);
//        if (control_down == NO && listeners->OnKeyUp_listener.object != NULL)
//            i_launch_key_event(view, ekGUI_EVENT_KEY_UP, ekGUI_KEY_CONTROL, &listeners->OnKeyUp_listener);
//    }
//}

