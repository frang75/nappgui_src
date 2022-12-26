/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui_gtk.c
 *
 */

/* Operating system native gui */

#include "osgui.inl"
#include "osgui_gtk.inl"
#include "oscontrol.inl"
#include "osglobals.inl"
#include "ospanel.inl"
#include "osmenu.inl"
#include "oswindow.inl"
#include "arrpt.h"
#include "bstd.h"
#include "cassert.h"
#include "core.h"
#include "dctxh.h"
#include "heap.h"
#include "image.h"
#include "strings.h"
#include "unicode.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

/*---------------------------------------------------------------------------*/

static PangoContext *kPANGO_CONTEXT = NULL;
PangoLayout *kPANGO_LAYOUT = NULL;
real32_t kPANGO_FROM_PIXELS = 0.f;
static GdkCursor *kNS_RESIZE_CURSOR = NULL;
static GdkCursor *kEW_RESIZE_CURSOR = NULL;
static GdkCursor *kDEFAULT_CURSOR = NULL;
static ArrPt(Image) *kREGISTER_ICONS = NULL;

/*---------------------------------------------------------------------------*/

const guint kVIRTUAL_KEY[] =
{
    UINT32_MAX,             /*ekKEY_UNASSIGNED      = 0*/
    GDK_KEY_A,              /*ekKEY_A               = 1*/
    GDK_KEY_S,              /*ekKEY_S               = 2*/
    GDK_KEY_D,              /*ekKEY_D               = 3*/
    GDK_KEY_F,              /*ekKEY_F               = 4*/
    GDK_KEY_H,              /*ekKEY_H               = 5*/
    GDK_KEY_G,              /*ekKEY_G               = 6*/
    GDK_KEY_Z,              /*ekKEY_Z               = 7*/
    GDK_KEY_X,              /*ekKEY_X               = 8*/
    GDK_KEY_C,              /*ekKEY_C               = 9*/

    GDK_KEY_V,              /*ekKEY_V               = 10*/
    GDK_KEY_masculine,      /*ekKEY_BSLASH          = 11*/
    GDK_KEY_B,              /*ekKEY_B               = 12*/
    GDK_KEY_Q,              /*ekKEY_Q               = 13*/
    GDK_KEY_W,              /*ekKEY_W               = 14*/
    GDK_KEY_E,              /*ekKEY_E               = 15*/
    GDK_KEY_R,              /*ekKEY_R               = 16*/
    GDK_KEY_Y,              /*ekKEY_Y               = 17*/
    GDK_KEY_T,              /*ekKEY_T               = 18*/
    GDK_KEY_1,              /*ekKEY_1               = 19*/

    GDK_KEY_2,              /*ekKEY_2               = 20*/
    GDK_KEY_3,              /*ekKEY_3               = 21*/
    GDK_KEY_4,              /*ekKEY_4               = 22*/
    GDK_KEY_6,              /*ekKEY_6               = 23*/
    GDK_KEY_5,              /*ekKEY_5               = 24*/
    GDK_KEY_9,              /*ekKEY_9               = 25*/
    GDK_KEY_7,              /*ekKEY_7               = 26*/
    GDK_KEY_8,              /*ekKEY_8               = 27*/
    GDK_KEY_0,              /*ekKEY_0               = 28*/
    GDK_KEY_ccedilla,       /*ekKEY_RCURLY          = 29*/

    GDK_KEY_O,              /*ekKEY_O               = 30*/
    GDK_KEY_U,              /*ekKEY_U               = 31*/
    GDK_KEY_dead_acute,     /*ekKEY_LCURLY          = 32*/
    GDK_KEY_I,              /*ekKEY_I               = 33*/
    GDK_KEY_P,              /*ekKEY_P               = 34*/
    GDK_KEY_Return,         /*ekKEY_RETURN          = 35*/
    GDK_KEY_L,              /*ekKEY_L               = 36*/
    GDK_KEY_J,              /*ekKEY_J               = 37*/
    GDK_KEY_semicolon,      /*ekKEY_SEMICOLON       = 38*/
    GDK_KEY_K,              /*ekKEY_K               = 39*/

    GDK_KEY_apostrophe,     /*ekKEY_QUEST           = 40*/
    GDK_KEY_comma,          /*ekKEY_COMMA           = 41*/
    GDK_KEY_minus,          /*ekKEY_MINUS           = 42*/
    GDK_KEY_N,              /*ekKEY_N               = 43*/
    GDK_KEY_M,              /*ekKEY_M               = 44*/
    GDK_KEY_period,         /*ekKEY_PERIOD          = 45*/
    GDK_KEY_Tab,            /*ekKEY_TAB             = 46*/
    GDK_KEY_space,          /*ekKEY_SPACE           = 47*/
    GDK_KEY_less,           /*ekKEY_GTLT            = 48*/
    GDK_KEY_BackSpace,      /*ekKEY_BACK            = 49*/

    GDK_KEY_Escape,         /*ekKEY_ESCAPE          = 50*/
    GDK_KEY_F17,            /*ekKEY_F17             = 51*/
    GDK_KEY_KP_Decimal,     /*ekKEY_NUMDECIMAL      = 52*/
    GDK_KEY_KP_Multiply,    /*ekKEY_NUMMULT         = 53*/
    GDK_KEY_KP_Add,         /*ekKEY_NUMADD          = 54*/
    GDK_KEY_Num_Lock,       /*ekKEY_NUMLOCK         = 55*/
    GDK_KEY_KP_Divide,      /*ekKEY_NUMDIV          = 56*/
    GDK_KEY_KP_Enter,       /*ekKEY_NUMRET          = 57*/
    GDK_KEY_KP_Subtract,    /*ekKEY_NUMMINUS        = 58*/
    GDK_KEY_F18,            /*ekKEY_F18             = 59*/

    GDK_KEY_F19,            /*ekKEY_F19             = 60*/
    ' ',                    /*ekKEY_NUMEQUAL        = 61*/
    GDK_KEY_KP_0,           /*ekKEY_NUM0            = 62*/
    GDK_KEY_KP_1,           /*ekKEY_NUM1            = 63*/
    GDK_KEY_KP_2,           /*ekKEY_NUM2            = 64*/
    GDK_KEY_KP_3,           /*ekKEY_NUM3            = 65*/
    GDK_KEY_KP_4,           /*ekKEY_NUM4            = 66*/
    GDK_KEY_KP_5,           /*ekKEY_NUM5            = 67*/
    GDK_KEY_KP_6,           /*ekKEY_NUM6            = 68*/
    GDK_KEY_KP_7,           /*ekKEY_NUM7            = 69*/

    GDK_KEY_KP_8,           /*ekKEY_NUM8            = 70*/
    GDK_KEY_KP_9,           /*ekKEY_NUM9            = 71*/
    GDK_KEY_F5,             /*ekKEY_F5              = 72*/
    GDK_KEY_F6,             /*ekKEY_F6              = 73*/
    GDK_KEY_F7,             /*ekKEY_F7              = 74*/
    GDK_KEY_F3,             /*ekKEY_F3              = 75*/
    GDK_KEY_F8,             /*ekKEY_F8              = 76*/
    GDK_KEY_F9,             /*ekKEY_F9              = 77*/
    GDK_KEY_F11,            /*ekKEY_F11             = 78*/
    GDK_KEY_F13,            /*ekKEY_F13             = 79*/

    GDK_KEY_F16,            /*ekKEY_F16             = 80*/
    GDK_KEY_F14,            /*ekKEY_F14             = 81*/
    GDK_KEY_F10,            /*ekKEY_F10             = 82*/
    GDK_KEY_F12,            /*ekKEY_F12             = 83*/
    GDK_KEY_F15,            /*ekKEY_F15             = 84*/
    GDK_KEY_Page_Up,        /*ekKEY_PAGEUP          = 85*/
    GDK_KEY_Home,           /*ekKEY_HOME            = 86*/
    GDK_KEY_Delete,         /*ekKEY_SUPR            = 87*/
    GDK_KEY_F4,             /*ekKEY_F4              = 88*/
    GDK_KEY_Page_Down,      /*ekKEY_PAGEDOWN        = 89*/

    GDK_KEY_F2,             /*ekKEY_F2              = 90*/
    GDK_KEY_End,            /*ekKEY_END             = 91*/
    GDK_KEY_F1,             /*ekKEY_F1              = 92*/
    GDK_KEY_Left,           /*ekKEY_LEFT            = 93*/
    GDK_KEY_Right,          /*ekKEY_RIGHT           = 94*/
    GDK_KEY_Down,           /*ekKEY_DOWN            = 95*/
    GDK_KEY_Up,             /*ekKEY_UP              = 96*/
    GDK_KEY_Shift_L,        /*ekKEY_LSHIFT          = 97*/
    GDK_KEY_Shift_R,        /*ekKEY_RSHIFT          = 98*/
    GDK_KEY_Control_L,      /*ekKEY_LCTRL           = 99*/

    GDK_KEY_Control_R,      /*ekKEY_RCONTROL        = 100*/
    GDK_KEY_Alt_L,          /*ekKEY_LALT            = 101*/
    GDK_KEY_ISO_Level3_Shift,   /*ekKEY_RALT        = 102*/
    GDK_KEY_Insert,         /*ekKEY_INSERT          = 103*/
    GDK_KEY_exclamdown,     /*ekKEY_EXCLAM          = 104*/
    GDK_KEY_Menu,           /*ekKEY_MENU            = 105*/
    GDK_KEY_Super_L,        /*ekKEY_LWIN            = 106*/
    GDK_KEY_Super_R,        /*ekKEY_RWIN            = 107*/
    GDK_KEY_Caps_Lock,      /*ekKEY_CAPS            = 108*/
    GDK_KEY_ntilde,         /*ekKEY_TILDE           = 109*/

    GDK_KEY_dead_grave,     /*ekKEY_GRAVE           = 110*/
    GDK_KEY_plus            /*ekKEY_PLUS            = 111*/

};

uint32_t kNUM_VKEYS = sizeof(kVIRTUAL_KEY) / sizeof(guint);

/*---------------------------------------------------------------------------*/

void _osgui_start_imp(void)
{
}

/*---------------------------------------------------------------------------*/

void _osgui_finish_imp(void)
{
    osglobals_finish();

    g_object_unref((gpointer)kPANGO_LAYOUT);
    g_object_unref((gpointer)kPANGO_CONTEXT);

    if (kNS_RESIZE_CURSOR != NULL)
    {
        g_object_unref(kNS_RESIZE_CURSOR);
        g_object_unref(kEW_RESIZE_CURSOR);
        g_object_unref(kDEFAULT_CURSOR);
        kNS_RESIZE_CURSOR = NULL;
        kEW_RESIZE_CURSOR = NULL;
        kDEFAULT_CURSOR = NULL;
    }

    if (kREGISTER_ICONS != NULL)
        arrpt_destroy(&kREGISTER_ICONS, NULL, Image);
}

/*---------------------------------------------------------------------------*/

void _osgui_word_size(StringSizeData *data, const char_t *word, real32_t *width, real32_t *height)
{
    int w, h;
    cassert_no_null(data);
    cassert_no_null(width);
    cassert_no_null(height);
    pango_layout_set_text(data->layout, (const char*)word, -1);
    pango_layout_get_pixel_size(data->layout, &w, &h);
    *width = (real32_t)w;
    *height = (real32_t)h;
}

/*---------------------------------------------------------------------------*/

void _osgui_attach_menubar(OSWindow *window, OSMenu *menu)
{
    _osmenu_menubar(menu, window);
    _oswindow_set_menubar(window, menu);
}

/*---------------------------------------------------------------------------*/

void _osgui_detach_menubar(OSWindow *window, OSMenu *menu)
{
    _osmenu_menubar_unlink(menu, window);
    _oswindow_unset_menubar(window, menu);
}

/*---------------------------------------------------------------------------*/

void _osgui_change_menubar(OSWindow *window, OSMenu *previous_menu, OSMenu *new_menu)
{
    unref(window);
    unref(previous_menu);
    unref(new_menu);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

const char_t *_osgui_register_icon(const Image *image)
{
    static char_t ICON_NAME[32];
    if (kREGISTER_ICONS == NULL)
        kREGISTER_ICONS = arrpt_create(Image);

    bstd_sprintf(ICON_NAME, 32, "%p", (void*)image);
    if (arrpt_find(kREGISTER_ICONS, image, Image) == UINT32_MAX)
    {
        uint32_t width = image_width(image);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        gtk_icon_theme_add_builtin_icon(ICON_NAME, (gint)width, (GdkPixbuf*)image_native(image));
        #pragma GCC diagnostic pop
    }

    return ICON_NAME;
}

/*---------------------------------------------------------------------------*/

static void i_cursors(GtkWidget *widget)
{
    GdkDisplay *display = gtk_widget_get_display(widget);
    kNS_RESIZE_CURSOR = gdk_cursor_new_for_display(display, GDK_SB_V_DOUBLE_ARROW);
    kEW_RESIZE_CURSOR = gdk_cursor_new_for_display(display, GDK_SB_H_DOUBLE_ARROW);
    kDEFAULT_CURSOR = gdk_cursor_new_for_display(display, GDK_ARROW);
}

/*---------------------------------------------------------------------------*/

void _osgui_ns_resize_cursor(GtkWidget *widget)
{
    GdkWindow *window = gtk_widget_get_window(widget);

    if (kNS_RESIZE_CURSOR == NULL)
        i_cursors(widget);

    gdk_window_set_cursor(window, kNS_RESIZE_CURSOR);
}

/*---------------------------------------------------------------------------*/

void _osgui_ew_resize_cursor(GtkWidget *widget)
{
    GdkWindow *window = gtk_widget_get_window(widget);

    if (kEW_RESIZE_CURSOR == NULL)
        i_cursors(widget);

    gdk_window_set_cursor(window, kEW_RESIZE_CURSOR);
}

/*---------------------------------------------------------------------------*/

void _osgui_default_cursor(GtkWidget *widget)
{
    GdkWindow *window = gtk_widget_get_window(widget);

    if (kDEFAULT_CURSOR == NULL)
        i_cursors(widget);

    gdk_window_set_cursor(window, kDEFAULT_CURSOR);
}

/*---------------------------------------------------------------------------*/

void _osgui_pre_initialize(void)
{
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    real32_t dpi = (real32_t)pango_cairo_font_map_get_resolution((PangoCairoFontMap*)fontmap);
    kPANGO_CONTEXT = pango_font_map_create_context(fontmap);
    kPANGO_LAYOUT = pango_layout_new(kPANGO_CONTEXT);
    kPANGO_FROM_PIXELS = 1.f / (dpi / 72.f) * (real32_t)PANGO_SCALE;
    kREGISTER_ICONS = NULL;

    /* Set the default font */
    {
        GtkSettings *settings = gtk_settings_get_default();
        GValue dfont = G_VALUE_INIT;
        PangoFontDescription *fdesc;
        g_value_init(&dfont, G_TYPE_STRING);
        g_object_get_property(G_OBJECT(settings), "gtk-font-name", &dfont);
        fdesc = pango_font_description_from_string(g_value_get_string(&dfont));
        dctx_set_default_osfont(NULL, fdesc);
        g_value_unset(&dfont);
        pango_font_description_free(fdesc);
    }

    osglobals_init();
}

/*---------------------------------------------------------------------------*/

bool_t _osgui_is_pre_initialized(void)
{
    return osglobals_impostor_mapped();
}


