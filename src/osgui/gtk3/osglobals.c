/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osglobals.c
 *
 */

/* GTK System globals */

#include "osglobals.h"
#include "osglobals.inl"
#include "oscontrol.inl"
#include "cassert.h"
#include "color.h"
#include "heap.h"
#include "image.h"
#include "unicode.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

static bool_t i_IMPOSTOR_MAPPED = FALSE;
static GtkWidget *kWINDOW = NULL;
static GtkWidget *kLABEL = NULL;
static GtkWidget *kENTRY = NULL;
static GtkWidget *kBUTTON = NULL;
static GtkWidget *kCHECK[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static GtkWidget *kSEPARATOR = NULL;
static GtkWidget *kLINKBUTTON = NULL;
static GtkWidget *kPROGRESSBAR = NULL;
static GtkWidget *kSCROLL = NULL;
static GtkWidget *kFRAME = NULL;
static GtkWidget *kTABLE = NULL;
static GtkWidget *kHEADER = NULL;
static GdkPixbuf *kCHECKSBITMAP = NULL;
static uint32_t kCHECK_WIDTH = 0;
static uint32_t kCHECK_HEIGHT = 0;
static uint32_t kENTRY_HEIGHT = 0;
static uint32_t kSCROLL_WIDTH = 0;
static uint32_t kSCROLL_HEIGHT = 0;
static uint32_t kPROGRESS_HEIGHT = 0;
static bool_t kDARK_MODE = FALSE;
static color_t kLABEL_COLOR = 0;
static color_t kVIEW_COLOR = 0;
static color_t kLINE_COLOR = 0;
static color_t kLINK_COLOR = 0;
static color_t kBORD_COLOR = 0;
static color_t kTEXT_COLOR = 0;
static color_t kSELTX_COLOR = 0;
static color_t kHOTTX_COLOR = 0;
static color_t kTEXTBACKDROP_COLOR = 0;
static color_t kSELTXBACKDROP_COLOR = 0;
static color_t kHOTTXBACKDROP_COLOR = 0;

/*---------------------------------------------------------------------------*/

/* Gets the exact margins and size of a widget drawing, based on the generated pixels
Keep 'max_width', 'max_height' as small as possible, depending of widget type */
static void i_widget_margins(GtkWidget *widget, const uint32_t max_width, const uint32_t max_height, uint32_t *x, uint32_t *y, uint32_t *width, uint32_t *height, uint32_t *color)
{
    register uint32_t i, j;
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, max_width, max_height);
    cairo_t *cairo = cairo_create(surface);
    GdkPixbuf *bitmap = NULL;
    uint32_t *buffer = NULL;
    uint32_t x0 = max_width, x1 = 0, y0 = max_height, y1 = 0;
    gtk_widget_draw(widget, cairo);
    bitmap = gdk_pixbuf_get_from_surface(surface, 0, 0, max_width, max_height);
    cassert(gdk_pixbuf_get_rowstride(bitmap) % 4 == 0);
    buffer = (uint32_t*)gdk_pixbuf_get_pixels(bitmap);
    *color = 0;

    for (i = 0; i < max_width; ++i)
    for (j = 0; j < max_height; ++j)
    {
        if (buffer[j * max_width + i] != 0)
        {
            if (i < x0) x0 = i;
            if (i > x1) x1 = i;
            if (j < y0) y0 = j;
            if (j > y1) y1 = j;
            if (*color == 0)
                *color = buffer[j * max_width + i];
        }
    }

    cairo_surface_destroy(surface);
    cairo_destroy(cairo);
    g_object_unref(bitmap);

    *x = x0;
    *y = y0;
    *width = x1 - x0 + 1;
    *height = y1 - y0 + 1;
}

/*---------------------------------------------------------------------------*/
/*
Button states (5)
Normal
Prelight - Hot or mouse over
Backdrop - Background primary window
Insensitive - Disabled
*/
static void i_button_images(cairo_t *cairo, GtkWidget **button, gdouble offset)
{
    cairo_translate(cairo, offset, 0);
    gtk_widget_draw(button[0], cairo);

    cairo_translate(cairo, offset, 0);
    gtk_widget_draw(button[1], cairo);

    cairo_translate(cairo, offset, 0);
    gtk_widget_draw(button[2], cairo);

    cairo_translate(cairo, offset, 0);
    gtk_widget_draw(button[3], cairo);

    cairo_translate(cairo, offset, 0);
    gtk_widget_draw(button[4], cairo);
}

/*---------------------------------------------------------------------------*/

static color_t i_from_gdkcolor(const GdkRGBA *gdkcolor)
{
    return color_rgba(
                (uint8_t)(gdkcolor->red * 255.),
                (uint8_t)(gdkcolor->green * 255.),
                (uint8_t)(gdkcolor->blue * 255.),
                (uint8_t)(gdkcolor->alpha * 255.));
}

/*---------------------------------------------------------------------------*/

static color_t i_color_prop(GtkWidget *widget, const char_t *prop, GtkStateFlags flags)
{
    GValue value = G_VALUE_INIT;
    GdkRGBA *gdkcolor = NULL;
    color_t color = 0;
    GtkStyleContext *c = gtk_widget_get_style_context(widget);
    gtk_style_context_get_property(c, prop, flags, &value);
    gdkcolor = (GdkRGBA*)g_value_get_boxed(&value);
    color = i_from_gdkcolor(gdkcolor);
    g_value_unset(&value);
    return color;
}

/*---------------------------------------------------------------------------*/

/*
static color_t i_backcolor_prop(GtkWidget *widget, GtkStateFlags flags)
{
   GValue value = G_VALUE_INIT;
   GdkRGBA *gdkcolor = NULL;
   color_t color = 0;
   GtkStyleContext *c = gtk_widget_get_style_context(widget);
   gtk_style_context_get_property(c, "background-color", flags, &value);
   gdkcolor = (GdkRGBA*)g_value_get_boxed(&value);
   color = i_from_gdkcolor(gdkcolor);
   g_value_unset(&value);
   return color;
}
*/

/*---------------------------------------------------------------------------*/

/* Trying to consistently get the color of the frame (border, line),
reading the CSS properties, is a kind of impossible mission.
So we draw a frame and get the color. We start with the row in the middle,
to avoid roundness and transparencies in the corner. */
static color_t i_frame_color(void)
{
    register uint32_t i, j, size = 20;
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
    cairo_t *cairo = cairo_create(surface);
    GdkPixbuf *bitmap = NULL;
    uint32_t *buffer = NULL;
    color_t col = 0;
    gtk_widget_draw(kFRAME, cairo);
    bitmap = gdk_pixbuf_get_from_surface(surface, 0, 0, size, size);
    cassert(gdk_pixbuf_get_rowstride(bitmap) % 4 == 0);
    buffer = (uint32_t*)gdk_pixbuf_get_pixels(bitmap);

    for (i = 0; i < size && col == 0; ++i)
    for (j = size/2; j < size && col == 0; ++j)
    {
        if (buffer[j * size + i] != 0)
            col = buffer[j * size + i];
    }

    cairo_surface_destroy(surface);
    cairo_destroy(cairo);
    g_object_unref(bitmap);
    return col;
}

/*---------------------------------------------------------------------------*/

static void i_precompute_colors(void)
{
    real32_t r = 0, g = 0, b = 0;
    cassert(kLABEL_COLOR == 0);
    cassert(i_IMPOSTOR_MAPPED == TRUE);
    kLABEL_COLOR = i_color_prop(kLABEL, "color", GTK_STATE_FLAG_ACTIVE);
    kVIEW_COLOR = i_color_prop(kWINDOW, "background-color", GTK_STATE_FLAG_NORMAL);
    kLINE_COLOR = i_frame_color();
    kLINK_COLOR = i_color_prop(gtk_bin_get_child(GTK_BIN(kLINKBUTTON)), "color", GTK_STATE_FLAG_ACTIVE);
    kBORD_COLOR = kLINE_COLOR;
    kTEXT_COLOR = i_color_prop(kTABLE, "color", GTK_STATE_FLAG_NORMAL);
    kSELTX_COLOR = i_color_prop(kTABLE, "color", GTK_STATE_FLAG_SELECTED);
    kHOTTX_COLOR = i_color_prop(kTABLE, "color", GTK_STATE_FLAG_PRELIGHT);
    kTEXTBACKDROP_COLOR = i_color_prop(kTABLE, "color", GTK_STATE_FLAG_NORMAL | GTK_STATE_FLAG_BACKDROP);
    kSELTXBACKDROP_COLOR = i_color_prop(kTABLE, "color", GTK_STATE_FLAG_SELECTED | GTK_STATE_FLAG_BACKDROP);
    kHOTTXBACKDROP_COLOR = i_color_prop(kTABLE, "color", GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_BACKDROP);

    r = (real32_t)((uint8_t)(kVIEW_COLOR) / 255.f);
    g = (real32_t)((uint8_t)(kVIEW_COLOR >> 8)/255.f);
    b = (real32_t)((uint8_t)(kVIEW_COLOR >> 16)/255.f);
    kDARK_MODE = (.21 * r + .72 * g + .07 * b) < .5 ? TRUE : FALSE;
}

/* Useful debug code to save GdkPixbuf to file  */
/*---------------------------------------------------------------------------*/

/* #include "stream.h"
static gboolean i_encode(const gchar *data, gsize size, GError **error, gpointer stream)
{
   stm_write((Stream*)stream, (const byte_t*)data, (uint32_t)size);
   unref(error);
   return TRUE;
} */

/*---------------------------------------------------------------------------*/

/* static void i_pixbuf_save(GdkPixbuf *pixbuf, const char *type, Stream *stm)
{
   gboolean ok = FALSE;
   ok = gdk_pixbuf_save_to_callback(pixbuf, i_encode, (gpointer)stm, type, NULL, NULL);
   cassert_unref(ok == TRUE, ok);
} */

/*---------------------------------------------------------------------------*/

static void i_precompute_checks(void)
{
    uint32_t mx, my, mwidth, mheight, border = 0;
    cairo_surface_t *surface = NULL;
    cairo_t *cairo = NULL;

    cassert(kCHECK_WIDTH == 0);
    cassert(kCHECK_HEIGHT == 0);
    cassert(kCHECKSBITMAP == NULL);
    cassert(i_IMPOSTOR_MAPPED == TRUE);
    i_widget_margins(kCHECK[0], 30, 30, &mx, &my, &mwidth, &mheight, &border);

    /* Image with checkbox and radio states for drawing in contexts */
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 10 * mwidth, mheight);
    cairo = cairo_create(surface);
    cairo_translate(cairo, - (gdouble)(mx + mwidth), - (gdouble)my);
    i_button_images(cairo, kCHECK, (gdouble)mwidth);
    i_button_images(cairo, kCHECK + 5, (gdouble)mwidth);
    kCHECK_WIDTH = mwidth;
    kCHECK_HEIGHT = mheight;
    kCHECKSBITMAP = gdk_pixbuf_get_from_surface(surface, 0, 0, 10 * mwidth, mheight);

/*
    #include "stream.h"
    #include "image.inl"
    Stream *stm = stm_to_file("/home/fran/Desktop/check.png", NULL);
    i_pixbuf_save(kCHECKSBITMAP, "png", stm);
    stm_close(&stm);
 */

    cairo_surface_destroy(surface);
    cairo_destroy(cairo);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnWindowDamage(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cassert(widget == kWINDOW);
    unref(event);
    unref(data);

    if (i_IMPOSTOR_MAPPED == TRUE)
        return FALSE;

    i_IMPOSTOR_MAPPED = TRUE;
    i_precompute_colors();
    i_precompute_checks();

    {
        real32_t width, height;
        cassert(kENTRY_HEIGHT == 0);
        cassert(kPROGRESS_HEIGHT == 0);
        _oscontrol_widget_size(kENTRY, &width, &height);
        kENTRY_HEIGHT = (uint32_t)height;
        _oscontrol_widget_size(kPROGRESSBAR, &width, &height);
        kPROGRESS_HEIGHT = (uint32_t)height;
        unref(width);
    }

    cassert(kHEADER == NULL);

    {
        GtkTreeViewColumn *column = gtk_tree_view_get_column(GTK_TREE_VIEW(kTABLE), 0);
        kHEADER = gtk_tree_view_column_get_button(GTK_TREE_VIEW_COLUMN(column));
    }

    {
        GtkAllocation halloc, valloc;
        GtkWidget *hscroll = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(kSCROLL));
        GtkWidget *vscroll = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(kSCROLL));

    #if GTK_CHECK_VERSION(3, 20, 0)
        gtk_widget_get_allocated_size(hscroll, &halloc, NULL);
        gtk_widget_get_allocated_size(vscroll, &valloc, NULL);
    #else
        gtk_widget_get_allocation(hscroll, &halloc);
        gtk_widget_get_allocation(vscroll, &valloc);
    #endif

        kSCROLL_WIDTH = valloc.width;
        kSCROLL_HEIGHT = halloc.height;
    }

    gtk_widget_hide(widget);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

/* We create a hidden 'fake' Window to get style information
We didn't find another 'effective' method to consult theme colors, padding, etc. */
static void i_impostor_window(void)
{
   /* Very useful code to get the real full css file applied to entire app */
   /*{
       gchar *theme_name = NULL;
       GtkCssProvider *prov = NULL;
       char *pdata = NULL;
       GtkSettings *settings = gtk_settings_get_default();
       Stream *stm = stm_to_file("/home/fran/Desktop/app_css_export1.css", NULL);
       g_object_get(settings, "gtk-theme-name", &theme_name, NULL);
       prov = gtk_css_provider_get_named(theme_name, NULL);

       if (prov == NULL)
           prov = gtk_css_provider_get_default();

       pdata = gtk_css_provider_to_string(prov);
       stm_writef(stm, pdata);
       stm_close(&stm);
   }*/

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *text = gtk_text_view_new();
    GtkCellRenderer *renderer = NULL;
    GtkTreeViewColumn *column1 = NULL;
    GtkTreeViewColumn *column2 = NULL;
    GtkListStore *store = NULL;
    cassert(kWINDOW == NULL);
    kWINDOW = gtk_offscreen_window_new();
    /*kWINDOW = gtk_window_new(GTK_WINDOW_TOPLEVEL);*/
    kLABEL = gtk_label_new("__LABEL__");
    kENTRY = gtk_entry_new();
    kBUTTON = gtk_button_new();
    kSEPARATOR = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    kLINKBUTTON = gtk_link_button_new_with_label("", "__LINK__");
    kPROGRESSBAR = gtk_progress_bar_new();
    kSCROLL = scroll;
    kFRAME = gtk_frame_new(NULL);
    kTABLE = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(kFRAME), kTABLE);
    renderer = gtk_cell_renderer_text_new();
    column1 = gtk_tree_view_column_new_with_attributes("Column1", renderer, "text", 0, NULL);
    column2 = gtk_tree_view_column_new_with_attributes("Column2", renderer, "text", 0, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    gtk_tree_view_append_column(GTK_TREE_VIEW(kTABLE), column1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(kTABLE), column2);
    gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column1), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN (column1), 100);
    gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column2), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN (column2), 100);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(kTABLE), TRUE);
    gtk_widget_set_size_request(kTABLE, (gint)200, (gint)100);
    store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(kTABLE), GTK_TREE_MODEL(store));
    g_object_unref(store);
    kCHECK[0] = gtk_check_button_new();
    kCHECK[1] = gtk_check_button_new();
    kCHECK[2] = gtk_check_button_new();
    kCHECK[3] = gtk_check_button_new();
    kCHECK[4] = gtk_check_button_new();
    kCHECK[5] = gtk_check_button_new();
    kCHECK[6] = gtk_check_button_new();
    kCHECK[7] = gtk_check_button_new();
    kCHECK[8] = gtk_check_button_new();
    kCHECK[9] = gtk_check_button_new();
    gtk_container_add(GTK_CONTAINER(scroll), text);
    gtk_container_add(GTK_CONTAINER(kWINDOW), vbox);
    gtk_box_pack_start(GTK_BOX(hbox1), kCHECK[0], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), kCHECK[1], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), kCHECK[2], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), kCHECK[3], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), kCHECK[4], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), kCHECK[5], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), kCHECK[6], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), kCHECK[7], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), kCHECK[8], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), kCHECK[9], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), kBUTTON, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), kLABEL, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), kENTRY, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), kSEPARATOR, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), kLINKBUTTON, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), kPROGRESSBAR, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), kFRAME, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[0]), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[1]), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[2]), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[3]), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[4]), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[5]), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[6]), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[7]), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[8]), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(kCHECK[9]), TRUE);
    gtk_widget_set_state_flags(kCHECK[0], (GtkStateFlags)GTK_STATE_FLAG_NORMAL, FALSE);
    gtk_widget_set_state_flags(kCHECK[1], (GtkStateFlags)GTK_STATE_PRELIGHT, FALSE);
    gtk_widget_set_state_flags(kCHECK[2], (GtkStateFlags)GTK_STATE_SELECTED, FALSE);
    gtk_widget_set_state_flags(kCHECK[3], (GtkStateFlags)(GTK_STATE_FLAG_BACKDROP | GTK_STATE_PRELIGHT), FALSE);
    gtk_widget_set_sensitive(kCHECK[4], FALSE);
    gtk_widget_set_state_flags(kCHECK[5], (GtkStateFlags)GTK_STATE_FLAG_NORMAL, FALSE);
    gtk_widget_set_state_flags(kCHECK[6], (GtkStateFlags)GTK_STATE_PRELIGHT, FALSE);
    gtk_widget_set_state_flags(kCHECK[7], (GtkStateFlags)GTK_STATE_SELECTED, FALSE);
    gtk_widget_set_state_flags(kCHECK[8], (GtkStateFlags)(GTK_STATE_FLAG_BACKDROP | GTK_STATE_PRELIGHT), FALSE);
    gtk_widget_set_sensitive(kCHECK[9], FALSE);
    g_signal_connect(kWINDOW, "damage-event", G_CALLBACK(i_OnWindowDamage), NULL);
    gtk_widget_show_all(kWINDOW);

    /*
    Before i_OnWindowDamage some warnings
    Ubuntu 20.04 GTK 3.24.20
    Gdk-WARNING **: 20:44:39.639: ../../../../../gdk/x11/gdkwindow-x11.c:5633 drawable is not a native X11 window
    */
}

/*---------------------------------------------------------------------------*/

device_t osglobals_device(const void *non_used)
{
    cassert(FALSE);
    cassert(non_used == NULL);
    return ekDESKTOP;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_color(const syscolor_t *color)
{
    cassert_no_null(color);
    cassert(kWINDOW != NULL);

    switch (*color) {
    case ekSYSCOLOR_DARKMODE:
        return kDARK_MODE;

    case ekSYSCOLOR_LABEL:
        cassert(kLABEL_COLOR != 0);
        return kLABEL_COLOR;

    case ekSYSCOLOR_VIEW:
        cassert(kVIEW_COLOR != 0);
        return kVIEW_COLOR;

    case ekSYSCOLOR_LINE:
        cassert(kLINE_COLOR != 0);
        return kLINE_COLOR;

    case ekSYSCOLOR_LINK:
        cassert(kLINK_COLOR != 0);
        return kLINK_COLOR;

    case ekSYSCOLOR_BORDER:
        cassert(kBORD_COLOR != 0);
        return kBORD_COLOR;

    cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

void osglobals_resolution(const void *non_used, real32_t *width, real32_t *height)
{
	unref(non_used);
	cassert_no_null(width);
	cassert_no_null(height);
	cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void osglobals_mouse_position(const void *non_used, real32_t *x, real32_t *y)
{
    cassert(FALSE);
	unref(non_used);
	unref(x);
	unref(y);
}

/*---------------------------------------------------------------------------*/

Cursor *osglobals_cursor(const gui_cursor_t cursor, const Image *image, const real32_t hot_x, const real32_t hot_y)
{
    GdkDisplay *display = gdk_display_get_default();
    GdkCursor *gdkcursor = NULL;
    switch (cursor) {
    case ekGUI_CURSOR_ARROW:
        gdkcursor = gdk_cursor_new_for_display(display, GDK_ARROW);
        break;

    case ekGUI_CURSOR_HAND:
        gdkcursor = gdk_cursor_new_for_display(display, GDK_HAND2);
        break;

    case ekGUI_CURSOR_IBEAM:
        gdkcursor = gdk_cursor_new_for_display(display, GDK_XTERM);
        break;

    case ekGUI_CURSOR_CROSS:
        gdkcursor = gdk_cursor_new_for_display(display, GDK_FLEUR);
        break;

    case ekGUI_CURSOR_SIZEWE:
        gdkcursor = gdk_cursor_new_for_display(display, GDK_SB_H_DOUBLE_ARROW);
        break;

    case ekGUI_CURSOR_SIZENS:
        gdkcursor = gdk_cursor_new_for_display(display, GDK_SB_V_DOUBLE_ARROW);
        break;

    case ekGUI_CURSOR_USER:
    {
        const GdkPixbuf *pixbuf = image_native(image);
        gdkcursor = gdk_cursor_new_from_pixbuf(display, (GdkPixbuf*)pixbuf, (gint)hot_x, (gint)hot_y);
        break;
    }

    cassert_default();
    }

    heap_auditor_add("GdkCursor");
    return (Cursor*)gdkcursor;
}

/*---------------------------------------------------------------------------*/

void osglobals_cursor_destroy(Cursor **cursor)
{
    cassert_no_null(cursor);
    cassert_no_null(*cursor);
    g_object_unref((GdkCursor*)*cursor);
    heap_auditor_delete("GdkCursor");
    *cursor = NULL;
}

/*---------------------------------------------------------------------------*/

void osglobals_value(const uint32_t index, void *value)
{
    cassert_no_null(value);
    switch (index) {
    case 0:
        (*(uint32_t*)value) = 0;
        break;

    case 1:
        (*(uint32_t*)value) = 0;
        break;

    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void osglobals_init(void)
{
    i_impostor_window();
}

/*---------------------------------------------------------------------------*/

void osglobals_finish(void)
{
    if (kWINDOW != NULL)
    {
        /* A floating object was finalized. This means that someone
        called g_object_unref() on an object that had only a floating
        reference; the initial floating reference is not owned by anyone
        and must be removed with g_object_ref_sink(). */
        g_object_ref_sink(kWINDOW);
        g_object_unref(kWINDOW);
        kWINDOW = NULL;
        kLABEL = NULL;
        kENTRY = NULL;
        kBUTTON = NULL;
        kSEPARATOR = NULL;
        kLINKBUTTON = NULL;
        kTABLE = NULL;
        kHEADER = NULL;
    }

    if (kCHECKSBITMAP != NULL)
        g_object_unref(kCHECKSBITMAP);
}

/*---------------------------------------------------------------------------*/

static __INLINE GtkWidget *i_entry(void)
{
    cassert(kENTRY != NULL);
    return kENTRY;
}

/*---------------------------------------------------------------------------*/

void osglobals_register_entry(GtkBorder *padding)
{
    if (padding != NULL)
    {
        GtkWidget *entry = i_entry();
        GtkStyleContext *c = gtk_widget_get_style_context(entry);
        gtk_style_context_get_padding(c, GTK_STATE_FLAG_NORMAL, padding);

        if (padding->top == 0 || padding->bottom == 0)
        {
            GtkRequisition s;
            uint32_t fsize;
            gtk_widget_get_preferred_size(entry, &s, NULL);
            fsize = _oscontrol_widget_font_size(entry);
            padding->top = (s.height - fsize - 2) / 2;
            padding->bottom = padding->top;
        }
    }
}

/*---------------------------------------------------------------------------*/

bool_t osglobals_impostor_mapped(void)
{
    return i_IMPOSTOR_MAPPED;
}

/*---------------------------------------------------------------------------*/

GtkStyleContext *osglobals_entry_context(void)
{
    cassert(kENTRY != NULL);
    return gtk_widget_get_style_context(kENTRY);
}

/*---------------------------------------------------------------------------*/

GtkStyleContext *osglobals_button_context(void)
{
    cassert(kBUTTON != NULL);
    return gtk_widget_get_style_context(kBUTTON);
}

/*---------------------------------------------------------------------------*/

GtkStyleContext *osglobals_table_context(void)
{
    cassert(kTABLE != NULL);
    return gtk_widget_get_style_context(kTABLE);
}

/*---------------------------------------------------------------------------*/

color_t osglobals_text_color(void)
{
    cassert(kTEXT_COLOR != 0);
    return kTEXT_COLOR;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_seltext_color(void)
{
    cassert(kSELTX_COLOR != 0);
    return kSELTX_COLOR;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_hottext_color(void)
{
    cassert(kHOTTX_COLOR != 0);
    return kHOTTX_COLOR;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_textbackdrop_color(void)
{
    cassert(kTEXTBACKDROP_COLOR != 0);
    return kTEXTBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_seltextbackdrop_color(void)
{
    cassert(kSELTXBACKDROP_COLOR != 0);
    return kSELTXBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_hottextbackdrop_color(void)
{
    cassert(kHOTTXBACKDROP_COLOR != 0);
    return kHOTTXBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_border_color(void)
{
    return kLINE_COLOR;
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_check_width(void)
{
    cassert(kCHECK_WIDTH != 0);
    return kCHECK_WIDTH;
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_check_height(void)
{
    cassert(kCHECK_HEIGHT != 0);
    return kCHECK_HEIGHT;
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_entry_height(void)
{
    cassert(kENTRY_HEIGHT != 0);
    return kENTRY_HEIGHT;
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_progress_height(void)
{
    cassert(kPROGRESS_HEIGHT != 0);
    return kPROGRESS_HEIGHT;
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_scroll_width(void)
{
    return kSCROLL_WIDTH;
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_scroll_height(void)
{
    return kSCROLL_HEIGHT;
}

/*---------------------------------------------------------------------------*/

GdkPixbuf* osglobals_checks_bitmap(void)
{
    cassert(kCHECKSBITMAP != NULL);
    return kCHECKSBITMAP;
}
