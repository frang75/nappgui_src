/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdctrl.h
 *
 */

/* Bode Ctrl */

#include "bode.hxx"

Ctrl* ctrl_create(Model *model, Plot *plot);

void ctrl_destroy(Ctrl **ctrl);

void ctrl_layout(Ctrl *ctrl, Layout *layout);

void ctrl_reset(Ctrl *ctrl, Button *button);

void ctrl_take(Ctrl *ctrl, Cell *cell);

void ctrl_restore(Ctrl *ctrl, Cell *cell);

void ctrl_clear(Ctrl *ctrl, Cell *cell);

void ctrl_info(Ctrl *ctrl, Button *button);

void ctrl_view1(Ctrl *ctrl, View *view);

void ctrl_view2(Ctrl *ctrl, View *view);

void ctrl_slider1(Ctrl *ctrl, Cell *cell);

void ctrl_run(Ctrl *ctrl);

void ctrl_OnModelChange(Ctrl *ctrl, Event *e);
