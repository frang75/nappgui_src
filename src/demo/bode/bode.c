/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bode.c
 *
 */

/* Bode Application */

#include "bdcalc.h"
#include "bdctrl.h"
#include "bdmodel.h"
#include "bdview.h"
#include "bdplot.h"
#include "nappgui.h"

typedef struct _app_t App;

struct _app_t
{
	Model *model;
    Plot *plot;
	Ctrl *ctrl;
	Window *window;
};

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
    App *app = heap_new(App);
	model_dbind();
	app->model = model_read();
    app->plot = plot_create(bode_npoints(), bode_sim_npoints());
    app->ctrl = ctrl_create(app->model, app->plot);
	app->window = bdview_create(app->ctrl);
    ctrl_run(app->ctrl);
    window_origin(app->window, app->model->wpos);
    window_size(app->window, s2df(700, 400));//app->model->wsize);
	window_OnClose(app->window, listener(app, i_OnClose, App));
	window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
	cassert_no_null(app);
	cassert_no_null(*app);
    model_save((*app)->model, (*app)->window);
    ctrl_destroy(&(*app)->ctrl);
    plot_destroy(&(*app)->plot);
	window_destroy(&(*app)->window);
    dbind_destroy(&(*app)->model, Model);
	heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
