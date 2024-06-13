/* WebView Hello World */

#include <nappgui.h>

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    Panel *tabpanel;
    uint32_t tabid;
    WebView *web1;
    WebView *web2;
};

/*---------------------------------------------------------------------------*/

static void i_web_navigate(WebView *view, const uint32_t index)
{
    switch (index)
    {
    case 0:
        webview_navigate(view, "https://nappgui.com");
        break;
    case 1:
        webview_navigate(view, "https://google.com");
        break;
    case 2:
        webview_navigate(view, "https://youtube.com");
        break;
    case 3:
        webview_navigate(view, "https://facebook.com");
        break;
    case 4:
        webview_navigate(view, "https://instagram.com");
        break;
    case 5:
        webview_navigate(view, "https://twitter.com");
        break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnTab(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    panel_visible_layout(app->tabpanel, p->index);
    panel_update(app->tabpanel);
    app->tabid = p->index;
}

/*---------------------------------------------------------------------------*/

static Layout *i_tabs_layout(App *app)
{
    Layout *layout = layout_create(1, 2);
    Button *button1 = button_radio();
    Button *button2 = button_radio();
    cassert_no_null(app);
    button_text(button1, "Web Tab 1");
    button_text(button2, "Web Tab 2");
    button_OnClick(button1, listener(app, i_OnTab, App));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_margin4(layout, 0, 0, 0, 5);
    layout_vmargin(layout, 0, 5);
    button_state(app->tabid == 0 ? button1 : button2, ekGUI_ON);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnBack(App *app, Event *e)
{
    webview_back(app->tabid == 0 ? app->web1 : app->web2);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnForward(App *app, Event *e)
{
    webview_forward(app->tabid == 0 ? app->web1 : app->web2);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Layout *i_nav_layout(App *app)
{
    Layout *layout = layout_create(2, 1);
    Button *button1 = button_push();
    Button *button2 = button_push();
    button_text(button1, "< Back");
    button_text(button2, "> Forward");
    button_OnClick(button1, listener(app, i_OnBack, App));
    button_OnClick(button2, listener(app, i_OnForward, App));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 1, 0);
    layout_hmargin(layout, 0, 5);
    unref(app);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    i_web_navigate(app->tabid == 0 ? app->web1 : app->web2, p->index);
}

/*---------------------------------------------------------------------------*/

static Layout *i_control_layout(App *app)
{
    Layout *layout1 = layout_create(1, 5);
    Layout *layout2 = i_tabs_layout(app);
    Layout *layout3 = i_nav_layout(app);
    Label *label1 = label_create();
    Label *label2 = label_create();
    ListBox *list = listbox_create();
    label_text(label1, "Tabs");
    label_text(label2, "Go To");
    listbox_OnSelect(list, listener(app, i_OnSelect, App));
    listbox_add_elem(list, "NAppGUI", NULL);
    listbox_add_elem(list, "Google", NULL);
    listbox_add_elem(list, "YouTube", NULL);
    listbox_add_elem(list, "Facebook", NULL);
    listbox_add_elem(list, "Instagram", NULL);
    listbox_add_elem(list, "Twitter", NULL);
    listbox_select(list, 0, TRUE);
    layout_label(layout1, label1, 0, 0);
    layout_layout(layout1, layout2, 0, 1);
    layout_label(layout1, label2, 0, 2);
    layout_listbox(layout1, list, 0, 3);
    layout_layout(layout1, layout3, 0, 4);
    layout_vmargin(layout1, 0, 3);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 3);
    layout_vmargin(layout1, 3, 5);
    layout_margin(layout1, 10);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Panel *i_tab_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(1, 1);
    Layout *layout2 = layout_create(1, 1);
    WebView *view1 = webview_create();
    WebView *view2 = webview_create();
    cassert_no_null(app);
    webview_size(view1, s2df(800, 600));
    webview_size(view2, s2df(800, 600));
    i_web_navigate(view1, 0);
    i_web_navigate(view2, 1);
    layout_webview(layout1, view1, 0, 0);
    layout_webview(layout2, view2, 0, 0);
    panel_layout(panel, layout1);
    panel_layout(panel, layout2);
    app->tabpanel = panel;
    app->web1 = view1;
    app->web2 = view2;
    panel_visible_layout(panel, app->tabid);
    return panel;
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(2, 1);
    Layout *layout2 = i_control_layout(app);
    Panel *tabpanel = i_tab_panel(app);
    layout_valign(layout1, 0, 0, ekTOP);
    layout_layout(layout1, layout2, 0, 0);
    layout_panel(layout1, tabpanel, 1, 0);
    layout_hexpand(layout1, 1);
    panel_layout(panel, layout1);
    return panel;
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static App *i_app(void)
{
    App *app = heap_new0(App);
    app->tabid = 0;
    return app;
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = i_app();
    Panel *panel = i_panel(app);
    app->window = window_create(ekWINDOW_STDRES);
    window_panel(app->window, panel);
    window_title(app->window, "Hello WebView");
    window_origin(app->window, v2df(500, 200));
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
