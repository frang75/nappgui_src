/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui.c
 *
 */

/* Operating system native gui */

#include "osgui.h"
#include "osgui.inl"
#include "oswindow.inl"
#include <draw2d/draw2d.h>
#include <draw2d/font.h>
#include <draw2d/image.h>
#include <core/arrpt.h>
#include <core/heap.h>
#include <core/strings.h>
#include <osbs/log.h>
#include <sewer/blib.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

static uint32_t i_NUM_USERS = 0;
static Font *i_DEFAULT_FONT = NULL;
static OSWindow *i_MAIN_WINDOW = NULL;
static OSMenu *i_MAIN_MENU = NULL;

static const vkey_t kASCII_VIRTUAL_KEY[] =
    {
        ekKEY_A,
        ekKEY_B,
        ekKEY_C,
        ekKEY_D,
        ekKEY_E,
        ekKEY_F,
        ekKEY_G,
        ekKEY_H,
        ekKEY_I,
        ekKEY_J,
        ekKEY_K,
        ekKEY_L,
        ekKEY_M,
        ekKEY_N,
        ekKEY_O,
        ekKEY_P,
        ekKEY_Q,
        ekKEY_R,
        ekKEY_S,
        ekKEY_T,
        ekKEY_U,
        ekKEY_V,
        ekKEY_W,
        ekKEY_X,
        ekKEY_Y,
        ekKEY_Z};

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
    unref(window);
    if ((window != NULL && window == i_MAIN_WINDOW) || (menu != NULL && menu == i_MAIN_MENU))
    {
        _osgui_detach_menubar(i_MAIN_WINDOW, i_MAIN_MENU);
        i_MAIN_MENU = NULL;
        i_MAIN_WINDOW = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void osgui_redraw_menubar(void)
{
#if defined(__WINDOWS__)
    if (i_MAIN_MENU != NULL && i_MAIN_WINDOW != NULL)
        _osgui_change_menubar(i_MAIN_WINDOW, i_MAIN_MENU, i_MAIN_MENU);
#endif
}

/*---------------------------------------------------------------------------*/

void osgui_message_loop(void)
{
    _osgui_message_loop_imp();
}

/*---------------------------------------------------------------------------*/

bool_t osgui_is_initialized(void)
{
    return _osgui_is_pre_initialized_imp();
}

/*---------------------------------------------------------------------------*/

void osgui_initialize(void)
{
    _osgui_pre_initialize_imp();
}

/*---------------------------------------------------------------------------*/

void osgui_terminate(void)
{
    _oswindow_set_app_terminate();
}

/*---------------------------------------------------------------------------*/

void osgui_set_app(void *app, void *icon)
{
    _oswindow_set_app(app, icon);
}

/*---------------------------------------------------------------------------*/

Font *_osgui_create_default_font(void)
{
    if (i_DEFAULT_FONT == NULL)
        i_DEFAULT_FONT = font_system(font_regular_size(), 0);
    return font_copy(i_DEFAULT_FONT);
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

vkey_t _osgui_vkey_from_text(const char_t *text)
{
    uint32_t vcp = 0;
    uint32_t nb = 0;
    uint32_t cp = unicode_to_u32b(text, ekUTF8, &nb);
    bool_t prev_ampersand = FALSE;

    while (cp != 0)
    {
        if (cp == '&')
        {
            prev_ampersand = !prev_ampersand;
        }
        else if (prev_ampersand == TRUE)
        {
            vcp = cp;
            prev_ampersand = FALSE;
        }
        else
        {
            prev_ampersand = FALSE;
        }

        text += nb;
        cp = unicode_to_u32b(text, ekUTF8, &nb);
    }

    vcp = unicode_toupper(vcp);

    /* Only letters will be transformed into hotkey */
    if (vcp >= 'A' && vcp <= 'Z')
    {
        cassert(sizeof(kASCII_VIRTUAL_KEY) / sizeof(vkey_t) == 'Z' - 'A' + 1);
        return kASCII_VIRTUAL_KEY[vcp - 'A'];
    }

    return ENUM_MAX(vkey_t);
}
