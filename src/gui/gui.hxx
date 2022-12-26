/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gui.hxx
 * https://nappgui.com/en/gui/gui.html
 *
 */

/* Graphics User Interface */

#ifndef __GUI_HXX__
#define __GUI_HXX__

#include "guictx.hxx"
#include "gui.def"

typedef enum _gui_notif_t
{
    ekGUI_NOTIF_LANGUAGE        = 1,
    ekGUI_NOTIF_WIN_DESTROY,
    ekGUI_NOTIF_MENU_DESTROY
} gui_notif_t;

typedef struct _control_t Control;
typedef struct _label_t Label;
typedef struct _button_t Button;
typedef struct _popup_t PopUp;
typedef struct _edit_t Edit;
typedef struct _combo_t Combo;
typedef struct _listbox_t ListBox;
typedef struct _updown_t UpDown;
typedef struct _slider_t Slider;
typedef struct _progress_t Progress;
typedef struct _view_t View;
typedef struct _textview_t TextView;
typedef struct _imageview_t ImageView;
typedef struct _tableview_t TableView;
typedef struct _splitview_t SplitView;
typedef struct _layout_t Layout;
typedef struct _cell_t Cell;
typedef struct _panel_t Panel;
typedef struct _window_t Window;
typedef struct _menu_t Menu;
typedef struct _menu_item_t MenuItem;

typedef ResPack*(*FPtr_respack)(const char_t *locale);

DeclSt(FPtr_respack);

#endif
