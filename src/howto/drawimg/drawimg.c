/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawimg.c
 *
 */

/* Drawing on an image */

#include "nappgui.h"
#include "allres.h"

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    Window *expwin;
    Font *font;
    View *view;
    ImageView *iview;
    uint32_t res;
    real32_t angle;
    real32_t scale;
    String *exp_path;
    codec_t exp_codec;
    uint32_t exp_bpp;
    bool_t exp_alpha;
};

static uint32_t i_WIDTH[4] = {600, 300, 150, 75};
static uint32_t i_HEIGHT[4] = {400, 200, 100, 50};
static real32_t i_SCALE[4] = {1, .5f, .25f, .125f};

/*---------------------------------------------------------------------------*/

//static void i_dbind(void)
//{
//    dbind_enum(codec_t, ekJPG, "JPG");
//    dbind_enum(codec_t, ekPNG, "PNG");
//    dbind_enum(codec_t, ekBMP, "BMP");
//    dbind_enum(codec_t, ekBMP, "ekGIF");
//    dbind(App, String*, exp_path);
//    dbind(App, codec_t, exp_codec);
//    dbind(App, uint32_t, exp_bpp);
//    dbind(App, bool_t, exp_alpha);
//}

/*---------------------------------------------------------------------------*/

static void i_draw(DCtx *ctx, const T2Df *t2d_global, const Font *font)
{
    T2Df t2d_object;
    V2Df triangle[] = { {472,0}, {600,144}, {344,144} };
    const Image *image1 = gui_image(MONKEY_GIF);
    const Image *image2 = gui_image(ZOMBIE_PNG);
    t2d_scalef(&t2d_object, t2d_global, .5f, .5f);
    draw_matrixf(ctx, &t2d_object);
    draw_image(ctx, image1, 688, 288);
    draw_line_color(ctx, color_rgb(255, 0, 0));
    draw_line_width(ctx, 3);
    draw_fill_color(ctx, color_rgb(0, 0, 255));
    t2d_rotatef(&t2d_object, t2d_global, kBMATH_PIf / 4);
    draw_matrixf(ctx, &t2d_object);
    draw_rect(ctx, ekSKFILL, 0, 0, 320, 200);
    draw_fill_color(ctx, color_rgb(0, 255, 0));
    draw_matrixf(ctx, t2d_global);
    draw_circle(ctx, ekSKFILL, 300, 200, 100);
    draw_line_color(ctx, color_rgb(0, 0, 255));
    draw_fill_color(ctx, color_rgb(255, 0, 0));
    draw_polygon(ctx, ekSKFILL, triangle, 3);
    t2d_scalef(&t2d_object, t2d_global, .7f, .7f);
    draw_matrixf(ctx, &t2d_object);
    draw_image(ctx, image2, 0, 0);
    draw_font(ctx, font);
    draw_matrixf(ctx, t2d_global);
    draw_text_color(ctx, color_rgb(255, 0, 0));
    draw_text(ctx, "Hello Drawings!", 200, 15);
    draw_line_color(ctx, color_rgb(0, 128, 0));
    draw_line(ctx, 150, 350, 330, 350);
    draw_line_color(ctx, color_rgb(0, 0, 255));
    draw_line(ctx, 100, 370, 330, 370);
    draw_line_color(ctx, color_rgb(128, 0, 0));
    draw_line(ctx, 20, 390, 330, 390);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(App *app, Event *e)
{
    T2Df t2d;
    const EvDraw *p = event_params(e, EvDraw);
    t2d_rotatef(&t2d, kT2D_IDENTf, app->angle);
    t2d_scalef(&t2d, &t2d, app->scale, 1);
    draw_clear(p->ctx, color_rgb(200, 200, 200));
    i_draw(p->ctx, &t2d, app->font);
}

/*---------------------------------------------------------------------------*/

static void i_draw_img(App *app)
{
    T2Df t2d;
    DCtx *ctx = dctx_bitmap(i_WIDTH[app->res], i_HEIGHT[app->res], ekRGB24);
    Image *image;
    t2d_scalef(&t2d, kT2D_IDENTf, i_SCALE[app->res], i_SCALE[app->res]);
    draw_clear(ctx, color_rgb(200, 200, 200));
    i_draw(ctx, &t2d, app->font);
    image = dctx_image(&ctx);
    imageview_image(app->iview, image);
    image_destroy(&image);
}

/*---------------------------------------------------------------------------*/

static void i_OnResolution(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    app->res = p->index;
    i_draw_img(app);
}

/*---------------------------------------------------------------------------*/

static Layout *i_filename_layout(void)
{
    Layout *layout = layout_create(2, 1);
    Edit *edit = edit_create();
    Button *button = button_push();
    //Image *image = image_system(".");
    //button_image(button, image);
    button_text(button, "Open");
    layout_edit(layout, edit, 0, 0);
    layout_button(layout, button, 1, 0);
    //image_destroy(&image);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_bpp_layout(void)
{
    Layout *layout = layout_create(1, 5);
    Button *button1 = button_radio();
    Button *button2 = button_radio();
    Button *button3 = button_radio();
    Button *button4 = button_radio();
    Button *button5 = button_radio();
    button_text(button1, "1 bpp (2 colors)");
    button_text(button2, "2 bpp (4 colors)");
    button_text(button3, "4 bpp (16 colors)");
    button_text(button4, "8 bpp (32 colors)");
    button_text(button5, "RGB (True color)");
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    layout_button(layout, button5, 0, 4);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnOk(App *app, Event *e)
{
    window_stop_modal(app->expwin, 1);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnCancel(App *app, Event *e)
{
    window_stop_modal(app->expwin, 0);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Window *i_export_window(App *app)
{
    Window *window = window_create(ekWNTITLE | ekWNCLOSE);
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(3, 4);
    Layout *layout2 = i_filename_layout();
    Layout *layout3 = i_bpp_layout();
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    PopUp *popup = popup_create();
    Button *button1 = button_check();
    Button *button2 = button_push();
    Button *button3 = button_push();
    label_text(label1, "File name:");
    label_text(label2, "Format:");
    label_text(label3, "Pixel Depth (bpp):");
    label_text(label4, "Transparent background:");
    button_text(button2, "Ok");
    button_text(button3, "Cancel");
    button_OnClick(button2, listener(app, i_OnOk, App));
    button_OnClick(button3, listener(app, i_OnCancel, App));
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 0, 1);
    layout_label(layout1, label3, 0, 2);
    layout_label(layout1, label4, 0, 3);
    layout_layout(layout1, layout2, 1, 0);
    layout_popup(layout1, popup, 1, 1);
    layout_layout(layout1, layout3, 1, 2);
    layout_button(layout1, button1, 1, 3);
    layout_button(layout1, button2, 2, 0);
    layout_button(layout1, button3, 2, 1);
    panel_layout(panel, layout1);
    window_panel(window, panel);
    window_title(window, "Image export");
    return window;
}

/*---------------------------------------------------------------------------*/

static void i_export_png(void)
{
    const uint32_t w = 640, h = 400;
    uint32_t i, j, wi = w / 4;
    Palette *palette = palette_create(4);
    Pixbuf *pixbuf = pixbuf_create(w, h, ekINDEX2);
    color_t *c = palette_colors(palette);
    Image *image = NULL;
    c[0] = color_rgba(255, 0, 0, 255);
    c[1] = color_rgba(0, 255, 0, 170);
    c[2] = color_rgba(0, 0, 255, 85);
    c[3] = color_rgba(255, 255, 255, 1);
    for (i = 0; i < w; ++i)
    {
        uint32_t idx = 3;
        if (i < wi)
            idx = 0;
        else if (i < 2 * wi)
            idx = 1;
        else if (i < 3 * wi)
            idx = 2;

        for (j = 0; j < h; ++j)
            pixbuf_set(pixbuf, i, j, idx);
    }

    image = image_from_pixbuf(pixbuf, palette);
    image_codec(image, ekGIF);
    image_to_file(image, "/home/fran/Desktop/export.gif", NULL);
    pixbuf_destroy(&pixbuf);
    palette_destroy(&palette);
    image_destroy(&image);

    {
        Image *img = image_from_file("/home/fran/Desktop/country.jpg", NULL);
        image_codec(img, ekGIF);
        image_to_file(img, "/home/fran/Desktop/country.gif", NULL);
        image_destroy(&img);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnExport(App *app, Event *e)
{
    V2Df p0, p1;
    S2Df s0, s1;
    uint32_t res = 0;
	unref(e);
    app->expwin = i_export_window(app);
    p0 = window_get_origin(app->window);
    s0 = window_get_size(app->window);
    s1 = window_get_size(app->expwin);
    p1 = v2df(p0.x + (s0.width - s1.width) / 2, p0.y + (s0.height - s1.height) / 2);
    window_origin(app->expwin, p1);
    res = window_modal(app->expwin, app->window);

    if (res == 1)
    {
        i_export_png();
    }

    window_destroy(&app->expwin);
}

/*---------------------------------------------------------------------------*/

static Layout *i_img_layout(App *app)
{
    Layout *layout = layout_create(7, 1);
    Label *label = label_create();
    Button *button1 = button_radio();
    Button *button2 = button_radio();
    Button *button3 = button_radio();
    Button *button4 = button_radio();
    Button *button5 = button_push();
    label_text(label, "Image context:");
    button_text(button1, "600x400");
    button_text(button2, "300x200");
    button_text(button3, "150x100");
    button_text(button4, "75x50");
    button_text(button5, "Export...");
    button_state(button1, ekON);
    button_OnClick(button1, listener(app, i_OnResolution, App));
    button_OnClick(button5, listener(app, i_OnExport, App));
    layout_label(layout, label, 0, 0);
    layout_button(layout, button1, 1, 0);
    layout_button(layout, button2, 2, 0);
    layout_button(layout, button3, 3, 0);
    layout_button(layout, button4, 4, 0);
    layout_button(layout, button5, 6, 0);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 10);
    layout_hmargin(layout, 2, 10);
    layout_hmargin(layout, 3, 10);
    layout_hexpand(layout, 5);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnAngle(App *app, Event *e)
{
    const EvSlider *p = event_params(e, EvSlider);
    app->angle = (p->pos - .5f) * kBMATH_PIf;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnScale(App *app, Event *e)
{
    const EvSlider *p = event_params(e, EvSlider);
    app->scale = p->pos + .5f;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static Layout *i_win_layout(App *app)
{
    Layout *layout = layout_create(5, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Slider *slider1 = slider_create();
    Slider *slider2 = slider_create();
    label_text(label1, "Window context: 600x400");
    label_text(label2, "Angle:");
    label_text(label3, "Scale:");
    slider_value(slider1, .5f);
    slider_value(slider2, .5f);
    slider_OnMoved(slider1, listener(app, i_OnAngle, App));
    slider_OnMoved(slider2, listener(app, i_OnScale, App));
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 1, 0);
    layout_label(layout, label3, 3, 0);
    layout_slider(layout, slider1, 2, 0);
    layout_slider(layout, slider2, 4, 0);
    layout_hmargin(layout, 0, 10);
    layout_hmargin(layout, 2, 10);
    layout_hexpand2(layout, 2, 4, .5f);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(2, 2);
    Layout *layout2 = i_win_layout(app);
    Layout *layout3 = i_img_layout(app);
    View *view = view_create();
    ImageView *iview = imageview_create();
    view_size(view, s2df(600, 400));
    imageview_size(iview, s2df(600, 400));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    imageview_scale(iview, ekASPECT);
    layout_layout(layout1, layout2, 0, 0);
    layout_view(layout1, view, 0, 1);
    layout_imageview(layout1, iview, 1, 1);
    layout_layout(layout1, layout3, 1, 0);
    layout_margin(layout1, 10);
    layout_hmargin(layout1, 0, 5);
    layout_vmargin(layout1, 0, 5);
    panel_layout(panel, layout1);
    app->view = view;
    app->iview = iview;
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

static App *i_create(void)
{
    App *app = heap_new0(App);
    Panel *panel = i_panel(app);
    gui_respack(allres_respack);
    gui_language("");
    app->window = window_create(ekWNSTD);
    app->font = font_system(25.f, 0);
    app->res = 0;
    app->angle = 0;
    app->scale = 1;
    i_draw_img(app);
    window_panel(app->window, panel);
    window_title(app->window, "Drawing on an image");
    window_origin(app->window, v2df(500, 200));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    font_destroy(&(*app)->font);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
