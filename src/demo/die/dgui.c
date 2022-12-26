/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dgui.c
 *
 */

/* Die Gui */

#include "dgui.h"
#include "ddraw.h"
#include "guiall.h"
#include "res_die.h"

/*---------------------------------------------------------------------------*/

static void i_OnDraw(App *app, Event *e)
{
    color_t green = color_rgb(102, 153, 26);
    const EvDraw *params = event_params(e, EvDraw);
    draw_clear(params->ctx, green);
    die_draw(params->ctx, 0, 0, params->width, params->height, app->padding, app->corner, app->radius, app->face);
}

/*---------------------------------------------------------------------------*/

static void i_OnFace(App *app, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    app->face = params->index + 1;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnPadding(App *app, Event *e)
{
    const EvSlider *params = event_params(e, EvSlider);
    app->padding = params->pos;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnCorner(App *app, Event *e)
{
    const EvSlider *params = event_params(e, EvSlider);
    app->corner = params->pos;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnRadius(App *app, Event *e)
{
    const EvSlider *params = event_params(e, EvSlider);
    app->radius = params->pos;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnLang(App *app, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    const char_t *lang = params->index == 0 ? "en_us" : "es_es";
    gui_language(lang);
    unref(app);
}

/*---------------------------------------------------------------------------*/

 static Panel *i_panel(App *app)
 {
    Panel *panel = panel_create();
    Layout *layout = layout_create(2, 1);
    Layout *layout1 = layout_create(2, 6);
    View *view = view_create();
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_multiline();
    PopUp *popup1 = popup_create();
    PopUp *popup2 = popup_create();
    Slider *slider1 = slider_create();
    Slider *slider2 = slider_create();
    Slider *slider3 = slider_create();
    ImageView *img = imageview_create();
    app->view = view;
    view_size(view, s2df(200, 200));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    label_text(label1, TEXT_LANG);
    label_text(label2, TEXT_FACE);
    label_text(label3, TEXT_PADDING);
    label_text(label4, TEXT_CORNER);
    label_text(label5, TEXT_RADIUS);
    label_text(label6, TEXT_INFO);
    popup_add_elem(popup1, TEXT_ENGLISH, resid_image(USA_PNG));
    popup_add_elem(popup1, TEXT_SPANISH, resid_image(SPAIN_PNG));
    popup_OnSelect(popup1, listener(app, i_OnLang, App));
    popup_add_elem(popup2, TEXT_ONE, NULL);
    popup_add_elem(popup2, TEXT_TWO, NULL);
    popup_add_elem(popup2, TEXT_THREE, NULL);
    popup_add_elem(popup2, TEXT_FOUR, NULL);
    popup_add_elem(popup2, TEXT_FIVE, NULL);
    popup_add_elem(popup2, TEXT_SIX, NULL);
    popup_OnSelect(popup2, listener(app, i_OnFace, App));
    popup_selected(popup2, app->face - 1);
    slider_value(slider1, app->padding);
    slider_value(slider2, app->corner);
    slider_value(slider3, app->radius);
    slider_OnMoved(slider1, listener(app, i_OnPadding, App));
    slider_OnMoved(slider2, listener(app, i_OnCorner, App));
    slider_OnMoved(slider3, listener(app, i_OnRadius, App));
    imageview_image(img, (const Image*)CARDS_PNG);
    layout_view(layout, view, 0, 0);
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 0, 1);
    layout_label(layout1, label3, 0, 2);
    layout_label(layout1, label4, 0, 3);
    layout_label(layout1, label5, 0, 4);
    layout_imageview(layout1, img, 0, 5);
    layout_popup(layout1, popup1, 1, 0);
    layout_popup(layout1, popup2, 1, 1);
    layout_slider(layout1, slider1, 1, 2);
    layout_slider(layout1, slider2, 1, 3);
    layout_slider(layout1, slider3, 1, 4);
    layout_label(layout1, label6, 1, 5);
    layout_layout(layout, layout1, 1, 0);
    layout_margin(layout, 10);
    layout_hsize(layout1, 1, 150);
    layout_hmargin(layout, 0, 10);
    layout_hmargin(layout1, 0, 5);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    layout_vmargin(layout1, 3, 5);
    layout_vmargin(layout1, 4, 5);
    layout_hexpand(layout, 0);
    layout_valign(layout, 1, 0, ekTOP);
    panel_layout(panel, layout);
    return panel;
}

/*---------------------------------------------------------------------------*/

Window *dgui_window(App *app)
{
    gui_respack(res_die_respack);
    gui_language("");

    {
        Panel *panel = i_panel(app);
        Window *window = window_create(ekWINDOW_STDRES);
        window_panel(window, panel);
        window_title(window, TEXT_TITLE);
        return window;
    }
}
