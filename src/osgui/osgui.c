/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui.c
 *
 */

/* Operating system native gui */

#include "osgui.h"
#include "osgui.inl"

#include "blib.h"
#include "bmem.h"
#include "cassert.h"
#include "font.h"
#include "heap.h"
#include "image.h"
#include "log.h"
#include "draw2d.h"
#include "strings.h"
#include "unicode.h"

#if defined(__GTK3__)
#include "gtk3/oswindow.inl"
#include "gtk3/osgui_gtk.inl"
#endif

static uint32_t i_NUM_USERS = 0;
static Font *i_DEFAULT_FONT = NULL;
static OSWindow *i_MAIN_WINDOW = NULL;
static OSMenu *i_MAIN_MENU = NULL;

/*---------------------------------------------------------------------------*/

static void i_osgui_atexit(void)
{
    if (i_NUM_USERS != 0)
        log_printf("Error! osgui is not properly closed (%d)\n", i_NUM_USERS);
}

/*---------------------------------------------------------------------------*/

void osgui_start(void)
{
    if (i_NUM_USERS == 0)
    {
        draw2d_start();
        _osgui_start_imp();
        blib_atexit(i_osgui_atexit);
    }

    i_NUM_USERS += 1;
}

/*---------------------------------------------------------------------------*/

void osgui_finish(void)
{
    cassert(i_NUM_USERS > 0);
    if (i_NUM_USERS == 1)
    {
        if (i_DEFAULT_FONT != NULL)
            font_destroy(&i_DEFAULT_FONT);

        _osgui_finish_imp();
        draw2d_finish();
    }

    i_NUM_USERS -= 1;
}

/*---------------------------------------------------------------------------*/

void osgui_set_menubar(OSMenu *menu, OSWindow *window)
{
    cassert_no_null(window);
    cassert(i_MAIN_WINDOW == NULL || i_MAIN_WINDOW == window);
    i_MAIN_WINDOW = window;
    if (i_MAIN_MENU != menu)
    {
        if (i_MAIN_MENU != NULL && menu != NULL)
        {
            _osgui_change_menubar(i_MAIN_WINDOW, i_MAIN_MENU, menu);
        }
        else if (i_MAIN_MENU != NULL && menu == NULL)
        {
            _osgui_detach_menubar(i_MAIN_WINDOW, i_MAIN_MENU);
        }
        else
        {
            cassert(i_MAIN_MENU == NULL && menu != NULL);
            _osgui_attach_menubar(i_MAIN_WINDOW, menu);
        }

        i_MAIN_MENU = menu;
    }
}

/*---------------------------------------------------------------------------*/

void osgui_unset_menubar(OSMenu *menu, OSWindow *window)
{
    if ((menu != NULL && i_MAIN_MENU == menu)
        || (window != NULL && i_MAIN_WINDOW == window))
    {
        cassert_no_null(i_MAIN_WINDOW);
        _osgui_detach_menubar(i_MAIN_WINDOW, i_MAIN_MENU);
        i_MAIN_MENU = NULL;
        i_MAIN_WINDOW = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void osgui_redraw_menubar(void)
{
#if defined (__WINDOWS__)
    if (i_MAIN_MENU != NULL && i_MAIN_WINDOW != NULL)
        _osgui_change_menubar(i_MAIN_WINDOW, i_MAIN_MENU, i_MAIN_MENU);
#endif
}

/*---------------------------------------------------------------------------*/

bool_t osgui_message(void *msg)
{
#if defined (__WINDOWS__)
    return _osgui_process_message(msg);
#else
    unref(msg);
    return TRUE;
#endif
}

/*---------------------------------------------------------------------------*/

bool_t osgui_is_initialized(void)
{
#if defined(__GTK3__)
    return _osgui_is_pre_initialized();
#endif

    return FALSE;
}

/*---------------------------------------------------------------------------*/

void osgui_initialize(void)
{
#if defined(__GTK3__)
    _osgui_pre_initialize();
#endif
}

/*---------------------------------------------------------------------------*/

void osgui_terminate(void)
{
#if defined(__GTK3__)
    _oswindow_set_app_terminate();
#endif
}

/*---------------------------------------------------------------------------*/

void osgui_set_app(void *app, void *icon)
{
#if defined(__GTK3__)
    _oswindow_gtk_app((GtkApplication*)app, (GdkPixbuf*)icon);
#endif

    unref(app);
    unref(icon);
}

/*---------------------------------------------------------------------------*/

Font *_osgui_create_default_font(void)
{
    if (i_DEFAULT_FONT == NULL)
        i_DEFAULT_FONT = font_system(font_regular_size(), 0);
    return font_copy(i_DEFAULT_FONT);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_jump_blanks(const char_t *str)
{
    cassert_no_null(str);
    for (;*str != '\0';)
    {
        if (*str == ' ' || *str == '\t' || *str == '\r')
        {
            str += 1;
        }
        else
        {
            return str;
        }
    }

    return str;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_jump_not_blanks(const char_t *str)
{
    cassert_no_null(str);
    for (;*str != '\0';)
    {
        if (*str != ' ' && *str != '\t' && *str != '\r' && *str != '\0' && *str != '\n')
        {
            str += 1;
        }
        else
        {
            return str;
        }
    }

    return str;
}

/*---------------------------------------------------------------------------*/
#define i_WORD_TYPE_END         0
#define i_WORD_TYPE_NEW_LINE    1
#define i_WORD_TYPE_BLANCKS     2
#define i_WORD_TYPE_TEXT        3

static const char_t *i_next_word(const char_t *str, int *word_type)
{
    cassert_no_null(str);
    cassert_no_null(word_type);
    {
        if (*str == '\0')
        {
            *word_type = i_WORD_TYPE_END;
            return NULL;
        }
        else if (*str == '\n')
        {
            *word_type = i_WORD_TYPE_NEW_LINE;
            return str + 1;
        }
    }

    {
        register const char_t *end = i_jump_blanks(str);
        if (end != str)
        {
            *word_type = i_WORD_TYPE_BLANCKS;
            return end;
        }
    }

    {
        register const char_t *end = i_jump_not_blanks(str);
        cassert(end != str);
        *word_type = i_WORD_TYPE_TEXT;
        return end;
    }
}

/*---------------------------------------------------------------------------*/

static void i_new_line(StringSizeData *data, real32_t *current_width, real32_t *current_height, real32_t *current_width_without_spaces, uint32_t *num_lines, real32_t *width, real32_t *height)
{
    cassert_no_null(current_width);
    cassert_no_null(current_height);
    cassert_no_null(current_width_without_spaces);
    cassert_no_null(num_lines);
    cassert_no_null(width);
    cassert_no_null(height);

    if (*current_width_without_spaces == 0.f)
    {
        real32_t word_width = 0.f, word_height = 0.f;
        _osgui_word_size(data, "A", &word_width, &word_height);
        *current_width_without_spaces = word_width;
        *current_height = word_height;
    }
    else
    {
        cassert(*current_height > 0.f);
    }

    if (*current_width_without_spaces > *width)
        *width = *current_width_without_spaces;
    *height += *current_height;

    *current_width = 0.f;
    *current_width_without_spaces = 0.f;
    *current_height = 0.f;
    *num_lines += 1;
}

/*---------------------------------------------------------------------------*/

static real32_t i_ceil(const real32_t n)
{
    int32_t in = (int32_t)n;
    if (n == (real32_t)in)
    {
        return (real32_t)in;
    }

    return (real32_t)(in + 1);
}

/*---------------------------------------------------------------------------*/

void _osgui_text_bounds(StringSizeData *data, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height)
{
    uint32_t num_lines = 0;
    real32_t ref_width = refwidth > 0.f ? refwidth : 1e8f;
    real32_t current_width = 0.f, current_height = 0.f, current_width_without_spaces = 0.f;
    register const char_t *ctext = text;
    cassert_no_null(width);
    cassert_no_null(height);
    *width = 0.f;
    *height = 0.f;

    while (ctext != NULL)
    {
        const char_t *next_text = NULL;
        int word_type = 0;

        next_text = i_next_word(ctext, &word_type);

        switch (word_type) {

        case i_WORD_TYPE_END:
            if (current_width > .01f || num_lines == 0)
                i_new_line(data, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
            break;

        case i_WORD_TYPE_NEW_LINE:
            i_new_line(data, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
            break;

        case i_WORD_TYPE_BLANCKS:
            if (current_width_without_spaces > 0.f)
            {
                char_t word[128];
                real32_t word_width = 0.f, word_height = 0.f;
                register uint32_t size = (uint32_t)(next_text - ctext);
                cassert(next_text > ctext);
                str_copy_cn(word, 128, ctext, size);
                word[size] = '\0';
                _osgui_word_size(data, word, &word_width, &word_height);
                if (current_width + word_width <= ref_width)
                {
                    current_width += word_width;
                    if (word_height > current_height)
                        current_height = word_height;
                }
                else
                {
                    i_new_line(data, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
                }
            }
            break;

        case i_WORD_TYPE_TEXT:
        {
            char_t word[128];
            real32_t word_width = 0.f, word_height = 0.f;
            register uint32_t size = (uint32_t)(next_text - ctext);
            cassert(next_text > ctext);
            cassert(word_type == i_WORD_TYPE_TEXT);
            str_copy_cn(word, 128, ctext, size);
            word[size] = '\0';
            _osgui_word_size(data, word, &word_width, &word_height);

            if (current_width + word_width <= ref_width)
            {
                current_width += word_width;
                if (word_height > current_height)
                    current_height = word_height;
            }
            else
            {
                i_new_line(data, &current_width, &current_height, &current_width_without_spaces, &num_lines, width, height);
                current_width = word_width;
                current_height = word_height;
            }

            current_width_without_spaces = current_width;
            break;
        }

        cassert_default();
        }

        ctext = next_text;
    }

    *width = i_ceil(*width);
    *height = i_ceil(*height);
}

/*---------------------------------------------------------------------------*/

const char_t *_osgui_component_type(const gui_type_t type)
{
    switch (type)
    {
        case ekGUI_TYPE_LABEL:
            return "OSLabel";
        case ekGUI_TYPE_BUTTON:
            return "OSButton";
        case ekGUI_TYPE_POPUP:
            return "OSPopUp";
        case ekGUI_TYPE_EDITBOX:
            return "OSEdit";
        case ekGUI_TYPE_COMBOBOX:
            return "OSComboBox";
        case ekGUI_TYPE_SLIDER:
            return "OSSlider";
        case ekGUI_TYPE_UPDOWN:
            return "OSUpDown";
        case ekGUI_TYPE_PROGRESS:
            return "OSProgress";
        case ekGUI_TYPE_TEXTVIEW:
            return "OSTextView";
        case ekGUI_TYPE_TABLEVIEW:
            return "OSTableView";
        case ekGUI_TYPE_TREEVIEW:
            return "OSTreeView";
        case ekGUI_TYPE_BOXVIEW:
            return "OSBoxView";
        case ekGUI_TYPE_SPLITVIEW:
            return "OSSplitView";
        case ekGUI_TYPE_CUSTOMVIEW:
            return "OSCView";
        case ekGUI_TYPE_PANEL:
            return "OSView";
        case ekGUI_TYPE_LINE:
            return "OSLine";
        case ekGUI_TYPE_HEADER:
            return "OSHeader";
        case ekGUI_TYPE_TOOLBAR:
            return "OSToolbar";
        case ekGUI_TYPE_WINDOW:
            return "OSWindow";
        cassert_default();
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

bool_t _osgui_button_text_allowed(const uint32_t flags)
{
    switch (button_get_type(flags))
    {
        case ekBUTTON_PUSH:
        case ekBUTTON_CHECK2:
        case ekBUTTON_CHECK3:
        case ekBUTTON_RADIO:
        case ekBUTTON_HEADER:
            return TRUE;
        case ekBUTTON_FLAT:
        case ekBUTTON_FLATGLE:
            return FALSE;
        cassert_default();
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t _osgui_button_image_allowed(const uint32_t flags)
{
    switch (button_get_type(flags))
    {
        case ekBUTTON_CHECK2:
        case ekBUTTON_CHECK3:
        case ekBUTTON_RADIO:
            return FALSE;
        case ekBUTTON_PUSH:
        case ekBUTTON_FLAT:
        case ekBUTTON_FLATGLE:
            return TRUE;
        cassert_default();
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*/

gui_size_t _osgui_size_font(const real32_t font_size)
{
    if (font_size > font_regular_size() - 0.1f)
        return ekGUI_SIZE_REGULAR;
    if (font_size > font_small_size() - 0.1f)
        return ekGUI_SIZE_SMALL;
    return ekGUI_SIZE_MINI;
}

/*---------------------------------------------------------------------------*/

Image *_osgui_scale_image(const Image *image, const Font *font)
{
    gui_size_t size = _osgui_size_font(font_size(font));
    switch (size) {
    case ekGUI_SIZE_REGULAR:
        return image_scale(image, UINT32_MAX, 16);
    case ekGUI_SIZE_SMALL:
        return image_scale(image, UINT32_MAX, 12);
    case ekGUI_SIZE_MINI:
        return image_scale(image, UINT32_MAX, 8);
    cassert_default();
    }

    return NULL;
}

