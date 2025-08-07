/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospopup.c
 *
 */

/* Operating System native popup button */

#include "osglobals_gtk.inl"
#include "osgui_gtk.inl"
#include "oscontrol_gtk.inl"
#include "oscombo_gtk.inl"
#include "ospanel_gtk.inl"
#include "ospopup_gtk.inl"
#include "oswindow_gtk.inl"
#include "../ospopup.h"
#include "../ospopup.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <draw2d/image.h>
#include <core/arrpt.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _ospopup_t
{
    OSControl control;
    uint32_t fsize;
    GtkWidget *popup;
    GtkWidget *button;
    GtkCellRenderer *imgcell;
    GtkCellRenderer *txtcell;
    Font *font;
    GtkCssProvider *css_padding;
    GtkCssProvider *css_font;
    bool_t launch_event;
    Listener *OnSelect;
    ArrPt(String) *texts;
    ArrPt(Image) *images;
};

/*---------------------------------------------------------------------------*/

static void i_OnSelect(GtkComboBox *widget, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert(popup->popup == GTK_WIDGET(widget));
    if (popup->launch_event == TRUE && popup->OnSelect != NULL)
    {
        EvButton params;
        params.state = ekGUI_ON;
        params.index = (uint32_t)gtk_combo_box_get_active(widget);
        params.text = NULL;
        listener_event(popup->OnSelect, ekGUI_EVENT_POPUP, popup, &params, NULL, OSPopUp, EvButton, void);
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEvent *event, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert_no_null(event);
    cassert_unref(widget == popup->control.widget, widget);
    cassert(GTK_IS_EVENT_BOX(widget) == TRUE);
    if (event->button.button == 1)
    {
        if (_oswindow_mouse_down(cast(popup, OSControl)) == TRUE)
            gtk_combo_box_popup(GTK_COMBO_BOX(popup->popup));
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnEnter(GtkWidget *widget, GdkEvent *event, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert_unref(widget == popup->control.widget, widget);
    unref(event);
    gtk_widget_set_state_flags(popup->button, GTK_STATE_FLAG_PRELIGHT, FALSE);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnLeave(GtkWidget *widget, GdkEvent *event, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert_unref(widget == popup->control.widget, widget);
    unref(event);
    gtk_widget_unset_state_flags(popup->button, GTK_STATE_FLAG_PRELIGHT);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

OSPopUp *ospopup_create(const uint32_t flags)
{
    OSPopUp *popup = heap_new0(OSPopUp);
    GtkWidget *widget = gtk_combo_box_new();
    Font *font = _osgui_create_default_font();
    const char_t *cssobj = _osglobals_css_combobox();
    cassert_unref(flags == ekPOPUP_FLAG, flags);

#if GTK_CHECK_VERSION(3, 16, 0)
    /* GtkCellView->GtkBox->GtkToggleButton */
    popup->button = gtk_bin_get_child(GTK_BIN(widget));
    popup->button = gtk_widget_get_parent(gtk_widget_get_parent(popup->button));
    cassert(GTK_IS_TOGGLE_BUTTON(popup->button));
#else
    popup->button = widget;
#endif

    popup->imgcell = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), popup->imgcell, FALSE);
    gtk_cell_renderer_set_alignment(popup->imgcell, 0.f, .5);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), popup->imgcell, "pixbuf", 0, NULL);
    gtk_cell_renderer_set_padding(popup->imgcell, 0, 0);
    popup->txtcell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(widget), popup->txtcell, TRUE);
    gtk_cell_renderer_set_alignment(popup->txtcell, 0.f, .5);
    gtk_cell_renderer_set_padding(popup->txtcell, 0, 0);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), popup->txtcell, "text", 1, NULL);
    g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(i_OnSelect), (gpointer)popup);

    /*
      Encapsulate the popup widget in an event box
      We need to capture the 'button-press-event' and is not possible doing directly in GtkComboBox
      https://stackoverflow.com/questions/9145741/how-to-detect-mouse-click-in-gtkcombobox
    */
    popup->popup = widget;
    widget = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(widget), popup->popup);
    gtk_event_box_set_above_child(GTK_EVENT_BOX(widget), TRUE);
    gtk_widget_add_events(widget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)popup);
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(i_OnEnter), (gpointer)popup);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(i_OnLeave), (gpointer)popup);
    gtk_widget_show(popup->popup);

    _oscontrol_init(&popup->control, ekGUI_TYPE_POPUP, widget, popup->button, TRUE);
    _oscontrol_update_css_font(popup->popup, cssobj, font, &popup->font, &popup->css_font);
    _oscontrol_update_css_padding(popup->button, _osglobals_css_button(), kPOPUP_VPADDING, kPOPUP_HPADDING, &popup->css_padding);
    popup->fsize = (uint32_t)(font_size(font) + 2.5f);
    font_destroy(&font);
    popup->launch_event = TRUE;
    popup->texts = arrpt_create(String);
    popup->images = arrpt_create(Image);
    return popup;
}

/*---------------------------------------------------------------------------*/

static void i_img_dest(Image **image)
{
    cassert_no_null(image);
    if (*image != NULL)
        image_destroy(image);
}

/*---------------------------------------------------------------------------*/

void ospopup_destroy(OSPopUp **popup)
{
    cassert_no_null(popup);
    cassert_no_null(*popup);
    listener_destroy(&(*popup)->OnSelect);
    arrpt_destroy(&(*popup)->texts, str_destroy, String);
    arrpt_destroy(&(*popup)->images, i_img_dest, Image);
    font_destroy(&(*popup)->font);
    _oscontrol_destroy_css_provider(&(*popup)->css_padding);
    _oscontrol_destroy_css_provider(&(*popup)->css_font);
    _oscontrol_destroy(*dcast(popup, OSControl));
    heap_delete(popup, OSPopUp);
}

/*---------------------------------------------------------------------------*/

void ospopup_OnSelect(OSPopUp *popup, Listener *listener)
{
    cassert_no_null(popup);
    listener_update(&popup->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void ospopup_tooltip(OSPopUp *popup, const char_t *text)
{
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    gtk_widget_set_tooltip_text(popup->popup, cast_const(text, gchar));
}

/*---------------------------------------------------------------------------*/

void ospopup_font(OSPopUp *popup, const Font *font)
{
    const char_t *cssobj = _osglobals_css_combobox();
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    _oscontrol_update_css_font(popup->popup, cssobj, font, &popup->font, &popup->css_font);
    popup->fsize = (uint32_t)(font_size(font) + 2.5f);
}

/*---------------------------------------------------------------------------*/

void ospopup_elem(OSPopUp *popup, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    uint32_t imgw, imgh;
    gint current;
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    current = gtk_combo_box_get_active(GTK_COMBO_BOX(popup->popup));
    _oscombo_elem(GTK_COMBO_BOX(popup->popup), op, index, text, image, popup->texts, popup->images, &imgw, &imgh);

    if (imgh < popup->fsize)
        imgh = popup->fsize;

    gtk_cell_renderer_set_fixed_size(popup->txtcell, -1, imgh);
    gtk_cell_renderer_set_fixed_size(popup->imgcell, imgw, imgh);

    if (current == -1)
        current = 0;

    popup->launch_event = FALSE;
    gtk_combo_box_set_active(GTK_COMBO_BOX(popup->popup), current);
    popup->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void ospopup_list_height(OSPopUp *popup, const uint32_t num_elems)
{
    unref(popup);
    unref(num_elems);
}

/*---------------------------------------------------------------------------*/

void ospopup_selected(OSPopUp *popup, const uint32_t index)
{
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    popup->launch_event = FALSE;

    if (index != UINT32_MAX)
        gtk_combo_box_set_active(GTK_COMBO_BOX(popup->popup), (gint)index);
    else
        gtk_combo_box_set_active(GTK_COMBO_BOX(popup->popup), -1);

    popup->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t ospopup_get_selected(const OSPopUp *popup)
{
    gint sel;
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    sel = gtk_combo_box_get_active(GTK_COMBO_BOX(popup->popup));
    return sel == -1 ? UINT32_MAX : (uint32_t)sel;
}

/*---------------------------------------------------------------------------*/

void ospopup_bounds(const OSPopUp *popup, const char_t *text, real32_t *width, real32_t *height)
{
    GtkRequisition minsize;
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    cassert_no_null(width);
    cassert_no_null(height);
    unref(text);
    gtk_widget_set_size_request(popup->control.widget, -1, -1);
    gtk_widget_get_preferred_size(popup->control.widget, &minsize, NULL);
    *width = (real32_t)minsize.width;
    *height = (real32_t)minsize.height;
}

/*---------------------------------------------------------------------------*/

void ospopup_attach(OSPopUp *popup, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(popup, OSControl));
}

/*---------------------------------------------------------------------------*/

void ospopup_detach(OSPopUp *popup, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(popup, OSControl));
}

/*---------------------------------------------------------------------------*/

void ospopup_visible(OSPopUp *popup, const bool_t visible)
{
    _oscontrol_set_visible(cast(popup, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void ospopup_enabled(OSPopUp *popup, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(popup, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void ospopup_size(const OSPopUp *popup, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(popup, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void ospopup_origin(const OSPopUp *popup, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(popup, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void ospopup_frame(OSPopUp *popup, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(popup);
    _oscontrol_set_frame(cast(popup, OSControl), x, y, width, height);
    gtk_widget_set_size_request(popup->popup, (gint)width, (gint)height);
    gtk_widget_set_size_request(popup->button, (gint)width, (gint)height);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_ospopup_focus_widget(OSPopUp *popup)
{
    cassert_no_null(popup);
    return popup->button;
}
