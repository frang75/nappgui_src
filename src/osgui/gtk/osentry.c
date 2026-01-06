/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osentry.c
 *
 */

/* GtkEntry, GtkTextView, GtkComboBox entry management */
/* GtkTextView is used as multiline EditBox in this context, not as "complete" TextView */

#include "osglobals_gtk.inl"
#include "osentry_gtk.inl"
#include "oscontrol_gtk.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

struct _osentry_t
{
    OSControl *control;
    String *type;
    GtkWidget *widget;
    GtkTextBuffer *buffer;
    GtkCssProvider *css_font;
    GtkCssProvider *css_color;
    GtkCssProvider *css_bgcolor;
    Font *font;
    color_t ccolor;
    bool_t launch_event;
    bool_t in_validate;
    bool_t autoselect;
    int32_t select_start;
    int32_t select_end;
    String *text_update;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
};

/*---------------------------------------------------------------------------*/

static void i_iter(GtkTextBuffer *buffer, const int32_t pos, GtkTextIter *iter)
{
    if (pos >= 0)
    {
        gtk_text_buffer_get_start_iter(buffer, iter);
        gtk_text_iter_forward_chars(iter, (gint)pos);
    }
    else
    {
        gtk_text_buffer_get_end_iter(buffer, iter);
    }
}

/*---------------------------------------------------------------------------*/

/*
 * If the function returns FALSE it is automatically removed from the list
 * of event sources and will not be called again.
 */
static gboolean i_OnIdleUpdate(OSEntry *entry)
{
    cassert_no_null(entry);
    if (entry->text_update != NULL)
    {
        bool_t prev = entry->launch_event;
        entry->launch_event = FALSE;
        _osentry_text(entry, tc(entry->text_update));
        entry->launch_event = prev;
        str_destroy(&entry->text_update);
    }

    if (entry->select_start != INT32_MAX)
    {
        cassert(entry->select_start >= -1);
        cassert(entry->select_end >= -1);

        if (GTK_IS_ENTRY(entry->widget) == TRUE)
        {
            gtk_editable_select_region(GTK_EDITABLE(entry->widget), (gint)entry->select_start, (gint)entry->select_end);
        }
        else
        {
            GtkTextIter st_iter;
            GtkTextIter ed_iter;
            cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
            i_iter(entry->buffer, entry->select_start, &st_iter);
            i_iter(entry->buffer, entry->select_end, &ed_iter);
            gtk_text_buffer_select_range(entry->buffer, &st_iter, &ed_iter);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(entry->widget), &st_iter, 0, TRUE, 0, 0);
        }

        entry->select_start = INT32_MAX;
        entry->select_end = INT32_MAX;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gchar *i_get_text(const OSEntry *entry, bool_t *allocated, uint32_t *nchars)
{
    cassert_no_null(entry);
    cassert_no_null(allocated);
    if (GTK_IS_ENTRY(entry->widget) == TRUE)
    {
        *allocated = FALSE;
        if (nchars != NULL)
            *nchars = (uint32_t)gtk_entry_get_text_length(GTK_ENTRY(entry->widget));
        return cast(gtk_entry_get_text(GTK_ENTRY(entry->widget)), gchar);
    }
    else
    {
        GtkTextIter st, end;
        cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
        gtk_text_buffer_get_start_iter(entry->buffer, &st);
        gtk_text_buffer_get_end_iter(entry->buffer, &end);
        *allocated = TRUE;

        if (nchars != NULL)
            *nchars = (uint32_t)gtk_text_iter_get_offset(&end);

        return gtk_text_buffer_get_text(entry->buffer, &st, &end, FALSE);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnFilter(OSEntry *entry, const uint32_t cpos, const int32_t len)
{
    cassert_no_null(entry);
    if (entry->launch_event == TRUE && gtk_widget_is_sensitive(entry->widget) && entry->OnFilter != NULL)
    {
        EvText params;
        EvTextFilter result;
        bool_t allocated = FALSE;
        params.text = cast_const(i_get_text(entry, &allocated, NULL), char_t);
        params.cpos = cpos;
        params.len = len;
        result.apply = FALSE;
        result.text[0] = '\0';
        result.cpos = UINT32_MAX;
        listener_event_imp(entry->OnFilter, ekGUI_EVENT_TXTFILTER, cast(entry->control, void), cast(&params, void), cast(&result, void), tc(entry->type), "EvText", "EvTextFilter");

        if (allocated)
            g_free(cast(params.text, gchar));

        if (result.apply == TRUE)
        {
            if (GTK_IS_ENTRY(entry->widget) == TRUE)
            {
                bool_t prev = entry->launch_event;
                entry->launch_event = FALSE;
                _osentry_text(entry, result.text);
                entry->launch_event = prev;
            }
            else
            {
                /*
                 * In GtkTextView, we cannot change the GtkTextBuffer inside of
                 * "insert-text", "delete-range" events
                 */
                cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
                str_upd(&entry->text_update, result.text);
                entry->select_start = (gint)cpos;
                entry->select_end = entry->select_start;
                g_idle_add((GSourceFunc)i_OnIdleUpdate, entry);
            }
        }

        if (result.cpos != UINT32_MAX)
        {
            entry->select_start = (gint)result.cpos;
            entry->select_end = entry->select_start;
            g_idle_add((GSourceFunc)i_OnIdleUpdate, entry);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnInsert(GtkEditable *editable, gchar *new_text, gint new_text_length, gpointer position, OSEntry *entry)
{
    cassert_no_null(position);
    unref(editable);
    unref(new_text);
    i_OnFilter(entry, (uint32_t)*cast(position, gint), (int32_t)new_text_length);
}

/*---------------------------------------------------------------------------*/

static void i_OnDelete(GtkEditable *editable, gint start_pos, gint end_pos, OSEntry *entry)
{
    unref(editable);
    i_OnFilter(entry, (uint32_t)start_pos, (int32_t)(start_pos - end_pos));
}

/*---------------------------------------------------------------------------*/

static void i_OnBufferInsert(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, OSEntry *entry)
{
    gint position = gtk_text_iter_get_offset(location);
    unref(buffer);
    unref(text);
    i_OnFilter(entry, (uint32_t)position, (int32_t)len);
}

/*---------------------------------------------------------------------------*/

static void i_OnBufferDelete(GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end, OSEntry *entry)
{
    gint stpos = gtk_text_iter_get_offset(start);
    unref(buffer);
    unref(end);
    i_OnFilter(entry, (uint32_t)stpos, INT32_MAX);
}

/*---------------------------------------------------------------------------*/

static OSEntry *i_create(GtkWidget *widget, OSControl *control, const char_t *type)
{
    OSEntry *entry = heap_new0(OSEntry);
    entry->control = control;
    entry->type = str_c(type);
    entry->widget = widget;
    entry->ccolor = kCOLOR_TRANSPARENT;
    entry->launch_event = TRUE;
    entry->in_validate = FALSE;
    entry->autoselect = FALSE;
    entry->select_start = INT32_MAX;
    entry->select_end = INT32_MAX;

    if (GTK_IS_ENTRY(widget) == TRUE)
    {
        g_signal_connect_after(G_OBJECT(widget), "insert-text", G_CALLBACK(i_OnInsert), (gpointer)entry);
        g_signal_connect_after(G_OBJECT(widget), "delete-text", G_CALLBACK(i_OnDelete), (gpointer)entry);
    }
    else
    {
        cassert(GTK_IS_TEXT_VIEW(widget) == TRUE);
        entry->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        g_signal_connect_after(G_OBJECT(entry->buffer), "insert-text", G_CALLBACK(i_OnBufferInsert), (gpointer)entry);
        g_signal_connect_after(G_OBJECT(entry->buffer), "delete-range", G_CALLBACK(i_OnBufferDelete), (gpointer)entry);
    }

    return entry;
}

/*---------------------------------------------------------------------------*/

OSEntry *_osentry_from_entry(GtkWidget *widget, OSControl *control)
{
    return i_create(widget, control, "OSEdit");
}

/*---------------------------------------------------------------------------*/

OSEntry *_osentry_from_combo(GtkWidget *widget, OSControl *control)
{
    return i_create(widget, control, "OSCombo");
}

/*---------------------------------------------------------------------------*/

void _osentry_destroy(OSEntry **entry)
{
    cassert_no_null(entry);
    cassert_no_null(*entry);

    /* Remove all pending idle funcions */
    while (g_idle_remove_by_data(*entry))
    {
    }

    str_destroy(&(*entry)->type);
    str_destopt(&(*entry)->text_update);
    listener_destroy(&(*entry)->OnFilter);
    listener_destroy(&(*entry)->OnChange);
    listener_destroy(&(*entry)->OnFocus);
    ptr_destopt(font_destroy, &(*entry)->font, Font);
    _oscontrol_destroy_css_provider(&(*entry)->css_font);
    _oscontrol_destroy_css_provider(&(*entry)->css_color);
    _oscontrol_destroy_css_provider(&(*entry)->css_bgcolor);
    heap_delete(entry, OSEntry);
}

/*---------------------------------------------------------------------------*/

void _osentry_OnFilter(OSEntry *entry, Listener *listener)
{
    cassert_no_null(entry);
    listener_update(&entry->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void _osentry_OnChange(OSEntry *entry, Listener *listener)
{
    cassert_no_null(entry);
    listener_update(&entry->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void _osentry_OnFocus(OSEntry *entry, Listener *listener)
{
    cassert_no_null(entry);
    listener_update(&entry->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void _osentry_text(OSEntry *entry, const char_t *text)
{
    cassert_no_null(entry);
    entry->launch_event = FALSE;

    if (GTK_IS_ENTRY(entry->widget) == TRUE)
    {
        gtk_entry_set_text(GTK_ENTRY(entry->widget), cast_const(text, gchar));
    }
    else
    {
        cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);

#if GTK_CHECK_VERSION(3, 22, 0)
        {
            uint32_t fstyle = font_style(entry->font);
            String *markup = NULL;
            if ((fstyle & ekFSTRIKEOUT) && (fstyle & ekFUNDERLINE))
                markup = str_printf("<u><s>%s</s></u>", text);
            else if (fstyle & ekFSTRIKEOUT)
                markup = str_printf("<s>%s</s>", text);
            else if (fstyle & ekFUNDERLINE)
                markup = str_printf("<u>%s</u>", text);

            if (markup != NULL)
            {
                GtkTextIter iter;
                gtk_text_buffer_set_text(entry->buffer, "", -1);
                gtk_text_buffer_get_start_iter(entry->buffer, &iter);
                gtk_text_buffer_insert_markup(entry->buffer, &iter, tc(markup), (gint)str_len(markup));
                str_destroy(&markup);
            }
            else
            {
                gtk_text_buffer_set_text(entry->buffer, cast_const(text, gchar), -1);
            }
        }
#else
        gtk_text_buffer_set_text(entry->buffer, cast_const(text, gchar), -1);
#endif
    }

    {
        int32_t len = (int32_t)str_len_c(text);
        _osentry_select(entry, len, len);
    }

    entry->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_cssobj(GtkWidget *widget)
{
    if (GTK_IS_ENTRY(widget) == TRUE)
        return _osglobals_css_entry();
    else
        return _osglobals_css_textview();
}

/*---------------------------------------------------------------------------*/

void _osentry_font(OSEntry *entry, const Font *font)
{
    cassert_no_null(entry);
    _oscontrol_update_css_font(entry->widget, i_cssobj(entry->widget), font, &entry->font, &entry->css_font);
}

/*---------------------------------------------------------------------------*/

void _osentry_align(OSEntry *entry, const align_t align)
{
    cassert_no_null(entry);
    if (GTK_IS_ENTRY(entry->widget) == TRUE)
    {
        gfloat a = 0.f;
        switch (align)
        {
        case ekLEFT:
            a = 0.f;
            break;
        case ekCENTER:
        case ekJUSTIFY:
            a = .5f;
            break;
        case ekRIGHT:
            a = .99f;
            break;
        default:
            cassert_default(align);
        }

        gtk_entry_set_alignment(GTK_ENTRY(entry->widget), a);
    }
    else
    {
        GtkJustification just = _oscontrol_justification(align);
        cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
        gtk_text_view_set_justification(GTK_TEXT_VIEW(entry->widget), just);
    }
}

/*---------------------------------------------------------------------------*/

void _osentry_passmode(OSEntry *entry, const bool_t passmode)
{
    cassert_no_null(entry);
    if (GTK_IS_ENTRY(entry->widget) == TRUE)
    {
        gtk_entry_set_visibility(GTK_ENTRY(entry->widget), (gboolean)!passmode);
    }
    else
    {
        /* GtkTextView doesn't support passmode */
        cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
    }
}

/*---------------------------------------------------------------------------*/

void _osentry_editable(OSEntry *entry, const bool_t is_editable)
{
    cassert_no_null(entry);
    if (GTK_IS_ENTRY(entry->widget) == TRUE)
    {
        gtk_editable_set_editable(GTK_EDITABLE(entry->widget), (gboolean)is_editable);
    }
    else
    {
        cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(entry->widget), (gboolean)is_editable);
    }
}

/*---------------------------------------------------------------------------*/

void _osentry_autoselect(OSEntry *entry, const bool_t autoselect)
{
    cassert_no_null(entry);
    entry->autoselect = autoselect;
}

/*---------------------------------------------------------------------------*/

void _osentry_select(OSEntry *entry, const int32_t start, const int32_t end)
{
    cassert_no_null(entry);
    entry->select_start = start;
    entry->select_end = end;
    if (gtk_widget_has_focus(entry->widget) == TRUE)
        g_idle_add((GSourceFunc)i_OnIdleUpdate, entry);
}

/*---------------------------------------------------------------------------*/

void _osentry_tooltip(OSEntry *entry, const char_t *text)
{
    cassert_no_null(entry);
    gtk_widget_set_tooltip_text(entry->widget, cast_const(text, gchar));
}

/*---------------------------------------------------------------------------*/

static void i_set_color(OSEntry *entry)
{
    const char_t *cssobj = NULL;
    color_t color = kCOLOR_DEFAULT;
    cassert_no_null(entry);
    cssobj = i_cssobj(entry->widget);
    color = gtk_widget_get_sensitive(entry->widget) ? entry->ccolor : kCOLOR_DEFAULT;

    /* We need the text CSS, not TextView CSS */
    if (GTK_IS_TEXT_VIEW(entry->widget) == TRUE)
        cssobj = _osglobals_css_textview_text();

    _oscontrol_update_css_color(entry->widget, cssobj, color, &entry->css_color);
}

/*---------------------------------------------------------------------------*/

void _osentry_color(OSEntry *entry, const color_t color)
{
    cassert_no_null(entry);
    entry->ccolor = color;
    i_set_color(entry);
}

/*---------------------------------------------------------------------------*/

void _osentry_bgcolor(OSEntry *entry, const color_t color)
{
    const char_t *cssobj = _osglobals_css_entry();
    cassert_no_null(entry);
    _oscontrol_update_css_bgcolor(entry->widget, cssobj, color, &entry->css_bgcolor);
}

/*---------------------------------------------------------------------------*/

void _osentry_clipboard(OSEntry *entry, const clipboard_t clipboard)
{
    cassert_no_null(entry);
    if (GTK_IS_ENTRY(entry->widget) == TRUE)
    {
        switch (clipboard)
        {
        case ekCLIPBOARD_COPY:
            gtk_editable_copy_clipboard(GTK_EDITABLE(entry->widget));
            break;
        case ekCLIPBOARD_CUT:
            gtk_editable_cut_clipboard(GTK_EDITABLE(entry->widget));
            break;
        case ekCLIPBOARD_PASTE:
            gtk_editable_paste_clipboard(GTK_EDITABLE(entry->widget));
            break;
        default:
            cassert_default(clipboard);
        }
    }
    else
    {
        GtkClipboard *system_board = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
        switch (clipboard)
        {
        case ekCLIPBOARD_COPY:
            gtk_text_buffer_copy_clipboard(entry->buffer, system_board);
            break;
        case ekCLIPBOARD_CUT:
            gtk_text_buffer_cut_clipboard(entry->buffer, system_board, TRUE);
            break;
        case ekCLIPBOARD_PASTE:
            gtk_text_buffer_paste_clipboard(entry->buffer, system_board, NULL, TRUE);
            break;
        default:
            cassert_default(clipboard);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _osentry_enabled(OSEntry *entry, const bool_t enabled)
{
    cassert_no_null(entry);
    gtk_widget_set_sensitive(entry->widget, (gboolean)enabled);
    i_set_color(entry);
}

/*---------------------------------------------------------------------------*/

static void i_deselect_widget(GtkWidget *widget, GtkTextBuffer *buffer)
{
    if (GTK_IS_ENTRY(widget) == TRUE)
    {
        gtk_editable_select_region(GTK_EDITABLE(widget), -1, -1);
    }
    else
    {
        GtkTextIter desel;
        cassert(GTK_IS_TEXT_VIEW(widget) == TRUE);
        gtk_text_buffer_get_start_iter(buffer, &desel);
        gtk_text_buffer_select_range(buffer, &desel, &desel);
    }
}

/*---------------------------------------------------------------------------*/

static void i_cache_select(OSEntry *entry)
{
    cassert_no_null(entry);
    if (entry->select_start == INT32_MAX)
    {
        if (GTK_IS_ENTRY(entry->widget) == TRUE)
        {
            gint start_pos, end_pos;
            gtk_editable_get_selection_bounds(GTK_EDITABLE(entry->widget), &start_pos, &end_pos);
            entry->select_start = (int32_t)start_pos;
            entry->select_end = (int32_t)end_pos;
        }
        else
        {
            GtkTextIter start, end;
            cassert(GTK_IS_TEXT_VIEW(entry->widget) == TRUE);
            gtk_text_buffer_get_selection_bounds(entry->buffer, &start, &end);
            entry->select_start = (int32_t)gtk_text_iter_get_offset(&start);
            entry->select_end = (int32_t)gtk_text_iter_get_offset(&end);
        }

        i_deselect_widget(entry->widget, entry->buffer);
    }
}

/*---------------------------------------------------------------------------*/

bool_t _osentry_resign_focus(OSEntry *entry)
{
    bool_t lost_focus = TRUE;
    cassert_no_null(entry);
    entry->in_validate = TRUE;
    if (entry->launch_event == TRUE && gtk_widget_is_sensitive(entry->widget) && entry->OnChange != NULL)
    {
        EvText params;
        bool_t allocated = FALSE;
        uint32_t nchars = 0;
        /* The OnChange event can lost focus (p.e: launching a modal window) */
        i_cache_select(entry);
        params.text = cast_const(i_get_text(entry, &allocated, &nchars), char_t);
        params.cpos = (uint32_t)entry->select_start;
        params.len = (int32_t)nchars;
        listener_event_imp(entry->OnChange, ekGUI_EVENT_TXTCHANGE, cast(entry->control, void), cast(&params, void), cast(&lost_focus, void), tc(entry->type), "EvText", "bool_t");
        if (allocated)
            g_free(cast(params.text, gchar));
    }

    entry->in_validate = FALSE;
    return lost_focus;
}

/*---------------------------------------------------------------------------*/

void _osentry_focus(OSEntry *entry, const bool_t focus)
{
    cassert_no_null(entry);
    if (focus == FALSE)
        i_cache_select(entry);

    if (entry->OnFocus != NULL)
    {
        bool_t params = focus;
        listener_event_imp(entry->OnFocus, ekGUI_EVENT_FOCUS, cast(entry->control, void), cast(&params, void), NULL, tc(entry->type), "bool_t", "void");
    }

    if (focus == TRUE)
    {
        if (entry->autoselect == TRUE)
        {
            entry->select_start = 0;
            entry->select_end = -1;
        }
        else if (entry->select_start == INT32_MAX)
        {
            entry->select_start = 0;
            entry->select_end = 0;
        }

        g_idle_add((GSourceFunc)i_OnIdleUpdate, entry);
    }
}

/*---------------------------------------------------------------------------*/

void _osentry_deselect(OSEntry *entry)
{
    cassert_no_null(entry);
    i_deselect_widget(entry->widget, entry->buffer);
    entry->select_start = INT32_MAX;
    entry->select_end = INT32_MAX;
}

/*---------------------------------------------------------------------------*/

const char_t *_osentry_get_const_text(const OSEntry *entry)
{
    cassert_no_null(entry);
    cassert(GTK_IS_ENTRY(entry->widget) == TRUE);
    return cast(gtk_entry_get_text(GTK_ENTRY(entry->widget)), char_t);
}

/*---------------------------------------------------------------------------*/

const Font *_osentry_get_font(const OSEntry *entry)
{
    cassert_no_null(entry);
    return entry->font;
}

/*---------------------------------------------------------------------------*/

bool_t _osentry_get_autoselect(const OSEntry *entry)
{
    cassert_no_null(entry);
    return entry->autoselect;
}

/*---------------------------------------------------------------------------*/

GtkCssProvider *_osentry_get_bgcolor(const OSEntry *entry)
{
    cassert_no_null(entry);
    return entry->css_bgcolor;
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osentry_get_widget(const OSEntry *entry)
{
    cassert_no_null(entry);
    return entry->widget;
}
