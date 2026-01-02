/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscomwin.c
 *
 */

/* Operating System native common windows */

#include "oscomwin_gtk.inl"
#include "osgui_gtk.inl"
#include "oscontrol_gtk.inl"
#include "../osgui.inl"
#include "../oscomwin.h"
#include <draw2d/color.h>
#include <core/event.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

static char *kFILENAME = NULL;
typedef struct _cdata_t CData;

struct _cdata_t
{
    gint x;
    gint y;
    align_t halign;
    align_t valign;
    GtkWidget *parent;
};

/*---------------------------------------------------------------------------*/

static const char_t *i_oscomwin_file(OSWindow *parent, const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir, const char_t *filename, const bool_t open)
{
    GtkWidget *dialog = NULL;
    GtkFileChooserAction action;
    bool_t dirmode = FALSE;
    gint res;

    if (size == 1 && str_equ_c(ftypes[0], "..DIR..") == TRUE)
    {
        action = open ? GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER : GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;
        dirmode = TRUE;
    }
    else
    {
        action = open ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE;
        dirmode = FALSE;
    }

    dialog = gtk_file_chooser_dialog_new(
        caption,
        parent ? GTK_WINDOW(cast(parent, OSControl)->widget) : NULL,
        action,
        "_Cancel", GTK_RESPONSE_CANCEL,
        open ? "_Open" : "_Save", GTK_RESPONSE_ACCEPT,
        NULL);

    if (str_empty_c(start_dir) == FALSE)
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), cast_const(start_dir, gchar));

    if (open == FALSE)
    {
        if (str_empty_c(filename) == FALSE)
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), cast_const(filename, gchar));
        else
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "");
    }

    if (!dirmode && size > 0)
    {
        uint32_t i = 0;
        for (i = 0; i < size; ++i)
        {
            String *pattern = str_printf("*.%s", ftypes[i]);
            GtkFileFilter *filter = gtk_file_filter_new();
            gtk_file_filter_set_name(filter, (const gchar *)tc(pattern));
            gtk_file_filter_add_pattern(filter, (const gchar *)tc(pattern));
            gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
            str_destroy(&pattern);
            /* g_object_unref(filter); */
        }
    }

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (kFILENAME)
    {
        g_free(kFILENAME);
        kFILENAME = NULL;
    }

    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        kFILENAME = gtk_file_chooser_get_filename(chooser);
    }

    gtk_widget_destroy(dialog);
    return cast_const(kFILENAME, char_t);
}

/*---------------------------------------------------------------------------*/

const char_t *oscomwin_dir(OSWindow *parent, const char_t *caption, const char_t *start_dir)
{
    const char_t *ftypes[] = {"..DIR.."};
    return i_oscomwin_file(parent, caption, ftypes, 1, start_dir, NULL, TRUE);
}

/*---------------------------------------------------------------------------*/

const char_t *oscomwin_file(OSWindow *parent, const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir, const char_t *filename, const bool_t open)
{
    return i_oscomwin_file(parent, caption, ftypes, size, start_dir, filename, open);
}

/*---------------------------------------------------------------------------*/

static void i_OnRealize(GtkWidget *widget, CData *data)
{
    cassert_no_null(data);

    if (data->halign != ekLEFT || data->valign != ekTOP)
    {
        gint width, height;
        gtk_window_get_size(GTK_WINDOW(widget), &width, &height);

        switch (data->halign)
        {
        case ekLEFT:
        case ekJUSTIFY:
            break;
        case ekCENTER:
            data->x -= width / 2;
            break;
        case ekRIGHT:
            data->x -= width;
            break;
        default:
            cassert_default(data->halign);
        }

        switch (data->valign)
        {
        case ekTOP:
        case ekJUSTIFY:
            break;
        case ekCENTER:
            data->y -= height / 2;
            break;
        case ekRIGHT:
            data->y -= height;
            break;
        default:
            cassert_default(data->valign);
        }
    }

    gtk_window_move(GTK_WINDOW(widget), data->x, data->y + 5);
}

/*---------------------------------------------------------------------------*/

void oscomwin_color(OSWindow *parent, const char_t *title, const real32_t x, const real32_t y, const align_t halign, const align_t valign, const color_t current, color_t *colors, const uint32_t n, Listener *OnChange)
{
    GtkWidget *dialog = NULL;
    GtkColorChooser *chooser = NULL;
    CData data;
    gint res = 0;
    GdkRGBA curcol;

    dialog = gtk_color_chooser_dialog_new(title, NULL);
    chooser = GTK_COLOR_CHOOSER(dialog);
    _oscontrol_to_gdkrgba(current, &curcol);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_color_chooser_set_use_alpha(chooser, TRUE);
    gtk_color_chooser_set_rgba(chooser, &curcol);

    if (n > 0)
    {
        uint32_t i, nm = MIN(n, 27);
        GdkRGBA cols[27];
        for (i = 0; i < nm; ++i)
            _oscontrol_to_gdkrgba(colors[i], &cols[i]);
        gtk_color_chooser_add_palette(chooser, GTK_ORIENTATION_HORIZONTAL, 9, (gint)nm, cols);
    }

    data.x = (gint)x;
    data.y = (gint)y;
    data.halign = halign;
    data.valign = valign;
    data.parent = parent ? cast(parent, OSControl)->widget : NULL;
    g_signal_connect(dialog, "realize", G_CALLBACK(i_OnRealize), &data);

    /* gtk_widget_show(dialog); */
    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == -5)
    {
        color_t selcol = 0;
        gtk_color_chooser_get_rgba(chooser, &curcol);
        selcol = _oscontrol_from_gdkrgba(&curcol);
        listener_event(OnChange, ekGUI_EVENT_COLOR, NULL, &selcol, NULL, void, color_t, void);
    }

    gtk_widget_destroy(dialog);
    listener_destroy(&OnChange);
}

/*---------------------------------------------------------------------------*/

void _oscomwin_finish(void)
{
    if (kFILENAME)
    {
        g_free(kFILENAME);
        kFILENAME = NULL;
    }
}
