/* NAppGUI GUI Basics */

#include "nappgui.h"
#include "labels.h"
#include "buttons.h"
#include "sliders.h"
#include "editor.h"
#include "fontx.h"
#include "form.h"
#include "seltext.h"
#include "popcom.h"
#include "listboxes.h"
#include "textviews.h"
#include "table.h"
#include "flyout.h"
#include "guibind.h"
#include "ipinput.h"
#include "layoutbind.h"
#include "baslayout.h"
#include "hotkeys.h"
#include "splits.h"
#include "sublayout.h"
#include "subpanel.h"
#include "tabstops.h"
#include "vpadding.h"
#include "modalwin.h"
#include "multilayout.h"
#include "dynlay.h"
#include "scrollpanel.h"
#include "res_guihello.h"

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    Layout *layout;
};

/*---------------------------------------------------------------------------*/

static void i_set_panel(App *app, const uint32_t index)
{
    Panel *panel = NULL;
    Button *defbutton = NULL;
    switch (index)
    {
    case 0:
        panel = labels_single_line();
        break;
    case 1:
        panel = labels_multi_line();
        break;
    case 2:
        panel = labels_mouse_over();
        break;
    case 3:
        panel = buttons_basics(&defbutton);
        break;
    case 4:
        panel = popup_combo();
        break;
    case 5:
        panel = listboxes();
        break;
    case 6:
        panel = form_basic(app->window);
        break;
    case 7:
        panel = seltext(app->window);
        break;
    case 8:
        panel = editor(app->window);
        break;
    case 9:
        panel = sliders();
        break;
    case 10:
        panel = vpadding();
        break;
    case 11:
        panel = tabstops();
        break;
    case 12:
        panel = textviews();
        break;
    case 13:
        panel = table_view();
        break;
    case 14:
        panel = split_panel();
        break;
    case 15:
        panel = modal_windows(app->window);
        break;
    case 16:
        panel = flyout_window(app->window);
        break;
    case 17:
        panel = hotkeys(app->window);
        break;
    case 18:
        panel = guibind();
        break;
    case 19:
        panel = layoutbind();
        break;
    case 20:
        panel = basic_layout();
        break;
    case 21:
        panel = sublayouts();
        break;
    case 22:
        panel = subpanels();
        break;
    case 23:
        panel = multilayouts();
        break;
    case 24:
        panel = scrollpanel();
        break;
    case 25:
        panel = dynlay_panel();
        break;
    case 26:
        panel = ip_input(app->window);
        break;
    case 27:
        panel = font_x_scale();
        break;
    }

    layout_panel_replace(app->layout, panel, 1, 0);

    if (app->window != NULL)
        window_defbutton(app->window, defbutton);
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    if (p->index != UINT32_MAX)
    {
        i_set_panel(app, p->index);
        window_update(app->window);
    }
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(2, 1);
    ListBox *list = listbox_create();
    app->layout = layout;
    listbox_size(list, s2df(180, 256));
    listbox_add_elem(list, "Labels single line", NULL);
    listbox_add_elem(list, "Labels multi line", NULL);
    listbox_add_elem(list, "Labels mouse sensitive", NULL);
    listbox_add_elem(list, "Buttons", NULL);
    listbox_add_elem(list, "PopUp Combo", NULL);
    listbox_add_elem(list, "ListBoxes", NULL);
    listbox_add_elem(list, "Form", NULL);
    listbox_add_elem(list, "Text select", NULL);
    listbox_add_elem(list, "Text editor", NULL);
    listbox_add_elem(list, "Sliders", NULL);
    listbox_add_elem(list, "Vertical padding", NULL);
    listbox_add_elem(list, "Tabstops", NULL);
    listbox_add_elem(list, "TextViews", NULL);
    listbox_add_elem(list, "TableView", NULL);
    listbox_add_elem(list, "SplitViews", NULL);
    listbox_add_elem(list, "Modal Windows", NULL);
    listbox_add_elem(list, "Flyout Windows", NULL);
    listbox_add_elem(list, "Hotkeys", NULL);
    listbox_add_elem(list, "Data Binding", NULL);
    listbox_add_elem(list, "Struct Binding", NULL);
    listbox_add_elem(list, "Basic Layout", NULL);
    listbox_add_elem(list, "SubLayouts", NULL);
    listbox_add_elem(list, "Subpanels", NULL);
    listbox_add_elem(list, "Multi-Layouts", NULL);
    listbox_add_elem(list, "Scroll panel", NULL);
    listbox_add_elem(list, "Dynamic layouts", NULL);
    listbox_add_elem(list, "IP Input", NULL);
    listbox_add_elem(list, "Font x-scale", NULL);
    listbox_select(list, 0, TRUE);
    listbox_OnSelect(list, listener(app, i_OnSelect, App));
    layout_listbox(layout, list, 0, 0);
    i_set_panel(app, 0);
    panel_layout(panel, layout);
    layout_valign(layout, 0, 0, ekTOP);
    layout_valign(layout, 1, 0, ekTOP);
    layout_margin(layout, 10);
    layout_hmargin(layout, 0, 10);
    return panel;
}

/*---------------------------------------------------------------------------*/

static Window *i_window(App *app)
{
    Panel *panel = i_panel(app);
    Window *window = window_create(ekWINDOW_STD | ekWINDOW_ESC | ekWINDOW_RETURN);
    window_panel(window, panel);
    window_title(window, "NAppGUI GUI Basics");
    return window;
}

/*---------------------------------------------------------------------------*/

static void i_modal_window(Window *parent, const char_t *message)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 1);
    Label *label = label_create();
    Window *window = window_create(ekWINDOW_STD | ekWINDOW_ESC | ekWINDOW_RETURN);
    label_text(label, message);
    layout_label(layout, label, 0, 0);
    layout_margin(layout, 10);
    panel_layout(panel, layout);
    window_panel(window, panel);
    window_title(window, "Message");

    {
        V2Df pos = window_get_origin(parent);
        S2Df s1 = window_get_size(parent);
        S2Df s2 = window_get_size(window);
        pos.x += (s1.width - s2.width) / 2;
        pos.y += (s1.height - s2.height) / 2;
        window_origin(window, pos);
    }

    window_modal(window, parent);
    window_destroy(&window);
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    const EvWinClose *p = event_params(e, EvWinClose);
    bool_t *close = event_result(e, bool_t);
    cassert_no_null(app);

    switch (p->origin)
    {
    case ekGUI_CLOSE_ESC:
        i_modal_window(app->window, "Pressed [ESC] key. Press [ESC] again or [RETURN] to exit.");
        osapp_finish();
        break;

    case ekGUI_CLOSE_INTRO:
        i_modal_window(app->window, "Pressed [RETURN] key. Press [ESC] or [RETURN] again to continue.");
        *close = FALSE;
        break;

    case ekGUI_CLOSE_BUTTON:
        osapp_finish();
        break;

    case ekGUI_CLOSE_DEACT:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = heap_new0(App);
    gui_respack(res_guihello_respack);
    gui_language("");
    app->window = i_window(app);
    window_origin(app->window, v2df(500.f, 200.f));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
