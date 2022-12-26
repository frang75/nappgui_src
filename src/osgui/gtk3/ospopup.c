/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospopup.c
 *
 */

/* Operating System native popup button */

#include "ospopup.h"
#include "ospopup.inl"
#include "osgui.inl"
#include "osgui_gtk.inl"
#include "oscontrol.inl"
#include "oscombo.inl"
#include "ospanel.inl"
#include "arrpt.h"
#include "cassert.h"
#include "event.h"
#include "font.h"
#include "heap.h"
#include "image.h"
#include "strings.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _ospopup_t
{
    OSControl control;
    uint32_t fsize;
    GtkWidget *button;
    GtkCellRenderer *imgcell;
    GtkCellRenderer *txtcell;
    GtkCssProvider *font;
    bool_t launch_event;
    Listener *OnSelect;
    ArrPt(String) *texts;
    ArrPt(Image) *images;
};

/*---------------------------------------------------------------------------*/

static void i_OnSelect(GtkComboBox *widget, gpointer data)
{
    OSPopUp *popup = (OSPopUp*)data;
    cassert(popup->control.widget == GTK_WIDGET(widget));
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

OSPopUp *ospopup_create(const uint32_t flags)
{
    OSPopUp *popup = heap_new0(OSPopUp);
    GtkWidget *widget = gtk_combo_box_new();
    Font *font = _osgui_create_default_font();
    cassert_unref(flags == ekPOPUP_FLAG, flags);
    /* GtkCellView->GtkBox->GtkToggleButton */
#if GTK_CHECK_VERSION(3, 16, 0)
    popup->button = gtk_bin_get_child(GTK_BIN(widget));
    popup->button = gtk_widget_get_parent(gtk_widget_get_parent(popup->button));
    cassert(str_equ_c((const char_t*)G_OBJECT_TYPE_NAME(popup->button), "GtkToggleButton"));
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
    g_signal_connect (widget, "changed", G_CALLBACK(i_OnSelect), (gpointer)popup);
    _oscontrol_init(&popup->control, ekGUI_TYPE_POPUP, widget, popup->button, TRUE);
    _oscontrol_set_font((OSControl*)popup, font, &popup->font);
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
    _oscontrol_destroy(*(OSControl**)popup);
    heap_delete(popup, OSPopUp);
}

/*---------------------------------------------------------------------------*/

void ospopup_OnSelect(OSPopUp *popup, Listener *listener)
{
    cassert_no_null(popup);
    listener_update(&popup->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void ospopup_elem(OSPopUp *popup, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    uint32_t imgw, imgh;
    gint current;
    cassert_no_null(popup);
    current = gtk_combo_box_get_active(GTK_COMBO_BOX(popup->control.widget));
    _oscombo_elem(GTK_COMBO_BOX(popup->control.widget), op, index, text, image, popup->texts, popup->images, &imgw, &imgh);

    if (imgh < popup->fsize)
        imgh = popup->fsize;

    gtk_cell_renderer_set_fixed_size(popup->txtcell, -1, imgh);
    gtk_cell_renderer_set_fixed_size(popup->imgcell, imgw, imgh);

    if (current == -1)
        current = 0;

    popup->launch_event = FALSE;
    gtk_combo_box_set_active(GTK_COMBO_BOX(popup->control.widget), current);
    popup->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void ospopup_tooltip(OSPopUp *popup, const char_t *text)
{
    cassert_no_null(popup);
    gtk_widget_set_tooltip_text(popup->control.widget, (const gchar*)text);
}

/*---------------------------------------------------------------------------*/

void ospopup_font(OSPopUp *popup, const Font *font)
{
    cassert_no_null(popup);
    _oscontrol_remove_provider(popup->control.widget, popup->font);
    _oscontrol_set_font((OSControl*)popup, font, &popup->font);
    popup->fsize = (uint32_t)(font_size(font) + 2.5f);
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
    popup->launch_event = FALSE;

    if (index != UINT32_MAX)
        gtk_combo_box_set_active(GTK_COMBO_BOX(popup->control.widget), (gint)index);
    else
        gtk_combo_box_set_active(GTK_COMBO_BOX(popup->control.widget), -1);

    popup->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t ospopup_get_selected(const OSPopUp *popup)
{
    gint sel;
    cassert_no_null(popup);
    sel = gtk_combo_box_get_active(GTK_COMBO_BOX(popup->control.widget));
    return sel == -1 ? UINT32_MAX : (uint32_t)sel;
}

/*---------------------------------------------------------------------------*/

void ospopup_bounds(const OSPopUp *popup, const char_t *text, real32_t *width, real32_t *height)
{
    GtkRequisition minsize;
    cassert_no_null(popup);
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
    _ospanel_attach_control(panel, (OSControl*)popup);
}

/*---------------------------------------------------------------------------*/

void ospopup_detach(OSPopUp *popup, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)popup);
}

/*---------------------------------------------------------------------------*/

void ospopup_visible(OSPopUp *popup, const bool_t is_visible)
{
    _oscontrol_set_visible((OSControl*)popup, is_visible);
}

/*---------------------------------------------------------------------------*/

void ospopup_enabled(OSPopUp *popup, const bool_t is_enabled)
{
    _oscontrol_set_enabled((OSControl*)popup, is_enabled);
}

/*---------------------------------------------------------------------------*/

void ospopup_size(const OSPopUp *popup, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)popup, width, height);
}

/*---------------------------------------------------------------------------*/

void ospopup_origin(const OSPopUp *popup, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)popup, x, y);
}

/*---------------------------------------------------------------------------*/

void ospopup_frame(OSPopUp *popup, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl*)popup, x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _ospopup_detach_and_destroy(OSPopUp **popup, OSPanel *panel)
{
    cassert_no_null(popup);
    ospopup_detach(*popup, panel);
    ospopup_destroy(popup);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_ospopup_focus(OSPopUp *popup)
{
    cassert_no_null(popup);
    return popup->button;
}

