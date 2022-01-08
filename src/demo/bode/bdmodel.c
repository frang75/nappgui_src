/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdmodel.c
 *
 */

/* Bode Model */

#include "bdmodel.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

void model_dbind(void)
{
    dbind(Params, real32_t, P[0]);
    dbind(Params, real32_t, P[1]);
    dbind(Params, real32_t, P[2]);
    dbind(Params, real32_t, P[3]);
    dbind(Params, real32_t, P[4]);
    dbind(Params, real32_t, Q[0]);
    dbind(Params, real32_t, Q[1]);
    dbind(Params, real32_t, Q[2]);
    dbind(Params, real32_t, Q[3]);
    dbind(Params, real32_t, Q[4]);
    dbind(Params, real32_t, Q[5]);
    dbind(Params, real32_t, Q[6]);
    dbind(Params, real32_t, Q[7]);
    dbind(Params, real32_t, Q[8]);
    dbind(Params, real32_t, KRg[0]);
    dbind(Params, real32_t, KRg[1]);
    dbind(Params, real32_t, KRg[2]);
    dbind(Params, real32_t, KRg[3]);
    dbind(Params, real32_t, KRg[4]);
    dbind(Params, real32_t, KRg[5]);
    dbind(Params, real32_t, K[0]);
    dbind(Params, real32_t, K[1]);
    dbind(Params, real32_t, K[2]);
    dbind(Params, real32_t, T);
    dbind(Params, real32_t, R);
    dbind(Model, real32_t, wpos.x);
    dbind(Model, real32_t, wpos.y);
    dbind(Model, real32_t, wsize.width);
    dbind(Model, real32_t, wsize.height);
    dbind(Model, Params, cparams);
    dbind(Model, Params, sparams);

    dbind_default(Params, real32_t, P[0], 100);
    dbind_default(Params, real32_t, P[1], 1);
    dbind_default(Params, real32_t, P[2], -1);
    dbind_default(Params, real32_t, P[3], 0);
    dbind_default(Params, real32_t, P[4], 0);
    dbind_default(Params, real32_t, Q[0], 1);
    dbind_default(Params, real32_t, Q[1], 1);
    dbind_default(Params, real32_t, Q[2], -2);
    dbind_default(Params, real32_t, Q[3], 6);
    dbind_default(Params, real32_t, Q[4], -8);
    dbind_default(Params, real32_t, Q[5], 0);
    dbind_default(Params, real32_t, Q[6], 0);
    dbind_default(Params, real32_t, Q[7], 0);
    dbind_default(Params, real32_t, Q[8], 0);
    dbind_default(Params, real32_t, KRg[0], 0);
    dbind_default(Params, real32_t, KRg[1], 10);
    dbind_default(Params, real32_t, KRg[2], 1);
    dbind_default(Params, real32_t, KRg[3], 10);
    dbind_default(Params, real32_t, KRg[4], 0);
    dbind_default(Params, real32_t, KRg[5], 10);
    dbind_default(Params, real32_t, K[0], 1);
    dbind_default(Params, real32_t, K[1], 1);
    dbind_default(Params, real32_t, K[2], 1);
    dbind_default(Params, real32_t, T, 1);
    dbind_default(Params, real32_t, R, 0);
    dbind_default(Model, real32_t, wpos.x, 0);
    dbind_default(Model, real32_t, wpos.y, 0);
    dbind_default(Model, real32_t, wsize.width, 640);
    dbind_default(Model, real32_t, wsize.height, 480);

    dbind_range(Params, real32_t, P[0], -1000, 1000);
    dbind_range(Params, real32_t, P[1], -1000, 1000);
    dbind_range(Params, real32_t, P[2], -1000, 1000);
    dbind_range(Params, real32_t, P[3], -1000, 1000);
    dbind_range(Params, real32_t, P[4], -1000, 1000);
    dbind_range(Params, real32_t, Q[0], -1000, 1000);
    dbind_range(Params, real32_t, Q[1], -1000, 1000);
    dbind_range(Params, real32_t, Q[2], -1000, 1000);
    dbind_range(Params, real32_t, Q[3], -1000, 1000);
    dbind_range(Params, real32_t, Q[4], -1000, 1000);
    dbind_range(Params, real32_t, Q[5], -1000, 1000);
    dbind_range(Params, real32_t, Q[6], -1000, 1000);
    dbind_range(Params, real32_t, Q[7], -1000, 1000);
    dbind_range(Params, real32_t, Q[8], -1000, 1000);
    dbind_range(Params, real32_t, KRg[0], -100, 100);
    dbind_range(Params, real32_t, KRg[1], -100, 100);
    dbind_range(Params, real32_t, KRg[2], 1, 100);
    dbind_range(Params, real32_t, KRg[3], -100, 100);
    dbind_range(Params, real32_t, KRg[4], -100, 100);
    dbind_range(Params, real32_t, KRg[5], -100, 100);
    dbind_range(Params, real32_t, K[0], 0, 10);
    dbind_range(Params, real32_t, K[1], 1, 10);
    dbind_range(Params, real32_t, K[2], 0, 10);
    dbind_range(Params, real32_t, T, -100, 100);
    dbind_range(Params, real32_t, R, -100, 100);

    dbind_precision(Params, real32_t, P[0], .01f);
    dbind_precision(Params, real32_t, P[1], .01f);
    dbind_precision(Params, real32_t, P[2], .01f);
    dbind_precision(Params, real32_t, P[3], .01f);
    dbind_precision(Params, real32_t, P[4], .01f);
    dbind_precision(Params, real32_t, Q[0], .01f);
    dbind_precision(Params, real32_t, Q[1], .01f);
    dbind_precision(Params, real32_t, Q[2], .01f);
    dbind_precision(Params, real32_t, Q[3], .01f);
    dbind_precision(Params, real32_t, Q[4], .01f);
    dbind_precision(Params, real32_t, Q[5], .01f);
    dbind_precision(Params, real32_t, Q[6], .01f);
    dbind_precision(Params, real32_t, Q[7], .01f);
    dbind_precision(Params, real32_t, Q[8], .01f);
    dbind_precision(Params, real32_t, KRg[0], .01f);
    dbind_precision(Params, real32_t, KRg[1], .01f);
    dbind_precision(Params, real32_t, KRg[2], .01f);
    dbind_precision(Params, real32_t, KRg[3], .01f);
    dbind_precision(Params, real32_t, KRg[4], .01f);
    dbind_precision(Params, real32_t, KRg[5], .01f);
    dbind_precision(Params, real32_t, K[0], .01f);
    dbind_precision(Params, real32_t, K[1], .01f);
    dbind_precision(Params, real32_t, K[2], .01f);
    dbind_precision(Params, real32_t, T, .001f);
    dbind_precision(Params, real32_t, R, .001f);
}

/*---------------------------------------------------------------------------*/

static void i_k_range(const Model *model)
{
    dbind_default(Params, real32_t, K[0], model->cparams.KRg[0]);
    dbind_default(Params, real32_t, K[1], model->cparams.KRg[2]);
    dbind_default(Params, real32_t, K[2], model->cparams.KRg[4]);
    dbind_range(Params, real32_t, K[0], model->cparams.KRg[0], model->cparams.KRg[1]);
    dbind_range(Params, real32_t, K[1], model->cparams.KRg[2], model->cparams.KRg[3]);
    dbind_range(Params, real32_t, K[2], model->cparams.KRg[4], model->cparams.KRg[5]);
}

/*---------------------------------------------------------------------------*/

void model_default(Model *model)
{
    dbind_default(Params, real32_t, K[0], 1);
    dbind_default(Params, real32_t, K[1], 1);
    dbind_default(Params, real32_t, K[2], 1);
    dbind_range(Params, real32_t, K[0], 0, 10);
    dbind_range(Params, real32_t, K[1], 1, 10);
    dbind_range(Params, real32_t, K[2], 0, 10);
    dbind_init(&model->cparams, Params);
}

/*---------------------------------------------------------------------------*/

Model *model_read(void)
{
    String *file = hfile_appdata("bconfig");
    Stream *stm = stm_from_file(tc(file), NULL);
    Model *model = NULL;

    if (stm != NULL)
    {
        model = dbind_read(stm, Model);
        if (model != NULL && stm_state(stm) != ekSTOK)
            dbind_destroy(&model, Model);
        stm_close(&stm);
    }

    if (model == NULL)
        model = dbind_create(Model);
    else
        i_k_range(model);

    str_destroy(&file);
    return model;
}

/*---------------------------------------------------------------------------*/

void model_save(Model *model, Window *window)
{
    String *file = hfile_appdata("bconfig");
    Stream *stm = stm_to_file(tc(file), NULL);
    if (stm != NULL)
    {
        cassert_no_null(model);
        model->wpos = window_get_origin(window);
        model->wsize = window_get_client_size(window);
        dbind_write(stm, model, Model);
        stm_close(&stm);
    }

    str_destroy(&file);
}

/*---------------------------------------------------------------------------*/

bool_t model_validate(Model *model)
{
    bool_t ok = TRUE;
    uint32_t i;

    /* At lest, one P coeff must be != 0 */
    for (i = 0; i < 5; ++i)
    {
        if (bmath_absf(model->cparams.P[i]) > .001f)
            break;
    }

    /* Slider ranges should be ascendent */
    if (i < 5)
    {
        if (model->cparams.KRg[0] >= model->cparams.KRg[1])
            ok = FALSE;
        if (model->cparams.KRg[2] >= model->cparams.KRg[3])
            ok = FALSE;
        if (model->cparams.KRg[4] >= model->cparams.KRg[5])
            ok = FALSE;
    }
    else
    {
        ok = FALSE;
    }

    if (ok == TRUE)
        i_k_range(model);

    return ok;
}
