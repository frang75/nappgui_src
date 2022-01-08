/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: main.cpp
 *
 */

/* NAppGUI C++ Hello World */
    
#include "nappgui.h"

class App;

class MainWindow : public IListener
{
public:
    MainWindow(App *app);
    ~MainWindow();

    void *operator new(size_t size) { return (void*)heap_malloc((uint32_t)size, "MainWindow"); }
    void operator delete(void *ptr, size_t size) { heap_free((byte_t**)&ptr, (uint32_t)size, "MainWindow"); }

private:
    void i_OnButton(Event *e);
    Panel *i_panel(void);

    Window *window;
    TextView *text;
    uint32_t clicks;
};

/*---------------------------------------------------------------------------*/

class App : public IListener
{
public:
    App();
    ~App();
    void i_OnClose(Event *e);
    void *operator new(size_t size) { return (void*)heap_malloc((uint32_t)size, "App"); }
    void operator delete(void *ptr, size_t size) { heap_free((byte_t**)&ptr, (uint32_t)size, "App"); }

private:
    MainWindow *main_window;
};

/*---------------------------------------------------------------------------*/

void MainWindow::i_OnButton(Event *e)
{
    String *msg = str_printf("Button click (%d)\n", this->clicks);
    textview_writef(this->text, tc(msg));
    str_destroy(&msg);
    this->clicks += 1;
    unref(e);
}

/*---------------------------------------------------------------------------*/

Panel *MainWindow::i_panel(void)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 3);
    Label *label = label_create();
    Button *button = button_push();
    TextView *textv = textview_create();
    this->text = textv;
    label_text(label, "Hello!, I'm a label");
    button_text(button, "Click Me!");
    button_OnClick(button, IListen(this, MainWindow, i_OnButton));
    layout_label(layout, label, 0, 0);
    layout_button(layout, button, 0, 1);
    layout_textview(layout, textv, 0, 2);
    layout_hsize(layout, 0, 250);
    layout_vsize(layout, 2, 100);
    layout_margin(layout, 5);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    panel_layout(panel, layout);
    return panel;
}

/*---------------------------------------------------------------------------*/

void App::i_OnClose(Event *e)
{
    osapp_finish();
    unref(e);
}

/*---------------------------------------------------------------------------*/

MainWindow::MainWindow(App *app)
{
    Panel *panel = i_panel();
    this->window = window_create(ekWNSTD);
    this->clicks = 0;
    window_panel(this->window, panel);
    window_title(this->window, "Hello, C++!");
    window_origin(this->window, v2df(500, 200));
    window_OnClose(this->window, IListen(app, App, i_OnClose));
    window_show(this->window);
}

/*---------------------------------------------------------------------------*/

MainWindow::~MainWindow()
{
    window_destroy(&this->window);
}

/*---------------------------------------------------------------------------*/

App::App(void)
{
    this->main_window = new MainWindow(this);
}

/*---------------------------------------------------------------------------*/

App::~App()
{
    delete this->main_window;
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    return new App();
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    delete *app;
    *app = NULL;
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
