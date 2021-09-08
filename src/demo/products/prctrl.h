/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: prctrl.h
 * https://nappgui.com/en/demo/products/prctrl.html
 *
 */

/* Products Controller */

#include "products.hxx"

Ctrl *ctrl_create(Model *model);

void ctrl_destroy(Ctrl **ctrl);

void ctrl_run(Ctrl *ctrl);

void ctrl_main_layout(Ctrl *ctrl, Layout *layout);

void ctrl_status_layout(Ctrl *ctrl, Layout *layout);

void ctrl_import_item(Ctrl *ctrl, MenuItem *item);

void ctrl_export_item(Ctrl *ctrl, MenuItem *item);

void ctrl_image_cell(Ctrl *ctrl, Cell *cell);

void ctrl_first_cell(Ctrl *ctrl, Cell *cell);

void ctrl_first_item(Ctrl *ctrl, MenuItem *item);

void ctrl_back_cell(Ctrl *ctrl, Cell *cell);

void ctrl_back_item(Ctrl *ctrl, MenuItem *item);

void ctrl_next_cell(Ctrl *ctrl, Cell *cell);

void ctrl_next_item(Ctrl *ctrl, MenuItem *item);

void ctrl_last_cell(Ctrl *ctrl, Cell *cell);

void ctrl_last_item(Ctrl *ctrl, MenuItem *item);

void ctrl_add_cell(Ctrl *ctrl, Cell *cell);

void ctrl_minus_cell(Ctrl *ctrl, Cell *cell);

void ctrl_filter_cell(Ctrl *ctrl, Cell *cell);

void ctrl_slider_cell(Ctrl *ctrl, Cell *cell);

void ctrl_counter_cell(Ctrl *ctrl, Cell *cell);

void ctrl_type_cell(Ctrl *ctrl, Cell *cell);

void ctrl_code_cell(Ctrl *ctrl, Cell *cell);

void ctrl_desc_cell(Ctrl *ctrl, Cell *cell);

void ctrl_price_cell(Ctrl *ctrl, Cell *cell);

void ctrl_user_cell(Ctrl *ctrl, Cell *cell);

void ctrl_pass_cell(Ctrl *ctrl, Cell *cell);

void ctrl_login_panel(Ctrl *ctrl, Panel *panel);

void ctrl_login_cell(Ctrl *ctrl, Cell *cell);

void ctrl_login_item(Ctrl *ctrl, MenuItem *item);

void ctrl_logout_cell(Ctrl *ctrl, Cell *cell);

void ctrl_logout_item(Ctrl *ctrl, MenuItem *item);

void ctrl_setting_cell(Ctrl *ctrl, Cell *cell);

void ctrl_setting_item(Ctrl *ctrl, MenuItem *item);

void ctrl_stats_cell(Ctrl *ctrl, Cell *cell);

void ctrl_lang_cell(Ctrl *ctrl, Cell *cell);

void ctrl_lang_menu(Ctrl *ctrl, Menu *menu);

void ctrl_exit_item(Ctrl *ctrl, MenuItem *item);

void ctrl_about_item(Ctrl *ctrl, MenuItem *item);

void ctrl_window(Ctrl *ctrl, Window *window);

void ctrl_theme_images(Ctrl *ctrl);
