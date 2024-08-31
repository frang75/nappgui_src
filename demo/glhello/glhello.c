/* GLDemo Application */

#include "nappgui.h"
#include "glhello.h"
#include "res_glhello.h"

#include <ogl3d/ogl3d.h>
#include "ogl1.h"
#include "ogl2.h"
#include "ogl3.h"

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    Panel *glpanel;
    View *glview;
    Label *errlabel;
    real32_t ptscale;
    OGL1 *ogl1;
    OGL2 *ogl2;
    OGL3 *ogl3;
    uint32_t api;
    real32_t angle;
    real32_t scale;
    Pixbuf *texdata;
};

static App *APP = NULL;

/*---------------------------------------------------------------------------*/

static void i_destroy_gl_apps(App *app)
{
    if (app->ogl1 != NULL)
        ogl1_destroy(&app->ogl1);

    if (app->ogl2 != NULL)
        ogl2_destroy(&app->ogl2);

    if (app->ogl3 != NULL)
        ogl3_destroy(&app->ogl3);
}

/*---------------------------------------------------------------------------*/

static void i_set_glcontext(App *app, const uint32_t index)
{
    if (index != app->api)
    {
        String *err = NULL;

        app->api = index;
        i_destroy_gl_apps(app);

        switch (index)
        {
        case 0:
        {
            oglerr_t glerr;
            app->ogl1 = ogl1_create(app->glview, &glerr);
            if (app->ogl1 == NULL)
                err = str_printf("Error creating OpenGL 1.1 context\n%s", ogl3d_err_str(glerr));
            break;
        }

        case 1:
        {
            oglerr_t glerr;
            app->ogl2 = ogl2_create(app->glview, &glerr);
            if (app->ogl2 == NULL)
                err = str_printf("Error creating OpenGL 2.1 context\n%s", ogl3d_err_str(glerr));
            break;
        }

        case 2:
        {
            oglerr_t glerr;
            app->ogl3 = ogl3_create(app->glview, &glerr);
            if (app->ogl3 == NULL)
                err = str_printf("Error creating OpenGL 3.3 context\n%s", ogl3d_err_str(glerr));
            break;
        }

        case 3:
            err = str_printf("Error creating OpenGL 4.3 context\n%s", "Not available");
            break;

        case 4:
            err = str_printf("Error creating DirectX 9 context\n%s", "Not available");
            break;

        case 5:
            err = str_printf("Error creating DirectX 9.c context\n%s", "Not available");
            break;

        case 6:
            err = str_printf("Error creating DirectX 10 context\n%s", "Not available");
            break;

        case 7:
            err = str_printf("Error creating DirectX 11 context\n%s", "Not available");
            break;

        case 8:
            err = str_printf("Error creating DirectX 12 context\n%s", "Not available");
            break;

        case 9:
            err = str_printf("Error creating Vulkan context\n%s", "Not available");
            break;

        case 10:
            err = str_printf("Error creating Metal context\n%s", "Not available");
            break;
        }

        if (err == NULL)
        {
            panel_visible_layout(app->glpanel, 0);
        }
        else
        {
            label_text(app->errlabel, tc(err));
            str_destroy(&err);
            panel_visible_layout(app->glpanel, 1);
        }

        panel_update(app->glpanel);
        view_update(app->glview);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    i_set_glcontext(app, p->index);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(App *app, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    /* Some displays (macOS Retina) have double-scaled pixels */
    real32_t width = p->width * app->ptscale;
    real32_t height = p->height * app->ptscale;
    switch (app->api)
    {
    case 0:
        ogl1_draw(app->ogl1, width, height, app->angle, app->scale);
        break;
    case 1:
        ogl2_draw(app->ogl2, width, height, app->angle, app->scale);
        break;
    case 2:
        ogl3_draw(app->ogl3, width, height, app->angle, app->scale);
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    case 8:
        break;
    case 9:
        break;
    case 10:
        break;
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnSize(App *app, Event *e)
{
    const EvSize *p = event_params(e, EvSize);
    real32_t width = p->width * app->ptscale;
    real32_t height = p->height * app->ptscale;
    switch (app->api)
    {
    case 0:
        ogl1_resize(app->ogl1, width, height);
        break;
    case 1:
        ogl2_resize(app->ogl2, width, height);
        break;
    case 2:
        ogl3_resize(app->ogl3, width, height);
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    case 8:
        break;
    case 9:
        break;
    case 10:
        break;
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnAngle(App *app, Event *e)
{
    const EvSlider *p = event_params(e, EvSlider);
    app->angle = p->pos;
    view_update(app->glview);
}

/*---------------------------------------------------------------------------*/

static void i_OnScale(App *app, Event *e)
{
    const EvSlider *p = event_params(e, EvSlider);
    app->scale = 2 * p->pos;
    view_update(app->glview);
}

/*---------------------------------------------------------------------------*/

static Panel *i_gl_panel(App *app)
{
    Panel *panel = panel_create();
    View *view = view_create();
    Label *label = label_multiline();
    Font *font = font_system(20, ekFNORMAL);
    Layout *layout1 = layout_create(1, 1);
    Layout *layout2 = layout_create(1, 1);
    view_size(view, s2df(512, 512));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    view_OnSize(view, listener(app, i_OnSize, App));
    layout_view(layout1, view, 0, 0);
    label_font(label, font);
    label_align(label, ekCENTER);
    layout_label(layout2, label, 0, 0);
    layout_hsize(layout2, 0, 512);
    layout_vsize(layout2, 0, 512);
    layout_halign(layout2, 0, 0, ekCENTER);
    layout_valign(layout2, 0, 0, ekTOP);
    font_destroy(&font);
    panel_layout(panel, layout1);
    panel_layout(panel, layout2);
    app->errlabel = label;
    app->glpanel = panel;
    app->glview = view;
    view_point_scale(app->glview, &app->ptscale);
    return panel;
}

/*---------------------------------------------------------------------------*/

static Layout *i_rlayout(App *app)
{
    Layout *layout = layout_create(1, 3);
    Panel *panel = i_gl_panel(app);
    Slider *slider1 = slider_create();
    Slider *slider2 = slider_create();
    slider_OnMoved(slider1, listener(app, i_OnAngle, App));
    slider_OnMoved(slider2, listener(app, i_OnScale, App));
    slider_value(slider2, .5f);
    layout_panel(layout, panel, 0, 0);
    layout_slider(layout, slider1, 0, 1);
    layout_slider(layout, slider2, 0, 2);
    layout_vmargin(layout, 0, 10);
    layout_vexpand(layout, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(2, 1);
    ListBox *listbox = listbox_create();
    Layout *layout2 = i_rlayout(app);
    listbox_add_elem(listbox, "OpenGL 1.1/Fixed", NULL);
    listbox_add_elem(listbox, "OpenGL 2.1/GLSL", NULL);
    listbox_add_elem(listbox, "OpenGL 3.3", NULL);
    listbox_add_elem(listbox, "OpenGL 4", NULL);
    listbox_add_elem(listbox, "DirectX 9/Fixed", NULL);
    listbox_add_elem(listbox, "DirectX 9.c/HLSL", NULL);
    listbox_add_elem(listbox, "DirectX 10", NULL);
    listbox_add_elem(listbox, "DirectX 11", NULL);
    listbox_add_elem(listbox, "DirectX 12", NULL);
    listbox_add_elem(listbox, "Vulkan", NULL);
    listbox_add_elem(listbox, "Metal", NULL);
    listbox_select(listbox, 0, TRUE);
    listbox_OnSelect(listbox, listener(app, i_OnSelect, App));
    listbox_size(listbox, s2df(150, 300));
    layout_listbox(layout1, listbox, 0, 0);
    layout_layout(layout1, layout2, 1, 0);
    panel_layout(panel, layout1);
    layout_valign(layout1, 0, 0, ekTOP);
    layout_hsize(layout1, 0, 150);
    layout_hexpand(layout1, 1);
    layout_margin(layout1, 10);
    layout_hmargin(layout1, 0, 10);
    return panel;
}

/*---------------------------------------------------------------------------*/

static Window *i_window(App *app)
{
    Panel *panel = i_panel(app);
    Window *window = window_create(ekWINDOW_STDRES);
    window_panel(window, panel);
    window_title(window, "Hello 3D Render");
    i_set_glcontext(app, 0);
    return window;
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    unref(app);
    unref(e);
    osapp_finish();
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = NULL;
    app = heap_new0(App);
    APP = app;
    ogl3d_start();
    app->api = UINT32_MAX;
    app->window = i_window(app);
    app->angle = 0;
    app->scale = 1;
    window_origin(app->window, v2df(500.f, 200.f));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    cassert_no_null(app);
    cassert_no_null(*app);

    i_destroy_gl_apps(*app);
    window_destroy(&(*app)->window);

    if ((*app)->texdata != NULL)
        pixbuf_destroy(&(*app)->texdata);

    ogl3d_finish();
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

void glhello_texdata(const byte_t **texdata, uint32_t *texwidth, uint32_t *texheight, pixformat_t *texformat)
{
    if (APP->texdata == NULL)
    {
        ResPack *pack = res_glhello_respack("");
        APP->texdata = image_pixels(image_from_resource(pack, WALL_PNG), ekFIMAGE);
        respack_destroy(&pack);
    }

    *texdata = pixbuf_data(APP->texdata);
    *texwidth = pixbuf_width(APP->texdata);
    *texheight = pixbuf_height(APP->texdata);
    *texformat = pixbuf_format(APP->texdata);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
