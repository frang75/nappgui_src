/* View with scroll */

#include "scrollview.h"
#include <gui/guiall.h>

typedef struct _vdata_t VData;

struct _vdata_t
{
    View *view;
    real32_t width;
    real32_t height;
};

static const real32_t i_BORDER_SIZE = 30;

/*---------------------------------------------------------------------------*/

static void i_destroy_data(VData **data)
{
    heap_delete(data, VData);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(VData *data, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    draw_fill_color(p->ctx, kCOLOR_RED);
    draw_rect(p->ctx, ekFILL, 0, 0, data->width, data->height);
    draw_fill_color(p->ctx, kCOLOR_BLUE);
    draw_rect(p->ctx, ekFILL, i_BORDER_SIZE, i_BORDER_SIZE, data->width - 2 * i_BORDER_SIZE, data->height - 2 * i_BORDER_SIZE);
}

/*---------------------------------------------------------------------------*/

static void i_OnFix(VData *data, Event *e)
{
    const EvButton *params = event_params(e, EvButton);

    switch (params->index)
    {
    case 0:
        data->width = 256;
        data->height = 256;
        break;
    case 1:
        data->width = 512;
        data->height = 256;
        break;
    case 2:
        data->width = 256;
        data->height = 512;
        break;
    case 3:
        data->width = 512;
        data->height = 512;
        break;
    default:
        cassert_default(params->index);
    }

    view_content_size(data->view, s2df(data->width, data->height), s2df(10, 10));
    view_update(data->view);
}

/*---------------------------------------------------------------------------*/

Panel *scrollview(void)
{
    VData *data = heap_new0(VData);
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = layout_create(2, 2);
    View *view = view_scroll();
    Button *radio1 = button_radio();
    Button *radio2 = button_radio();
    Button *radio3 = button_radio();
    Button *radio4 = button_radio();
    data->view = view;
    data->width = 256;
    data->height = 256;
    button_text(radio1, "Full fix");
    button_text(radio2, "Vertical fix");
    button_text(radio3, "Horizontal fix");
    button_text(radio4, "No fix");
    button_state(radio1, ekGUI_ON);
    button_OnClick(radio1, listener(data, i_OnFix, VData));
    view_size(view, s2df(256, 256));
    view_content_size(view, s2df(data->width, data->height), s2df(10, 10));
    view_OnDraw(view, listener(data, i_OnDraw, VData));
    layout_button(layout2, radio1, 0, 0);
    layout_button(layout2, radio2, 1, 0);
    layout_button(layout2, radio3, 0, 1);
    layout_button(layout2, radio4, 1, 1);
    layout_layout(layout1, layout2, 0, 0);
    layout_view(layout1, view, 0, 1);
    layout_halign(layout1, 0, 0, ekLEFT);
    layout_vmargin(layout1, 0, 10);
    layout_hmargin(layout2, 0, 10);
    layout_vmargin(layout2, 0, 5);
    layout_margin(layout1, 10);
    panel_layout(panel, layout1);
    panel_data(panel, &data, i_destroy_data, VData);
    return panel;
}
