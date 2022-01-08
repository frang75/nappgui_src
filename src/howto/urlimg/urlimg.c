/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: urlimg.c
 *
 */

/* Images from URL */

#include "nappgui.h"
#include "inet.h"
#include "httpreq.h"

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    ImageView *view;
    uint32_t selected;
    Label *imgname;
    Label *imgsize;
    Label *imgres;
    Label *imgformat;
};

static const char_t *i_FILES[] = {
                            "anim_04_bat.gif",
                            "anim_04_cube.gif",
                            "anim_04_dragon.gif",
                            "anim_04_game.gif",
                            "anim_04_item.gif",
                            "anim_04_player.gif",
                            "home_01_gray.jpg",
                            "home_01_rgb.jpg",
                            "home_02_index02.png",
                            "home_02_index02c.png",
                            "home_02_index04.png",
                            "home_02_index04c.png",
                            "home_02_index16.png",
                            "home_02_index16c.png",
                            "home_02_index256.png",
                            "home_02_index256c.png",
                            "home_02_rgba.png",
                            "home_02_rgbac.png",
                            "sea_01_gray.jpg",
                            "sea_01_rgb.jpg",
                            "sea_02_index02.png",
                            "sea_02_index02c.png",
                            "sea_02_index04.png",
                            "sea_02_index04c.png",
                            "sea_02_index16.png",
                            "sea_02_index16c.png",
                            "sea_02_index256.png",
                            "sea_02_index256c.png",
                            "sea_02_rgb.png",
                            "sea_02_rgbc.png",
                            "sea_03_bmp_index02.bmp",
                            "sea_03_bmp_index04.bmp",
                            "sea_03_bmp_index16.bmp",
                            "sea_03_bmp_index256.bmp",
                            "sea_03_bmp_rgb.bmp" };

/*---------------------------------------------------------------------------*/

static __INLINE String *i_pixformat(const pixformat_t format, const uint32_t ncolors)
{
    switch (format) {
    case ekINDEX1:
        return str_printf("Indexed 1bbp (%d colors)", ncolors);
    case ekINDEX2:
        return str_printf("Indexed 2bbp (%d colors)", ncolors);
    case ekINDEX4:
        return str_printf("Indexed 4bbp (%d colors)", ncolors);
    case ekINDEX8:
        return str_printf("Indexed 8bbp (%d colors)", ncolors);
    case ekGRAY8:
        return str_c("Gray8");
    case ekRGB24:
        return str_c("RGB24");
    case ekRGBA32:
        return str_c("RGBA32");
    case ekFIMAGE:
		break;
    }
    return str_c("Unknown");
}

/*---------------------------------------------------------------------------*/

static void i_download(App *app)
{
    String *url = str_printf("http://test.nappgui.com/image_formats/%s", i_FILES[app->selected]);
    Stream *stm = http_dget(tc(url), NULL, NULL);
    if (stm != NULL)
    {
        uint32_t ncolors = 0;
        uint64_t start = stm_bytes_readed(stm);
        Image *image = image_read(stm);
        uint64_t end = stm_bytes_readed(stm);
        uint32_t width = image_width(image);
        uint32_t height = image_width(image);
        pixformat_t format = image_format(image);
        String *ssize = str_printf("%d bytes", (uint32_t)(end - start));
        String *sres = NULL;
        String *sformat = NULL;

        // Full check of read/write pixels
        // We create again the same image, based on pixel info
        if (image_get_codec(image) != ekGIF)
        {
            Pixbuf *pixels = image_pixels(image, ekFIMAGE);
            Image *nimage = image_from_pixbuf(pixels, NULL);
            cassert(format == pixbuf_format(pixels));
            pixbuf_destroy(&pixels);
            image_destroy(&image);
            image = nimage;
        }

        imageview_image(app->view, image);
        sres = str_printf("%d x %d", width, height);
        sformat = i_pixformat(format, ncolors);
        label_text(app->imgname, i_FILES[app->selected]);
        label_text(app->imgsize, tc(ssize));
        label_text(app->imgres, tc(sres));
        label_text(app->imgformat, tc(sformat));
        stm_close(&stm);
        image_destroy(&image);
        str_destroy(&ssize);
        str_destroy(&sres);
        str_destroy(&sformat);
    }

    str_destroy(&url);
}

/*---------------------------------------------------------------------------*/

static Layout* i_label(const char_t *title, Label **info)
{
    Layout *layout = layout_create(2, 1);
    Label *label = label_create();
    Font *font = font_system(font_regular_size(), ekFBOLD);
    *info = label_create();
    label_text(label, title);
    label_font(label, font);
    layout_label(layout, label, 0, 0);
    layout_label(layout, *info, 1, 0);
    layout_halign(layout, 1, 0, ekJUSTIFY);
    layout_hmargin(layout, 0, 5);
    layout_hexpand(layout, 1);
    font_destroy(&font);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_add_files(ListBox *listbox)
{
    register uint32_t i, n = sizeof(i_FILES) / sizeof(char_t*);
    for (i = 0; i < n; ++i)
        listbox_add_elem(listbox, i_FILES[i], NULL);
    listbox_select(listbox, 0, TRUE);    
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    app->selected = p->index;
    i_download(app);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(2, 1);
    Layout *layout2 = layout_create(1, 2);
    Layout *layout3 = layout_create(1, 5);
    Label *label = label_create();
    ListBox *listbox = listbox_create();
    ImageView *view = imageview_create();
    app->view = view;
    label_text(label, "Images");
    i_add_files(listbox);
    listbox_OnSelect(listbox, listener(app, i_OnSelect, App));
    imageview_size(view, s2df(600, 400));
    layout_label(layout2, label, 0, 0);
    layout_listbox(layout2, listbox, 0, 1);
    layout_imageview(layout3, view, 0, 0);
    layout_layout(layout3, i_label("Image name:", &app->imgname), 0, 1);
    layout_layout(layout3, i_label("Image size:", &app->imgsize), 0, 2);
    layout_layout(layout3, i_label("Image dimensions:", &app->imgres), 0, 3);
    layout_layout(layout3, i_label("Pixel format:", &app->imgformat), 0, 4);
    layout_layout(layout1, layout2, 0, 0);
    layout_layout(layout1, layout3, 1, 0);
    layout_margin(layout1, 5);
    layout_hmargin(layout1, 0, 5);
    layout_vmargin(layout2, 0, 5);
    layout_vmargin(layout3, 0, 5);
    layout_vmargin(layout3, 1, 3);
    layout_vmargin(layout3, 2, 3);
    layout_hsize(layout1, 0, 200);
    layout_vexpand(layout2, 1);
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

static App *i_create(void)
{
    App *app = heap_new0(App);
    Panel *panel = i_panel(app);
    app->window = window_create(ekWNSTD);
    app->selected = 0;
    inet_start();
    i_download(app);
    window_panel(app->window, panel);
    window_title(app->window, "Images from URL 'http://test.nappgui.com/image_formats'");
    window_origin(app->window, v2df(500, 200));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    inet_finish();
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
