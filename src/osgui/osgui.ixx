/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui.ixx
 *
 */

/* Operating system native gui */

#ifndef __OSGUI_IXX__
#define __OSGUI_IXX__

#include "osgui.hxx"

typedef struct _oscolumn_t OSColumn;
typedef struct _strsize_data_t StringSizeData;
typedef struct _evscroll_t EvScroll;

struct _evscroll_t
{
    real32_t pos;
};


/* For revision */
enum gui_toolbar_button_type_t
{
    ekGUI_TOOLBAR_BUTTON_TYPE_TEXT,
    ekGUI_TOOLBAR_BUTTON_TYPE_IMAGE,
    ekGUI_TOOLBAR_BUTTON_TYPE_DONE,
    ekGUI_TOOLBAR_BUTTON_TYPE_PLAY,
    ekGUI_TOOLBAR_BUTTON_TYPE_PAUSE,
    ekGUI_TOOLBAR_BUTTON_TYPE_FLEXIBLE_SPACE
};

enum gui_toolbar_button_style_t
{
    ekGUI_TOOLBAR_BUTTON_STYLE_PLAIN,
    ekGUI_TOOLBAR_BUTTON_STYLE_BORDERED,
    ekGUI_TOOLBAR_BUTTON_STYLE_DONE
};

enum gui_indicator_style_t
{
    ekGUI_INDICATOR_STYLE_CONTINUOUS,
    ekGUI_INDICATOR_STYLE_DISCRETE
};

enum gui_indicator_state_t
{
    ekGUI_INDICATOR_STATE_OK,
    ekGUI_INDICATOR_STATE_WARNING,
    ekGUI_INDICATOR_STATE_CRITICAL
};

DeclPt(OSControl);
DeclPt(OSMenuItem);

#endif

