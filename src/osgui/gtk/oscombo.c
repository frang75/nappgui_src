/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscombo.c
 *
 */

/* Operating System native combo box */

#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "oscombo_gtk.inl"
#include "osgui_gtk.inl"
#include "osentry_gtk.inl"
#include "ospanel_gtk.inl"
#include "oswindow_gtk.inl"
#include "../oscombo.h"
#include "../oscombo.inl"
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

struct _oscombo_t
{
    OSControl control;
    uint32_t fsize;
    GtkWidget *combo;
    GtkWidget *button;
    GtkCellRenderer *imgcell;
    GtkCellRenderer *txtcell;
    GtkCssProvider *css_padding_entry;
    GtkCssProvider *css_padding_button;
    Listener *OnSelect;
    uint32_t selected;
    ArrPt(String) *texts;
    ArrPt(Image) *images;
    OSEntry *entry;
};

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEvent *event, OSCombo *combo)
{
    GtkWidget *entry = NULL;
    cassert_no_null(combo);
    cassert_no_null(event);
    cassert_unref(widget == combo->control.widget, widget);
    cassert(GTK_IS_EVENT_BOX(widget) == TRUE);
    if (_oswindow_mouse_down(cast(combo, OSControl)) == FALSE)
        return TRUE;

    if (event->button.button == 1)
    {
        bool_t over = FALSE;
#if GTK_CHECK_VERSION(3, 16, 0)
        over = _oscontrol_widget_mouse_over(combo->button, event);
#else
        over = _oscontrol_widget_mouse_over_right(combo->button, event, 30);
#endif
        if (over == TRUE)
        {
            gtk_combo_box_popup(GTK_COMBO_BOX(combo->combo));
            return FALSE;
        }

        _osentry_deselect(combo->entry);
    }

    entry = _osentry_get_widget(combo->entry);
    gtk_widget_event(entry, cast(event, GdkEvent));
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnMove(GtkWidget *widget, GdkEventMotion *event, OSCombo *combo)
{
    GtkWidget *entry = NULL;
    cassert_no_null(widget);
    cassert_no_null(combo);
    entry = _osentry_get_widget(combo->entry);
    gtk_widget_event(entry, cast(event, GdkEvent));
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnEnter(GtkWidget *widget, GdkEvent *event, OSCombo *combo)
{
    cassert_no_null(combo);
    cassert_unref(widget == combo->control.widget, widget);
    unref(event);
    gtk_widget_set_state_flags(combo->button, GTK_STATE_FLAG_PRELIGHT, FALSE);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnLeave(GtkWidget *widget, GdkEvent *event, OSCombo *combo)
{
    cassert_no_null(combo);
    cassert_unref(widget == combo->control.widget, widget);
    unref(event);
    gtk_widget_unset_state_flags(combo->button, GTK_STATE_FLAG_PRELIGHT);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gchar *i_OnSelect(GtkComboBox *widget, const char *path, OSCombo *combo)
{
    EvButton params;
    const String *str = NULL;
    unref(widget);
    cassert_no_null(combo);
    combo->selected = str_to_u32(path, 10, NULL);
    cassert(combo->selected < arrpt_size(combo->texts, String));
    params.state = ekGUI_ON;
    params.index = combo->selected;
    params.text = NULL;
    listener_event(combo->OnSelect, ekGUI_EVENT_BUTTON, combo, &params, NULL, OSCombo, EvButton, void);
    str = arrpt_get_const(combo->texts, combo->selected, String);
    return g_strdup(tc(str));
}

/*---------------------------------------------------------------------------*/

OSCombo *oscombo_create(const uint32_t flags)
{
    OSCombo *combo = heap_new0(OSCombo);
    GtkWidget *widget = gtk_combo_box_new_with_entry();
    GtkWidget *entry = gtk_bin_get_child(GTK_BIN(widget));
    const char_t *cssentry = _osglobals_css_entry();
    cassert_unref(flags == ekCOMBO_FLAG, flags);
    cassert(GTK_IS_ENTRY(entry) == TRUE);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 0);
    combo->imgcell = gtk_cell_renderer_pixbuf_new();
    combo->txtcell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), combo->imgcell, FALSE);
    gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(widget), combo->txtcell, TRUE);
    gtk_cell_renderer_set_alignment(combo->imgcell, 0.f, .5);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), combo->imgcell, "pixbuf", 0, NULL);
    gtk_cell_renderer_set_alignment(combo->txtcell, 0.f, .5);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), combo->txtcell, "text", 1, NULL);
    _oscontrol_update_css_padding(entry, cssentry, kENTRY_VPADDING, kENTRY_HPADDING, &combo->css_padding_entry);
    g_signal_connect(G_OBJECT(widget), "format-entry-text", G_CALLBACK(i_OnSelect), (gpointer)combo);

#if GTK_CHECK_VERSION(3, 22, 0)
    {
        const char_t *cssbutton = _osglobals_css_button();
        combo->button = gtk_bin_get_child(GTK_BIN(widget));
        cassert(GTK_IS_ENTRY(combo->button));
        combo->button = gtk_widget_get_parent(combo->button);
        cassert(GTK_IS_BOX(combo->button));
        cassert(_oscontrol_num_children(combo->button) == 2);
        combo->button = _oscontrol_get_child(combo->button, 1);
        cassert(GTK_IS_TOGGLE_BUTTON(combo->button));
        _oscontrol_update_css_padding(combo->button, cssbutton, 0, kCOMBO_HPADDING, &combo->css_padding_button);
    }
#else
    combo->button = widget;
#endif

    /*
      Encapsulate the combo widget in an event box
      We need to capture the 'button-press-event' and is not possible doing directly in GtkComboBox
      https://stackoverflow.com/questions/9145741/how-to-detect-mouse-click-in-gtkcombobox
    */
    combo->combo = widget;
    widget = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(widget), combo->combo);
    gtk_event_box_set_above_child(GTK_EVENT_BOX(widget), TRUE);
    gtk_widget_add_events(widget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK);
    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)combo);
    g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)combo);
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(i_OnEnter), (gpointer)combo);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(i_OnLeave), (gpointer)combo);
    gtk_widget_show(combo->combo);

    combo->entry = _osentry_from_combo(entry, cast(combo, OSControl));
    combo->texts = arrpt_create(String);
    combo->images = arrpt_create(Image);
    combo->selected = UINT32_MAX;

    {

        Font *font = _osgui_create_default_font();
        combo->fsize = (uint32_t)(font_size(font) + 2.5f);
        _osentry_font(combo->entry, font);
        font_destroy(&font);
    }

    _oscontrol_init(&combo->control, ekGUI_TYPE_COMBOBOX, widget, entry, TRUE);
    return combo;
}

/*---------------------------------------------------------------------------*/

static void i_img_dest(Image **image)
{
    cassert_no_null(image);
    if (*image != NULL)
        image_destroy(image);
}

/*---------------------------------------------------------------------------*/

void oscombo_destroy(OSCombo **combo)
{
    cassert_no_null(combo);
    cassert_no_null(*combo);
    listener_destroy(&(*combo)->OnSelect);
    arrpt_destroy(&(*combo)->texts, str_destroy, String);
    arrpt_destroy(&(*combo)->images, i_img_dest, Image);
    _oscontrol_destroy_css_provider(&(*combo)->css_padding_entry);
    _oscontrol_destroy_css_provider(&(*combo)->css_padding_button);
    _osentry_destroy(&(*combo)->entry);
    _oscontrol_destroy(*dcast(combo, OSControl));
    heap_delete(combo, OSCombo);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnFilter(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    _osentry_OnFilter(combo->entry, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnChange(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    _osentry_OnChange(combo->entry, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnFocus(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    _osentry_OnFocus(combo->entry, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnSelect(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    listener_update(&combo->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_text(OSCombo *combo, const char_t *text)
{
    cassert_no_null(combo);
    _osentry_text(combo->entry, text);
}

/*---------------------------------------------------------------------------*/

void oscombo_tooltip(OSCombo *combo, const char_t *text)
{
    cassert_no_null(combo);
    _osentry_tooltip(combo->entry, text);
}

/*---------------------------------------------------------------------------*/

void oscombo_font(OSCombo *combo, const Font *font)
{
    cassert_no_null(combo);
    _osentry_font(combo->entry, font);
}

/*---------------------------------------------------------------------------*/

void oscombo_align(OSCombo *combo, const align_t align)
{
    cassert_no_null(combo);
    _osentry_align(combo->entry, align);
}

/*---------------------------------------------------------------------------*/

void oscombo_passmode(OSCombo *combo, const bool_t passmode)
{
    cassert_no_null(combo);
    _osentry_passmode(combo->entry, passmode);
}

/*---------------------------------------------------------------------------*/

void oscombo_editable(OSCombo *combo, const bool_t is_editable)
{
    cassert_no_null(combo);
    _osentry_editable(combo->entry, is_editable);
}

/*---------------------------------------------------------------------------*/

void oscombo_autoselect(OSCombo *combo, const bool_t autoselect)
{
    cassert_no_null(combo);
    _osentry_autoselect(combo->entry, autoselect);
}

/*---------------------------------------------------------------------------*/

void oscombo_select(OSCombo *combo, const int32_t start, const int32_t end)
{
    cassert_no_null(combo);
    _osentry_select(combo->entry, start, end);
}

/*---------------------------------------------------------------------------*/

void oscombo_color(OSCombo *combo, const color_t color)
{
    cassert_no_null(combo);
    _osentry_color(combo->entry, color);
}

/*---------------------------------------------------------------------------*/

void oscombo_bgcolor(OSCombo *combo, const color_t color)
{
    cassert_no_null(combo);
    _osentry_bgcolor(combo->entry, color);
}

/*---------------------------------------------------------------------------*/

void oscombo_elem(OSCombo *combo, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    uint32_t imgw, imgh;
    cassert_no_null(combo);
    cassert(GTK_IS_EVENT_BOX(combo->control.widget));
    cassert(GTK_IS_COMBO_BOX(combo->combo));

    _oscombo_elem(GTK_COMBO_BOX(combo->combo), op, index, text, image, combo->texts, combo->images, &imgw, &imgh);

    if (imgh < combo->fsize)
        imgh = combo->fsize;

    gtk_cell_renderer_set_fixed_size(combo->imgcell, (gint)imgw, (gint)imgh);
    gtk_cell_renderer_set_fixed_size(combo->txtcell, -1, (gint)imgh);

    if (op == ekCTRL_OP_ADD || op == ekCTRL_OP_INS)
    {
        const char_t *etext = _osentry_get_const_text(combo->entry);
        if (str_empty_c(etext) == TRUE)
            _osentry_text(combo->entry, text);
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Is not possible configure the visible items in drop-down menu
 * https://gitlab.gnome.org/GNOME/gtk/issues/158
 */
void oscombo_list_height(OSCombo *combo, const uint32_t num_elems)
{
    unref(combo);
    unref(num_elems);
}

/*---------------------------------------------------------------------------*/

void oscombo_selected(OSCombo *combo, const uint32_t index)
{
    const String *text = NULL;
    cassert_no_null(combo);
    text = arrpt_get_const(combo->texts, index, String);
    combo->selected = index;
    _osentry_text(combo->entry, tc(text));
}

/*---------------------------------------------------------------------------*/

uint32_t oscombo_get_selected(const OSCombo *combo)
{
    cassert_no_null(combo);
    if (combo->selected >= arrpt_size(combo->texts, String))
        cast(combo, OSCombo)->selected = UINT32_MAX;
    return combo->selected;
}

/*---------------------------------------------------------------------------*/

void oscombo_bounds(const OSCombo *combo, const real32_t refwidth, real32_t *width, real32_t *height)
{
    GtkRequisition minsize;
    cassert_no_null(combo);
    cassert_no_null(width);
    cassert_no_null(height);
    gtk_widget_set_size_request(combo->control.widget, -1, -1);
    gtk_widget_get_preferred_size(combo->control.widget, &minsize, NULL);
    *width = refwidth;
    *height = (real32_t)minsize.height;
}

/*---------------------------------------------------------------------------*/

void oscombo_clipboard(OSCombo *combo, const clipboard_t clipboard)
{
    cassert_no_null(combo);
    _osentry_clipboard(combo->entry, clipboard);
}

/*---------------------------------------------------------------------------*/

void oscombo_attach(OSCombo *combo, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(combo, OSControl));
}

/*---------------------------------------------------------------------------*/

void oscombo_detach(OSCombo *combo, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(combo, OSControl));
}

/*---------------------------------------------------------------------------*/

void oscombo_visible(OSCombo *combo, const bool_t visible)
{
    _oscontrol_set_visible(cast(combo, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void oscombo_enabled(OSCombo *combo, const bool_t enabled)
{
    cassert_no_null(combo);
    _osentry_enabled(combo->entry, enabled);
}

/*---------------------------------------------------------------------------*/

void oscombo_size(const OSCombo *combo, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(combo, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void oscombo_origin(const OSCombo *combo, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(combo, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void oscombo_frame(OSCombo *combo, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(combo, OSControl), x, y, width, height);
    gtk_widget_set_size_request(combo->combo, (gint)width, (gint)height);
}

/*---------------------------------------------------------------------------*/

bool_t _oscombo_autosel(const OSCombo *combo)
{
    cassert_no_null(combo);
    return _osentry_get_autoselect(combo->entry);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_oscombo_focus_widget(OSCombo *combo)
{
    cassert_no_null(combo);
    return _osentry_get_widget(combo->entry);
}

/*---------------------------------------------------------------------------*/

bool_t _oscombo_resign_focus(const OSCombo *combo)
{
    cassert_no_null(combo);
    return _osentry_resign_focus(combo->entry);
}

/*---------------------------------------------------------------------------*/

void _oscombo_focus(OSCombo *combo, const bool_t focus)
{
    cassert_no_null(combo);
    _osentry_focus(combo->entry, focus);
}

/*---------------------------------------------------------------------------*/

void _oscombo_elem(GtkComboBox *combo, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image, ArrPt(String) *texts, ArrPt(Image) *images, uint32_t *imgwidth, uint32_t *imgheight)
{
    cassert_no_null(combo);
    cassert_no_null(imgwidth);
    cassert_no_null(imgheight);
    switch (op)
    {
    case ekCTRL_OP_ADD:
    {
        String *str = str_c(text);
        Image *img = image ? image_copy(image) : NULL;
        arrpt_append(texts, str, String);
        arrpt_append(images, img, Image);
        break;
    }

    case ekCTRL_OP_DEL:
        arrpt_delete(texts, index, str_destroy, String);
        arrpt_delete(images, index, i_img_dest, Image);
        break;

    case ekCTRL_OP_INS:
    {
        String *str = str_c(text);
        Image *img = image ? image_copy(image) : NULL;
        arrpt_insert(texts, index, str, String);
        arrpt_insert(images, index, img, Image);
        break;
    }

    case ekCTRL_OP_SET:
    {
        String **str = arrpt_all(texts, String) + index;
        str_upd(str, text);
        if (image != NULL)
        {
            Image **img = arrpt_all(images, Image) + index;
            image_destroy(img);
            *img = image_copy(image);
        }
        break;
    }

    default:
        cassert_default(op);
    }

    {
        GtkListStore *store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
        GtkTreeIter iter;
        String **strs = arrpt_all(texts, String);
        Image **imgs = arrpt_all(images, Image);
        uint32_t i, n = arrpt_size(texts, String);
        uint32_t maxw = 0, maxh = 0;
        cassert(n == arrpt_size(images, Image));
        for (i = 0; i < n; ++i, ++strs, ++imgs)
        {
            const void *pixbuf = NULL;
            if (*imgs != NULL)
            {
                uint32_t w = image_width(*imgs);
                uint32_t h = image_height(*imgs);
                pixbuf = image_native(*imgs);
                if (w > maxw)
                    maxw = w;
                if (h > maxh)
                    maxh = h;
            }

            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, pixbuf, 1, tc(*strs), -1);
        }

        if (maxw > 0)
            maxw += 5;

        *imgwidth = maxw;
        *imgheight = maxh;
        gtk_combo_box_set_model(combo, GTK_TREE_MODEL(store));
        g_object_unref(store);
    }
}
