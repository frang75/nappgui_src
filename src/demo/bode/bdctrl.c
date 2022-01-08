/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdctrl.c
 *
 */

/* Bode Ctrl */

#include "bdctrl.h"
#include "bdmodel.h"
#include "bdcalc.h"
#include "bdplot.h"
#include "nappgui.h"

struct _ctrl_t
{
    Model *model;
    Plot *plot;
    Layout *layout;
    Cell *restore;
    Cell *clear;
    Cell *slider1;
    View *view1;
    View *view2;
};

/*---------------------------------------------------------------------------*/

Ctrl* ctrl_create(Model *model, Plot *plot)
{
    Ctrl *ctrl = heap_new0(Ctrl);
    ctrl->model = model;
    ctrl->plot = plot;
    return ctrl;
}

/*---------------------------------------------------------------------------*/

void ctrl_destroy(Ctrl **ctrl)
{
    heap_delete(ctrl, Ctrl);
}

/*---------------------------------------------------------------------------*/

static void i_snap(Ctrl *ctrl, const bool_t snap)
{
    cell_enabled(ctrl->restore, snap);
    cell_enabled(ctrl->clear, snap);
}

/*---------------------------------------------------------------------------*/

void ctrl_layout(Ctrl* ctrl, Layout* layout)
{
    ctrl->layout = layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnReset(Ctrl *ctrl, Event *e)
{
    unref(e);
    model_default(ctrl->model);
    layout_dbind_obj(ctrl->layout, ctrl->model, Model);
    i_snap(ctrl, FALSE);
    plot_clear_snap(ctrl->plot);
    view_update(ctrl->view1);
    view_update(ctrl->view2);
}

/*---------------------------------------------------------------------------*/

void ctrl_reset(Ctrl *ctrl, Button *button)
{
    button_OnClick(button, listener(ctrl, i_OnReset, Ctrl));
}

/*---------------------------------------------------------------------------*/

static void i_OnTake(Ctrl *ctrl, Event *e)
{
    unref(e);
    ctrl->model->sparams = ctrl->model->cparams;
    i_snap(ctrl, TRUE);
    plot_take_snap(ctrl->plot);
    view_update(ctrl->view1);
    view_update(ctrl->view2);
}

/*---------------------------------------------------------------------------*/

void ctrl_take(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnTake, Ctrl));
}

/*---------------------------------------------------------------------------*/

static void i_OnRestore(Ctrl *ctrl, Event *e)
{
    unref(e);
    ctrl->model->cparams = ctrl->model->sparams;
    i_snap(ctrl, FALSE);
    layout_dbind_obj(ctrl->layout, ctrl->model, Model);
    plot_restore_snap(ctrl->plot);
    view_update(ctrl->view1);
    view_update(ctrl->view2);
}

/*---------------------------------------------------------------------------*/

void ctrl_restore(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnRestore, Ctrl));
    ctrl->restore = cell;
}

/*---------------------------------------------------------------------------*/

static void i_OnClear(Ctrl *ctrl, Event *e)
{
    unref(e);
    i_snap(ctrl, FALSE);
    plot_clear_snap(ctrl->plot);
    view_update(ctrl->view1);
    view_update(ctrl->view2);
}

/*---------------------------------------------------------------------------*/

void ctrl_clear(Ctrl *ctrl, Cell *cell)
{
    Button *button = cell_button(cell);
    button_OnClick(button, listener(ctrl, i_OnClear, Ctrl));
    ctrl->clear = cell;
}

/*---------------------------------------------------------------------------*/

static void i_OnInfo(Ctrl *ctrl, Event *e)
{
    unref(ctrl);
    unref(e);
    osapp_open_url("https://nappgui.com/en/demo/bode.html");
}

/*---------------------------------------------------------------------------*/

void ctrl_info(Ctrl *ctrl, Button *button)
{
    button_OnClick(button, listener(ctrl, i_OnInfo, Ctrl));    
}

/*---------------------------------------------------------------------------*/

static void i_OnMove1(Ctrl *ctrl, Event *e)
{
    S2Df size;
    const EvMouse *p = event_params(e, EvMouse);
    view_get_size(ctrl->view1, &size);
    plot_mouse1_x(ctrl->plot, p->x / size.width);
    view_update(ctrl->view1);
}

/*---------------------------------------------------------------------------*/

static void i_OnMove2(Ctrl *ctrl, Event *e)
{
    S2Df size;
    const EvMouse *p = event_params(e, EvMouse);
    view_get_size(ctrl->view2, &size);
    plot_mouse2_x(ctrl->plot, p->x / size.width);
    view_update(ctrl->view2);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw1(Ctrl *ctrl, Event *e)
{
	const EvDraw *p = event_params(e, EvDraw);
    plot_draw_graph1(ctrl->plot, p->ctx, p->width, p->height);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw2(Ctrl *ctrl, Event *e)
{
	const EvDraw* p = event_params(e, EvDraw);
    plot_draw_graph2(ctrl->plot, p->ctx, p->width, p->height);
}

/*---------------------------------------------------------------------------*/

void ctrl_view1(Ctrl *ctrl, View *view)
{
    view_OnDraw(view, listener(ctrl, i_OnDraw1, Ctrl));
    view_OnMove(view, listener(ctrl, i_OnMove1, Ctrl));
    ctrl->view1 = view;	
}

/*---------------------------------------------------------------------------*/

void ctrl_view2(Ctrl *ctrl, View *view)
{
    view_OnDraw(view, listener(ctrl, i_OnDraw2, Ctrl));
    view_OnMove(view, listener(ctrl, i_OnMove2, Ctrl));
    ctrl->view2 = view;	
}

/*---------------------------------------------------------------------------*/

void ctrl_slider1(Ctrl *ctrl, Cell *cell)
{
    ctrl->slider1 = cell;
}

/*---------------------------------------------------------------------------*/

static void i_update_bode(Ctrl *ctrl, const real32_t T)
{
    uint32_t last_sim_i = UINT32_MAX;
    bode_set_P_coeffs(ctrl->model->cparams.P);
    bode_set_Q_coeffs(ctrl->model->cparams.Q);
    bode_set_K(ctrl->model->cparams.K);
    bode_set_T(ctrl->model->cparams.T);
    bode_set_R(ctrl->model->cparams.R);
    bode_update(&last_sim_i);
    plot_update(ctrl->plot, T, last_sim_i);
}

/*---------------------------------------------------------------------------*/

void ctrl_run(Ctrl *ctrl)
{
    i_snap(ctrl, FALSE);
    i_update_bode(ctrl, ctrl->model->cparams.T);
    layout_dbind_obj(ctrl->layout, ctrl->model, Model);
    cell_focus(ctrl->slider1);
}

/*---------------------------------------------------------------------------*/

void ctrl_OnModelChange(Ctrl *ctrl, Event *e)
{
	bool_t *ok = event_result(e, bool_t);
    cassert(evbind_object(e, Model) == ctrl->model);
	*ok = model_validate(ctrl->model);
	if (*ok == TRUE)
	{
        i_update_bode(ctrl, ctrl->model->cparams.T);
        view_update(ctrl->view1);
        view_update(ctrl->view2);
    }
}
