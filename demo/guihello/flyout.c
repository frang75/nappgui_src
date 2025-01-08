/* Flyout window */

#include "flyout.h"
#include <gui/guiall.h>

typedef struct _flyout_t FlyOut;

struct _flyout_t
{
    Window *parent;
    Window *flywin;
    Menu *menu;
    TextView *text;
    GuiControl *edit;
    uint32_t align;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_flyout(FlyOut **flyout)
{
    cassert_no_null(flyout);
    cassert_no_null(*flyout);
    window_destroy(&(*flyout)->flywin);
    if ((*flyout)->menu != NULL)
        menu_destroy(&(*flyout)->menu);
    heap_delete(flyout, FlyOut);
}

/*---------------------------------------------------------------------------*/

static Window *i_create_flywin(void)
{
    uint32_t nrows = 4;
    Layout *layout = layout_create(2, nrows);
    Panel *panel = panel_create();
    Window *window = window_create(ekWINDOW_RETURN | ekWINDOW_ESC);
    uint32_t i;

    for (i = 0; i < nrows; ++i)
    {
        char_t text[64];
        Label *label = label_create();
        Slider *slider = slider_create();
        bstd_sprintf(text, sizeof(text), "Flyout control %d", i);
        label_text(label, text);
        layout_label(layout, label, 0, i);
        layout_slider(layout, slider, 1, i);

        if (i < nrows - 1)
            layout_vmargin(layout, i, 5);
    }

    layout_hmargin(layout, 0, 5);
    layout_margin(layout, 10);
    layout_skcolor(layout, kCOLOR_RED);
    panel_layout(panel, layout);
    window_panel(window, panel);
    return window;
}

/*---------------------------------------------------------------------------*/

static void i_OnIdleLaunch(FlyOut *flyout, Event *e)
{
    /* Edit control bounds in window coordinates */
    R2Df frame = window_control_frame(flyout->parent, flyout->edit);
    /* Top-Left edit control in screen coordinates */
    V2Df pos = window_client_to_screen(flyout->parent, frame.pos);
    /* Flyout window size */
    S2Df size = window_get_size(flyout->flywin);

    switch (flyout->align)
    {
    case 0:
        pos.y += frame.size.height;
        break;
    case 1:
        pos.y -= size.height;
        break;
    case 2:
        pos.x -= size.width - frame.size.width;
        pos.y += frame.size.height;
        break;
    case 3:
        pos.x -= size.width - frame.size.width;
        pos.y -= size.height;
        break;
    }

    /* Position in screen coordinates */
    window_origin(flyout->flywin, pos);
    window_overlay(flyout->flywin, flyout->parent);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(FlyOut *flyout, Event *e)
{
    gui_OnIdle(listener(flyout, i_OnIdleLaunch, FlyOut));
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Layout *i_controls_layout(FlyOut *flyout)
{
    Layout *layout = layout_create(5, 1);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Slider *slider = slider_create();
    Edit *edit = edit_create();
    button_text(button1, "Push Button");
    button_text(button2, "...");
    button_OnClick(button2, listener(flyout, i_OnClick, FlyOut));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 4, 0);
    layout_slider(layout, slider, 1, 0);
    layout_edit(layout, edit, 3, 0);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_hexpand(layout, 2);
    flyout->edit = guicontrol(edit);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnAlign(FlyOut *flyout, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    cassert_no_null(flyout);
    flyout->align = p->index;
}

/*---------------------------------------------------------------------------*/

static Layout *i_align_layout(FlyOut *flyout)
{
    Layout *layout = layout_create(4, 1);
    Button *button1 = button_radio();
    Button *button2 = button_radio();
    Button *button3 = button_radio();
    Button *button4 = button_radio();
    button_text(button1, "Left-top");
    button_text(button2, "Left-bottom");
    button_text(button3, "Right-top");
    button_text(button4, "Right-bottom");
    button_OnClick(button1, listener(flyout, i_OnAlign, FlyOut));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 1, 0);
    layout_button(layout, button3, 2, 0);
    layout_button(layout, button4, 3, 0);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_hmargin(layout, 2, 5);
    button_state(button1, ekGUI_ON);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnMenu(FlyOut *flyout, Event *e)
{
    const EvMenu *p = event_params(e, EvMenu);
    textview_writef(flyout->text, p->text);
    textview_writef(flyout->text, "\n");
}

/*---------------------------------------------------------------------------*/

static void i_OnDown(FlyOut *flyout, Event *e)
{
    const EvMouse *p = event_params(e, EvMouse);
    if (p->button == ekGUI_MOUSE_RIGHT && p->tag != UINT32_MAX)
    {
        uint32_t i = 0;

        if (flyout->menu != NULL)
            menu_destroy(&flyout->menu);

        flyout->menu = menu_create();

        for (i = 0; i < 4; ++i)
        {
            char_t text[64];
            MenuItem *item = menuitem_create();
            bstd_sprintf(text, sizeof(text), "Item %d Option %d", p->tag + 1, i + 1);
            menuitem_text(item, text);
            menuitem_OnClick(item, listener(flyout, i_OnMenu, FlyOut));
            menu_item(flyout->menu, item);
        }

        {
            V2Df pos = gui_mouse_pos();
            menu_launch(flyout->menu, pos);
        }
    }
}

/*---------------------------------------------------------------------------*/

static Layout *i_listbox_layout(FlyOut *flyout)
{
    Layout *layout = layout_create(2, 1);
    ListBox *list = listbox_create();
    TextView *text = textview_create();
    listbox_add_elem(list, "Item 1", NULL);
    listbox_add_elem(list, "Item 2", NULL);
    listbox_add_elem(list, "Item 3", NULL);
    listbox_add_elem(list, "Item 4", NULL);
    listbox_OnDown(list, listener(flyout, i_OnDown, FlyOut));
    layout_listbox(layout, list, 0, 0);
    layout_textview(layout, text, 1, 0);
    layout_hmargin(layout, 0, 20);
    flyout->text = text;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(FlyOut *flyout)
{
    Layout *layout1 = layout_create(1, 5);
    Layout *layout2 = i_align_layout(flyout);
    Layout *layout3 = i_controls_layout(flyout);
    Layout *layout4 = i_listbox_layout(flyout);
    Label *label1 = label_multiline();
    Label *label2 = label_create();
    label_text(label1, "A flyout window will be show when you press ... button\nPress [ESC] or [RETURN] to close the flyout window");
    label_text(label2, "A popup menu will be show when right click in ListBox");
    layout_label(layout1, label1, 0, 0);
    layout_layout(layout1, layout2, 0, 1);
    layout_layout(layout1, layout3, 0, 2);
    layout_label(layout1, label2, 0, 3);
    layout_layout(layout1, layout4, 0, 4);
    layout_vmargin(layout1, 0, 10);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    layout_vmargin(layout1, 3, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static FlyOut *i_flyout(Window *window)
{
    FlyOut *flyout = heap_new(FlyOut);
    flyout->parent = window;
    flyout->flywin = i_create_flywin();
    flyout->menu = NULL;
    flyout->align = 0;
    return flyout;
}

/*---------------------------------------------------------------------------*/

Panel *flyout_window(Window *window)
{
    FlyOut *flyout = i_flyout(window);
    Layout *layout = i_layout(flyout);
    Panel *panel = panel_create();
    panel_data(panel, &flyout, i_destroy_flyout, FlyOut);
    panel_layout(panel, layout);
    return panel;
}
