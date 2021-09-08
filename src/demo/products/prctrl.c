/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: prctrl.c
 *
 */

/* Products Controller */

#include "prctrl.h"
#include "prmodel.h"
#include "nappgui.h"
#include "httpreq.h"
#include "json.h"
#include "res_gui.h"

typedef enum _status_t
{
    ekWAIT_LOGIN,
    ekIN_LOGIN,
    ekERR_LOGIN,
    ekOK_LOGIN
} status_t;

typedef struct _user_t User;
typedef struct _ujson_t UJson;

struct _user_t
{
    String *name;
    String *mail;
    Image *image64;
};

struct _ujson_t
{
    int32_t code;
    User data;
};

struct _ctrl_t
{
    Model *model;
    status_t status;
    wserv_t err;
    uint32_t selected;
    real32_t stats[12];
    UJson *ujson;
    Window *window;
    Layout *main_layout;
    Layout *status_layout;
    Cell *image_cell;
    Cell *first_cell;
    Cell *back_cell;
    Cell *next_cell;
    Cell *last_cell;
    Cell *add_cell;
    Cell *minus_cell;
    Cell *filter_cell;
    Cell *slider_cell;
    Cell *counter_cell;    
    Cell *code_cell;
    Cell *desc_cell;
    Cell *price_cell;    
    Cell *lang_cell;
    Cell *setting_cell;
    Cell *user_cell;
    Cell *pass_cell;
    Cell *login_cell;
    Cell *logout_cell;
    Cell *stats_cell;
    Panel *login_panel;
    Menu *lang_menu;
    MenuItem *import_item;
    MenuItem *export_item;
    MenuItem *first_item;
    MenuItem *back_item;
    MenuItem *next_item;
    MenuItem *last_item;
    MenuItem *setting_item;
    MenuItem *login_item;
    MenuItem *logout_item;
};

/*---------------------------------------------------------------------------*/

static real32_t i_MAX_STATS = 20.f;

/*---------------------------------------------------------------------------*/

Ctrl *ctrl_create(Model *model)
{
    Ctrl *ctrl = heap_new0(Ctrl);
    ctrl->model = model;
    ctrl->status = ekWAIT_LOGIN;
    ctrl->selected = 0;
    dbind(User, String*, name);
    dbind(User, String*, mail);
    dbind(User, Image*, image64);
    dbind(UJson, int32_t, code);
    dbind(UJson, User, data);
    return ctrl;
}

/*---------------------------------------------------------------------------*/

void ctrl_destroy(Ctrl **ctrl)
{
    heap_delete(ctrl, Ctrl);
}

/*---------------------------------------------------------------------------*/

void ctrl_main_layout(Ctrl *ctrl, Layout *layout)
{
    model_layout(layout);
    ctrl->main_layout = layout;
}

/*---------------------------------------------------------------------------*/

void ctrl_status_layout(Ctrl *ctrl, Layout *layout)
{
    ctrl->status_layout = layout;
}

/*---------------------------------------------------------------------------*/

static void i_update_product(Ctrl *ctrl)
{
    uint32_t total = model_count(ctrl->model);
    bool_t enabled = FALSE;
    bool_t is_first = (total == 0 || ctrl->selected == 0) ? TRUE : FALSE;
    bool_t is_last = (total == 0 || ctrl->selected == (total - 1)) ? TRUE : FALSE;
    Slider *slider = cell_slider(ctrl->slider_cell);
    Label *counter = cell_label(ctrl->counter_cell);
    Product *product = NULL;

    if (total > 0)
    {
        char_t msg[64];
        uint32_t i, n = sizeof(ctrl->stats) / sizeof(real32_t);
        View *vstats = cell_view(ctrl->stats_cell);
        product = model_product(ctrl->model, ctrl->selected);
        bstd_sprintf(msg, 64, "[%d/%d]", ctrl->selected + 1, total);
        label_text(counter, msg);
        slider_value(slider, (real32_t)ctrl->selected / (real32_t)(total > 1 ? total - 1 : 1));        
        enabled = TRUE;
        for (i = 0; i < n; ++i)
            ctrl->stats[i] = bmath_randf(2.f, i_MAX_STATS - 2.f);
        view_update(vstats);
    }
    else
    {
        label_text(counter, "[0/0]");
        slider_value(slider, 0.f);
        enabled = FALSE;
    }

    layout_dbind_obj(ctrl->main_layout, product, Product);
    cell_enabled(ctrl->add_cell, enabled);
    cell_enabled(ctrl->minus_cell, enabled);
    cell_enabled(ctrl->slider_cell, enabled);
    cell_enabled(ctrl->filter_cell, enabled);
    cell_enabled(ctrl->first_cell, !is_first);
    cell_enabled(ctrl->back_cell, !is_first);
    cell_enabled(ctrl->next_cell, !is_last);
    cell_enabled(ctrl->last_cell, !is_last);
    menuitem_enabled(ctrl->first_item, !is_first);
    menuitem_enabled(ctrl->back_item, !is_first);
    menuitem_enabled(ctrl->next_item, !is_last);
    menuitem_enabled(ctrl->last_item, !is_last);
}

/*---------------------------------------------------------------------------*/

static void i_status(Ctrl *ctrl)
{
    ImageView *view = layout_get_imageview(ctrl->status_layout, 0, 0);
    Label *label = layout_get_label(ctrl->status_layout, 1, 0);

    switch (ctrl->status) {
    case ekWAIT_LOGIN:
        imageview_image(view, (const Image*)LOGIN16_PNG);
        label_text(label, WAIT_LOGIN);
        break;

    case ekIN_LOGIN:
        imageview_image(view, (const Image*)SPIN_GIF);
        label_text(label, IN_LOGIN);
        break;

    case ekERR_LOGIN:
        imageview_image(view, (const Image*)ERROR_PNG);
        switch (ctrl->err) {
        case ekWS_CONNECT:
            label_text(label, ERR_CONNECT);
            break;
        case ekWS_JSON:
            label_text(label, ERR_JSON);
            break;
        case ekWS_ACCESS:
            label_text(label, ERR_ACCESS);
            break;
        case ekWS_OK:
        cassert_default();
        }
        break;

    case ekOK_LOGIN:
        imageview_image(view, (const Image*)OK_PNG);
        label_text(label, OK_LOGIN);
        break;

    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void ctrl_run(Ctrl *ctrl)
{
    Button *setting_button;
    PopUp *lang_popup;
    MenuItem *lang_item;
    uint32_t lang_index;
    ctrl->status = ekWAIT_LOGIN;
    setting_button = cell_button(ctrl->setting_cell);
    layout_show_col(ctrl->main_layout, 1, TRUE);
    button_state(setting_button, ekON);
    menuitem_state(ctrl->setting_item, ekON);
    lang_popup = cell_popup(ctrl->lang_cell);
    lang_index = popup_get_selected(lang_popup);
    lang_item = menu_get_item(ctrl->lang_menu, lang_index);
    menuitem_state(lang_item, ekON);
    menuitem_enabled(ctrl->login_item, TRUE);
    menuitem_enabled(ctrl->logout_item, FALSE);
    menuitem_enabled(ctrl->import_item, FALSE);
    menuitem_enabled(ctrl->export_item, FALSE);    
    i_status(ctrl);
    cell_focus(ctrl->user_cell);
    i_update_product(ctrl);
    window_defbutton(ctrl->window, cell_button(ctrl->login_cell));
}

/*---------------------------------------------------------------------------*/

static void i_OnFirst(Ctrl *ctrl, Event *e)
{
    ctrl->selected = 0;
    i_update_product(ctrl);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnImport(Ctrl *ctrl, Event *e)
{
    const char_t *type[] = { "dbp" };
    const char_t *file = comwin_open_file(ctrl->window, type, 1, NULL);
    if (file != NULL)
    {
        ferror_t err;
        if (model_import(ctrl->model, file, &err) == TRUE)
            i_update_product(ctrl);
    }
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_import_item(Ctrl *ctrl, MenuItem *item)
{
    ctrl->import_item = item;
    menuitem_OnClick(item, listener(ctrl, i_OnImport, Ctrl));
}

/*---------------------------------------------------------------------------*/

static void i_OnExport(Ctrl *ctrl, Event *e)
{
    const char_t *type[] = { "dbp" };
    const char_t *file = comwin_save_file(ctrl->window, type, 1, NULL);
    if (file != NULL)
    {
        ferror_t err;
        model_export(ctrl->model, file, &err);
    }
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_export_item(Ctrl *ctrl, MenuItem *item)
{
    ctrl->export_item = item;
    menuitem_OnClick(item, listener(ctrl, i_OnExport, Ctrl));
}

/*---------------------------------------------------------------------------*/

static void i_OnImgDraw(Ctrl *ctrl, Event *e)
{
    const EvDraw *params = event_params(e, EvDraw);
    const Image *image = gui_image(EDIT_PNG);
    uint32_t w = image_width(image);
    uint32_t h = image_height(image);
    draw_image(params->ctx, image, params->width - w - 10, params->height - h - 10);
    unref(ctrl);
}

/*---------------------------------------------------------------------------*/

static void i_OnImgClick(Ctrl *ctrl, Event *e)
{
    const char_t *type[] = { "png", "jpg" };
    const char_t *file = comwin_open_file(ctrl->window, type, 2, NULL);
    if (file != NULL)
    {
        Image *image = image_from_file(file, NULL);
        if (image != NULL)
        {
            ImageView *view = cell_imageview(ctrl->image_cell);
            imageview_image(view, image);
            image_destroy(&image);
        }
    }
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_image_cell(Ctrl *ctrl, Cell *cell)
{
    ImageView *view = cell_imageview(cell);
    model_image(cell);
    imageview_OnOverDraw(view, listener(ctrl, i_OnImgDraw, Ctrl));
    imageview_OnClick(view, listener(ctrl, i_OnImgClick, Ctrl));
    ctrl->image_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_first_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnFirst, Ctrl));
    ctrl->first_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_first_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnFirst, Ctrl));
    ctrl->first_item = item;
}

/*---------------------------------------------------------------------------*/

static void i_OnBack(Ctrl *ctrl, Event *e)
{
    if (ctrl->selected > 0)
    {
        ctrl->selected -= 1;
        i_update_product(ctrl);
    }
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_back_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnBack, Ctrl));
    ctrl->back_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_back_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnBack, Ctrl));
    ctrl->back_item = item;
}

/*---------------------------------------------------------------------------*/

static void i_OnNext(Ctrl *ctrl, Event *e)
{
    uint32_t total = model_count(ctrl->model);
    if (ctrl->selected < total - 1)
    {
        ctrl->selected += 1;
        i_update_product(ctrl);
    }
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_next_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnNext, Ctrl));
    ctrl->next_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_next_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnNext, Ctrl));
    ctrl->next_item = item;
}

/*---------------------------------------------------------------------------*/

static void i_OnLast(Ctrl *ctrl, Event *e)
{
    uint32_t total = model_count(ctrl->model);
    if (ctrl->selected < total - 1)
    {
        ctrl->selected = total - 1;
        i_update_product(ctrl);
    }
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_last_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnLast, Ctrl));
    ctrl->last_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_last_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnLast, Ctrl));
    ctrl->last_item = item;
}

/*---------------------------------------------------------------------------*/

static void i_OnAdd(Ctrl *ctrl, Event *e)
{
    model_add(ctrl->model);
    ctrl->selected = model_count(ctrl->model) - 1;
    i_update_product(ctrl);
    cell_focus(ctrl->code_cell);
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_add_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnAdd, Ctrl));
    ctrl->add_cell = cell;
}

/*---------------------------------------------------------------------------*/

static void i_OnDelete(Ctrl *ctrl, Event *e)
{
    model_delete(ctrl->model, ctrl->selected);
    if (ctrl->selected == model_count(ctrl->model) && ctrl->selected > 0)
        ctrl->selected -= 1;
    i_update_product(ctrl);
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_minus_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnDelete, Ctrl));
    ctrl->minus_cell = cell;
}

/*---------------------------------------------------------------------------*/

static void i_OnFilter(Ctrl *ctrl, Event *e)
{
    const EvText *params = event_params(e, EvText);
    EvTextFilter *result = event_result(e, EvTextFilter);
    Combo *combo = event_sender(e, Combo);
    uint32_t color = color_rgb(255, 0, 0);

    if (unicode_nchars(params->text, ekUTF8) >= 3)
    {
        if (model_filter(ctrl->model, params->text) == TRUE)
        {
            color = UINT32_MAX;
            ctrl->selected = 0;
            i_update_product(ctrl);
        }
    }

    combo_color(combo, color);
    result->apply = FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_OnFilterEnd(Ctrl *ctrl, Event *e)
{
    const EvText *params = event_params(e, EvText);
    Combo *combo = event_sender(e, Combo);

    if (model_filter(ctrl->model, params->text) == TRUE)
        combo_ins_elem(combo, 0, params->text, NULL);
    else
        combo_text(combo, "");

    ctrl->selected = 0;
    i_update_product(ctrl);

    combo_color(combo, UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void ctrl_filter_cell(Ctrl *ctrl, Cell *cell)
{
    Combo *combo = cell_combo(cell);
    combo_OnFilter(combo, listener(ctrl, i_OnFilter, Ctrl));
    combo_OnChange(combo, listener(ctrl, i_OnFilterEnd, Ctrl));
    ctrl->filter_cell = cell;
}

/*---------------------------------------------------------------------------*/

static void i_OnSlider(Ctrl *ctrl, Event *e)
{
    const EvSlider *params = event_params(e, EvSlider);
    uint32_t total = model_count(ctrl->model);
    uint32_t selected = 0;
    if (total > 0)
        selected = (uint32_t)((real32_t)(total - 1) * params->pos);

    if (selected != ctrl->selected)
    {
        ctrl->selected = selected;
        i_update_product(ctrl);
    }
}

/*---------------------------------------------------------------------------*/

void ctrl_slider_cell(Ctrl *ctrl, Cell *cell)
{
    Slider *slider = cell_slider(cell);
    slider_OnMoved(slider, listener(ctrl, i_OnSlider, Ctrl));
    ctrl->slider_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_counter_cell(Ctrl *ctrl, Cell *cell)
{
    ctrl->counter_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_type_cell(Ctrl *ctrl, Cell *cell)
{
    model_type(cell);
    unref(ctrl);
}

/*---------------------------------------------------------------------------*/

void ctrl_code_cell(Ctrl *ctrl, Cell *cell)
{
    model_code(cell);
    ctrl->code_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_desc_cell(Ctrl *ctrl, Cell *cell)
{
    model_desc(cell);
    ctrl->desc_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_price_cell(Ctrl *ctrl, Cell *cell)
{
    model_price(cell);
    ctrl->price_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_user_cell(Ctrl *ctrl, Cell *cell)
{
    ctrl->user_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_pass_cell(Ctrl *ctrl, Cell *cell)
{
    ctrl->pass_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_login_panel(Ctrl *ctrl, Panel *panel)
{
    ctrl->login_panel = panel;
}

/*---------------------------------------------------------------------------*/

static UJson *i_user_webserv(const char_t *user, const char_t *pass, wserv_t *ret)
{
    Http *http = NULL;
    String *path = NULL;
    UJson *ujson = NULL;

    *ret = ekWS_OK;
    if (str_empty_c(user) || str_empty_c(pass))
    {
        *ret = ekWS_ACCESS;
        return NULL;
    }

    http = http_create("serv.nappgui.com", 80);
    path = str_printf("/duser.php?user=%s&pass=%s", user, pass);
    if (http_get(http, tc(path), NULL, 0, NULL) == TRUE)
    {
        uint32_t status = http_response_status(http);
        if (status >= 200 && status <= 299)
        {
            Stream *stm = stm_memory(4096);
            http_response_body(http, stm, NULL);
            ujson = json_read(stm, NULL, UJson);

            if (!ujson)
            {
                *ret = ekWS_JSON;
            }
            else if (ujson->code != 0)
            {
                json_destroy(&ujson, UJson);
                *ret = ekWS_ACCESS;
            }

            stm_close(&stm);
        }
        else
        {
            *ret = ekWS_ACCESS;
        }
    }

    str_destroy(&path);
    http_destroy(&http);
    return ujson;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_login_begin(Ctrl *ctrl)
{
    Edit *user = cell_edit(ctrl->user_cell);
    Edit *pass = cell_edit(ctrl->pass_cell);
    wserv_t ret = ekWS_OK;
    ctrl->ujson = i_user_webserv(edit_get_text(user), edit_get_text(pass), &ret);
    if (ctrl->ujson != NULL)
    {
        ret = model_webserv(ctrl->model);
        if (ret != ekWS_OK)
            json_destroy(&ctrl->ujson, UJson);
    }

    return (uint32_t)ret;
}

/*---------------------------------------------------------------------------*/

static void i_login_end(Ctrl *ctrl, const uint32_t rvalue)
{
    wserv_t ret = (wserv_t)rvalue;
    if (ret == ekWS_OK)
    {
        Layout *layout = panel_get_layout(ctrl->login_panel, 1);
        ImageView *view = layout_get_imageview(layout, 0, 0);
        Label *label0 = layout_get_label(layout, 0, 1);
        Label *label1 = layout_get_label(layout, 0, 2);
        window_defbutton(ctrl->window, NULL);
        imageview_image(view, ctrl->ujson->data.image64);
        label_text(label0, tc(ctrl->ujson->data.name));
        label_text(label1, tc(ctrl->ujson->data.mail));
        menuitem_enabled(ctrl->login_item, FALSE);
        menuitem_enabled(ctrl->logout_item, TRUE);
        menuitem_enabled(ctrl->import_item, TRUE);
        menuitem_enabled(ctrl->export_item, TRUE);
        panel_visible_layout(ctrl->login_panel, 1);
        ctrl->status = ekOK_LOGIN;
        ctrl->selected = 0;
        i_update_product(ctrl);
        json_destroy(&ctrl->ujson, UJson);
        cell_focus(ctrl->code_cell);
        panel_update(ctrl->login_panel);
    }
    else
    {
        cassert(ctrl->ujson == NULL);
        ctrl->status = ekERR_LOGIN;
        ctrl->err = ret;
    }

    i_status(ctrl);
}

/*---------------------------------------------------------------------------*/

static void i_OnLogin(Ctrl *ctrl, Event *e)
{
    if (ctrl->status != ekIN_LOGIN)
    {
        ctrl->status = ekIN_LOGIN;
        i_status(ctrl);
        osapp_task(ctrl, 0, i_login_begin, NULL, i_login_end, Ctrl);
    }
    
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_login_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnLogin, Ctrl));
    ctrl->login_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_login_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnLogin, Ctrl));
    ctrl->login_item = item;
}

/*---------------------------------------------------------------------------*/

static void i_OnLogout(Ctrl *ctrl, Event *e)
{
    Edit *edit0 = cell_edit(ctrl->user_cell);
    Edit *edit1 = cell_edit(ctrl->pass_cell);
    model_clear(ctrl->model);
    edit_text(edit0, "");
    edit_text(edit1, "");
    menuitem_enabled(ctrl->login_item, TRUE);
    menuitem_enabled(ctrl->logout_item, FALSE);
    menuitem_enabled(ctrl->import_item, FALSE);
    menuitem_enabled(ctrl->export_item, FALSE);
    ctrl->status = ekWAIT_LOGIN;
    panel_visible_layout(ctrl->login_panel, 0);
    i_update_product(ctrl);
    i_status(ctrl);
    cell_focus(ctrl->user_cell);
    panel_update(ctrl->login_panel);
    window_defbutton(ctrl->window, cell_button(ctrl->login_cell));
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_logout_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnLogout, Ctrl));
    ctrl->logout_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_logout_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnLogout, Ctrl));
    ctrl->logout_item = item;
}

/*---------------------------------------------------------------------------*/

static void i_OnSetting(Ctrl *ctrl, Event *e)
{
    state_t state = ekON;
    if (event_type(e) == ekEVBUTTON)
    {
        const EvButton *params = event_params(e, EvButton);
        state = params->state;
    }
    else
    {
        Button *button = cell_button(ctrl->setting_cell);
        cassert(event_type(e) == ekEVMENU);
        state = button_get_state(button);
        state = state == ekON ? ekOFF : ekON;
        button_state(button, state);
    }

    menuitem_state(ctrl->setting_item, state);
    layout_show_col(ctrl->main_layout, 1, state == ekON ? TRUE : FALSE);
    layout_update(ctrl->main_layout);
}

/*---------------------------------------------------------------------------*/

void ctrl_setting_cell(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnSetting, Ctrl));
    ctrl->setting_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_setting_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnSetting, Ctrl));
    ctrl->setting_item = item;
}

/*---------------------------------------------------------------------------*/

static void i_OnStats(Ctrl *ctrl, Event *e)
{
    const EvDraw *params = event_params(e, EvDraw);
    uint32_t i, n = sizeof(ctrl->stats) / sizeof(real32_t);
    real32_t p = 10.f, x = p, y0 = params->height - p;
    real32_t w = (params->width - p * 2) / n;
    real32_t h = params->height - p * 2;
    real32_t avg = 0, pavg;
    char_t tavg[16];
    color_t c[2];
    real32_t stop[2] = {0, 1};
    c[0] = kHOLDER;
    c[1] = gui_view_color();

    draw_fill_linear(params->ctx, c,stop, 2, 0, p, 0, params->height - p + 1);

    for (i = 0; i < n; ++i)
    {
        real32_t hr = h * (ctrl->stats[i] / i_MAX_STATS);
        real32_t y = p + h - hr;
        draw_rect(params->ctx, ekFILL, x, y, w - 2, hr);
        avg += ctrl->stats[i];
        x += w;
    }

    avg /= n;
    pavg = h * (avg / i_MAX_STATS);
    pavg = p + h - pavg;
    bstd_sprintf(tavg, sizeof(tavg), "%.2f", avg);
    draw_text_color(params->ctx, kTXTRED);
    draw_line_color(params->ctx, kTXTRED);
    draw_line(params->ctx, p - 2, pavg, params->width - p, pavg);
    draw_line_color(params->ctx, gui_label_color());
    draw_line(params->ctx, p - 2, y0 + 2, params->width - p, y0 + 2);
    draw_line(params->ctx, p - 2, y0 + 2, p - 2, p);
    draw_text(params->ctx, tavg, p, pavg);
}

/*---------------------------------------------------------------------------*/

void ctrl_stats_cell(Ctrl *ctrl, Cell *cell)
{
    View *view = cell_view(cell);
    view_OnDraw(view, listener(ctrl, i_OnStats, Ctrl));
    ctrl->stats_cell = cell;
}

/*---------------------------------------------------------------------------*/

static void i_OnLang(Ctrl *ctrl, Event *e)
{
    MenuItem *item = NULL;
    uint32_t lang_id = 0;
    static const char_t *LANGS[] = { "en_US", "es_ES", "pt_PT", "it_IT", "vi_VN", "ru_RU", "ja_JP" };
    if (event_type(e) == ekEVPOPUP)
    {
        const EvButton *params = event_params(e, EvButton);
        item = menu_get_item(ctrl->lang_menu, params->index);
        lang_id = params->index;
    }
    else
    {
        const EvMenu *params = event_params(e, EvMenu);
        PopUp *popup = cell_popup(ctrl->lang_cell);
        cassert(event_type(e) == ekEVMENU);
        popup_selected(popup, params->index);
        item = event_sender(e, MenuItem);
        lang_id = params->index;
    }

    menu_off_items(ctrl->lang_menu);
    menuitem_state(item, ekON);
    gui_language(LANGS[lang_id]);
}

/*---------------------------------------------------------------------------*/

void ctrl_lang_cell(Ctrl *ctrl, Cell *cell)
{
    PopUp *popup = cell_popup(cell);
    popup_OnSelect(popup, listener(ctrl, i_OnLang, Ctrl));
    ctrl->lang_cell = cell;
}

/*---------------------------------------------------------------------------*/

void ctrl_lang_menu(Ctrl *ctrl, Menu *menu)
{
    uint32_t i, n = menu_size(menu);        
    for (i = 0; i < n; ++i)
    {
        MenuItem *item = menu_get_item(menu, i);
        menuitem_OnClick(item, listener(ctrl, i_OnLang, Ctrl));
    }
    ctrl->lang_menu = menu;    
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(Ctrl *ctrl, Event *e)
{
    osapp_finish();
    unref(ctrl);
    unref(e);
}

/*---------------------------------------------------------------------------*/

void ctrl_exit_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnExit, Ctrl));
}

/*---------------------------------------------------------------------------*/

static void i_OnAbout(Ctrl *ctrl, Event *e)
{
    unref(ctrl);
    unref(e);
    osapp_open_url("https://nappgui.com/en/demo/products.html");
}

/*---------------------------------------------------------------------------*/

void ctrl_about_item(Ctrl *ctrl, MenuItem *item)
{
    menuitem_OnClick(item, listener(ctrl, i_OnAbout, Ctrl));
}

/*---------------------------------------------------------------------------*/

void ctrl_window(Ctrl *ctrl, Window *window)
{
    window_OnClose(window, listener(ctrl, i_OnExit, Ctrl));
    ctrl->window = window;
}

/*---------------------------------------------------------------------------*/

void ctrl_theme_images(Ctrl *ctrl)
{
    bool_t dark = gui_dark_mode();
    button_image(cell_button(ctrl->first_cell), (const Image*)(dark ? FIRSTD_PNG : FIRST_PNG));
    button_image(cell_button(ctrl->back_cell), (const Image*)(dark ? BACKD_PNG : BACK_PNG));
    button_image(cell_button(ctrl->next_cell), (const Image*)(dark ? NEXTD_PNG : NEXT_PNG));
    button_image(cell_button(ctrl->last_cell), (const Image*)(dark ? LASTD_PNG : LAST_PNG));
    button_image(cell_button(ctrl->add_cell), (const Image*)ADD_PNG);
    button_image(cell_button(ctrl->minus_cell), (const Image*)MINUS_PNG);
    button_image(cell_button(ctrl->setting_cell), (const Image*)SETTINGS_PNG);
    button_image(cell_button(ctrl->login_cell), (const Image*)LOGIN16_PNG);
    button_image(cell_button(ctrl->logout_cell), (const Image*)(dark ? LOGOUT16D_PNG : LOGOUT16_PNG));
    menuitem_image(ctrl->import_item, (const Image*)OPEN_PNG);
    menuitem_image(ctrl->export_item, (const Image*)(dark ? SAVED_PNG : SAVE_PNG));
    menuitem_image(ctrl->first_item, (const Image*)(dark ? FIRST16D_PNG : FIRST16_PNG));
    menuitem_image(ctrl->back_item, (const Image*)(dark ? BACK16D_PNG : BACK16_PNG));
    menuitem_image(ctrl->next_item, (const Image*)(dark ? NEXT16D_PNG : NEXT16_PNG));
    menuitem_image(ctrl->last_item, (const Image*)(dark ? LAST16D_PNG : LAST16_PNG));
    menuitem_image(ctrl->login_item, (const Image*)LOGIN16_PNG);
    menuitem_image(ctrl->logout_item, (const Image*)(dark ? LOGOUT16D_PNG : LOGOUT16_PNG));
}
