/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabs.c
 *
 */

/* Operating System native tabctrl */

#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ostabs_gtk.inl"
#include "ospanel_gtk.inl"
#include "../ostabs.h"
#include "../ostabs.inl"
#include <draw2d/image.h>
#include <core/arrst.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

typedef struct _elem_t Elem;

struct _elem_t
{
    GtkWidget *label;
    GtkWidget *image;
};

struct _ostabs_t
{
    OSControl control;
    uint32_t flags;
    Listener *OnSelect;
    bool_t launch_event;
    ArrSt(Elem) *elems;
};

DeclSt(Elem);

/*---------------------------------------------------------------------------*/

static void i_OnSelect(GtkNotebook *widget, GtkWidget *page, guint npage, OSTabs *tabs)
{
    cassert_no_null(tabs);
    cassert_unref(tabs->control.widget == GTK_WIDGET(widget), widget);
    unref(page);
    if (tabs->launch_event == TRUE && tabs->OnSelect != NULL)
    {
        EvButton params;
        params.state = ekGUI_ON;
        params.index = (uint32_t)npage;
        params.text = NULL;
        listener_event(tabs->OnSelect, ekGUI_EVENT_TABS, tabs, &params, NULL, OSTabs, EvButton, void);
    }
}

/*---------------------------------------------------------------------------*/

OSTabs *ostabs_create(const uint32_t flags)
{
    OSTabs *tabs = heap_new0(OSTabs);
    gui_pos_t pos = (gui_pos_t)tabs_get_pos(flags);
    GtkWidget *widget = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(widget), TRUE);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(widget), _oscontrol_position(pos));
    gtk_notebook_set_show_border(GTK_NOTEBOOK(widget), FALSE);
    g_signal_connect(widget, "switch-page", G_CALLBACK(i_OnSelect), tabs);
    tabs->flags = flags;
    _oscontrol_init(&tabs->control, ekGUI_TYPE_TABLIST, widget, widget, TRUE);
    tabs->elems = arrst_create(Elem);
    return tabs;
}

/*---------------------------------------------------------------------------*/

void ostabs_destroy(OSTabs **tabs)
{
    cassert_no_null(tabs);
    cassert_no_null(*tabs);
    listener_destroy(&(*tabs)->OnSelect);
    arrst_destroy(&(*tabs)->elems, NULL, Elem);
    heap_delete(tabs, OSTabs);
}

/*---------------------------------------------------------------------------*/

void ostabs_OnSelect(OSTabs *tabs, Listener *listener)
{
    cassert_no_null(tabs);
    listener_update(&tabs->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void ostabs_tooltip(OSTabs *tabs, const char_t *text)
{
    cassert_no_null(tabs);
    gtk_widget_set_tooltip_text(tabs->control.widget, cast_const(text, gchar));
}

/*---------------------------------------------------------------------------*/

void ostabs_font(OSTabs *tabs, const Font *font)
{
    unref(tabs);
    unref(font);
}

/*---------------------------------------------------------------------------*/

void ostabs_elem(OSTabs *tabs, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(tabs);
    tabs->launch_event = FALSE;
    switch (op)
    {
    case ekCTRL_OP_ADD:
    case ekCTRL_OP_INS:
    {
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        GtkWidget *dummy = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget *label = gtk_label_new(cast_const(text, gchar));
        GtkWidget *img = gtk_image_new();
        GdkPixbuf *pixbuf = NULL;
        Elem *elem = NULL;

        if (image != NULL)
            pixbuf = cast(image_native(image), GdkPixbuf);

        gtk_image_set_from_pixbuf(GTK_IMAGE(img), pixbuf);
        gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), img, FALSE, FALSE, 0);
        gtk_widget_show_all(box);
        gtk_widget_set_size_request(dummy, 0, 0);

        if (op == ekCTRL_OP_ADD)
        {
            elem = arrst_new0(tabs->elems, Elem);
            gtk_notebook_append_page(GTK_NOTEBOOK(tabs->control.widget), dummy, box);
        }
        else
        {
            elem = arrst_insert_n0(tabs->elems, index, 1, Elem);
            gtk_notebook_insert_page(GTK_NOTEBOOK(tabs->control.widget), dummy, box, (gint)index);
        }

        elem->label = label;
        elem->image = img;
        gtk_widget_show_all(tabs->control.widget);
        break;
    }

    case ekCTRL_OP_SET:
    {
        Elem *elem = arrst_get(tabs->elems, index, Elem);
        GdkPixbuf *pixbuf = NULL;

        if (image != NULL)
            pixbuf = cast(image_native(image), GdkPixbuf);

        gtk_label_set_text(GTK_LABEL(elem->label), cast_const(text, gchar));
        gtk_image_set_from_pixbuf(GTK_IMAGE(elem->image), pixbuf);
        break;
    }

    case ekCTRL_OP_DEL:
        arrst_delete(tabs->elems, index, NULL, Elem);
        gtk_notebook_remove_page(GTK_NOTEBOOK(tabs->control.widget), (gint)index);
        break;

    default:
        cassert_default(op);
    }

    tabs->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void ostabs_selected(OSTabs *tabs, const uint32_t index)
{
    cassert_no_null(tabs);
    tabs->launch_event = FALSE;
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs->control.widget), (gint)index);
    tabs->launch_event = FALSE;
}

/*---------------------------------------------------------------------------*/

uint32_t ostabs_get_selected(const OSTabs *tabs)
{
    cassert_no_null(tabs);
    return (uint32_t)gtk_notebook_get_current_page(GTK_NOTEBOOK(tabs->control.widget));
}

/*---------------------------------------------------------------------------*/

void ostabs_bounds(const OSTabs *tabs, const real32_t length, real32_t *width, real32_t *height)
{
    GtkRequisition minsize;
    cassert_no_null(tabs);
    cassert_no_null(width);
    cassert_no_null(height);
    gtk_widget_set_size_request(tabs->control.widget, -1, -1);
    gtk_widget_get_preferred_size(tabs->control.widget, &minsize, NULL);

    switch (tabs_get_pos(tabs->flags))
    {
    case ekTABS_LEFT:
    case ekTABS_RIGHT:
        *width = (real32_t)minsize.width;
        *height = length;
        break;
    case ekTABS_TOP:
    case ekTABS_BOTTOM:
        *width = length;
        *height = (real32_t)minsize.height;
        break;
    default:
        cassert_default(tabs_get_pos(tabs->flags));
    }
}

/*---------------------------------------------------------------------------*/

void ostabs_attach(OSTabs *tabs, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(tabs, OSControl));
}

/*---------------------------------------------------------------------------*/

void ostabs_detach(OSTabs *tabs, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(tabs, OSControl));
}

/*---------------------------------------------------------------------------*/

void ostabs_visible(OSTabs *tabs, const bool_t visible)
{
    _oscontrol_set_visible(cast(tabs, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void ostabs_enabled(OSTabs *tabs, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(tabs, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void ostabs_size(const OSTabs *tabs, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(tabs, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void ostabs_origin(const OSTabs *tabs, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(tabs, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void ostabs_frame(OSTabs *tabs, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(tabs, OSControl), x, y, width, height);
}
