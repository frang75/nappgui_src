/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: imageview.c
 *
 */

/* Image view */

#include "imageview.h"
#include "view.h"
#include "view.inl"
#include "cell.inl"
#include "gui.inl"
#include "component.inl"
#include "layout.h"
#include <draw2d/image.h>
#include <draw2d/color.h>
#include <draw2d/draw.h>
#include <draw2d/dctx.h>
#include <geom2d/s2d.h>
#include <geom2d/t2d.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/objh.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

typedef struct _vimgdata_t VImgData;

struct _vimgdata_t
{
    Image *image;
    uint32_t frame;
    real64_t ftime;
    S2Df size;
    gui_scale_t scale;
    bool_t mouse_over;
    Listener *OnOverDraw;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(VImgData **data)
{
    cassert_no_null(data);
    cassert_no_null(data);
    ptr_destopt(image_destroy, &(*data)->image, Image);
    listener_destroy(&(*data)->OnOverDraw);
    heap_delete(data, VImgData);
}

/*---------------------------------------------------------------------------*/

static void i_image_transform(T2Df *t2d, const gui_scale_t scale, const real32_t view_width, const real32_t view_height, const real32_t img_width, const real32_t img_height)
{
    real32_t ratio_x = 1.f;
    real32_t ratio_y = 1.f;
    switch (scale)
    {
    case ekGUI_SCALE_NONE:
    case ekGUI_SCALE_ADJUST:
        ratio_x = 1.f;
        ratio_y = 1.f;
        break;

    case ekGUI_SCALE_AUTO:
        ratio_x = view_width / img_width;
        ratio_y = view_height / img_height;
        break;

    case ekGUI_SCALE_ASPECT:
        ratio_x = view_width / img_width;
        ratio_y = view_height / img_height;
        if (ratio_x < ratio_y)
            ratio_y = ratio_x;
        else
            ratio_x = ratio_y;
        break;

    case ekGUI_SCALE_ASPECTDW:
        ratio_x = view_width / img_width;
        ratio_y = view_height / img_height;
        if (ratio_x < ratio_y)
            ratio_y = ratio_x;
        else
            ratio_x = ratio_y;

        if (ratio_x > 1.f)
        {
            ratio_x = 1.f;
            ratio_y = 1.f;
        }
        break;
        cassert_default();
    }

    {
        real32_t fimg_width = bmath_roundf(ratio_x * img_width);
        real32_t fimg_height = bmath_roundf(ratio_y * img_height);
        real32_t fx = bmath_floorf(.5f * (view_width - fimg_width));
        real32_t fy = bmath_floorf(.5f * (view_height - fimg_height));
        t2d_movef(t2d, kT2D_IDENTf, fx, fy);
        t2d_scalef(t2d, t2d, ratio_x, ratio_y);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(View *view, Event *e)
{
    VImgData *data = NULL;
    const EvDraw *params = NULL;
    cassert_no_null(view);
    cassert(event_type(e) == ekGUI_EVENT_DRAW);
    data = view_get_data(view, VImgData);
    cassert_no_null(data);
    params = event_params(e, EvDraw);
    cassert_no_null(params);

    if (data->image != NULL)
    {
        T2Df t2d;
        real32_t w = (real32_t)image_width(data->image);
        real32_t h = (real32_t)image_height(data->image);
        i_image_transform(&t2d, data->scale, params->width, params->height, w, h);
        draw_matrixf(params->ctx, &t2d);
        draw_image_frame(params->ctx, data->image, data->frame, 0, 0);
    }

    if (data->OnOverDraw != NULL && data->mouse_over == TRUE)
    {
        draw_matrixf(params->ctx, kT2D_IDENTf);
        listener_pass_event(data->OnOverDraw, e, view, View);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnAcceptFocus(View *view, Event *e)
{
    bool_t *r = event_result(e, bool_t);
    unref(view);
    *r = FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_apply_size(ImageView *view, VImgData *data)
{
    cassert_no_null(data);
    if (data->scale == ekGUI_SCALE_ADJUST && data->image != NULL)
    {
        S2Df size;
        size.width = (real32_t)image_width(data->image);
        size.height = (real32_t)image_height(data->image);
        view_size(cast(view, View), size);
    }
    else
    {
        view_size(cast(view, View), data->size);
    }
}

/*---------------------------------------------------------------------------*/

ImageView *imageview_create(void)
{
    VImgData *data = heap_new0(VImgData);
    View *view = NULL;
    data->frame = UINT32_MAX;
    data->scale = ekGUI_SCALE_NONE;
    data->size = s2df(64.f, 64.f);
    view = view_create();
    i_apply_size(cast(view, ImageView), data);
    view_data(view, &data, i_destroy_data, VImgData);
    view_OnDraw(view, listener(view, i_OnDraw, View));
    view_OnAcceptFocus(view, listener(view, i_OnAcceptFocus, View));
    _view_set_subtype(view, "ImageView");
    _view_OnImage(view, (FPtr_set_image)imageview_image);
    return cast(view, ImageView);
}

/*---------------------------------------------------------------------------*/

void imageview_size(ImageView *view, S2Df size)
{
    VImgData *data = view_get_data(cast(view, View), VImgData);
    cassert_no_null(data);
    data->size = size;
    i_apply_size(view, data);
}

/*---------------------------------------------------------------------------*/

void imageview_scale(ImageView *view, const gui_scale_t scale)
{
    VImgData *data = view_get_data(cast(view, View), VImgData);
    cassert_no_null(data);
    data->scale = scale;
    i_apply_size(view, data);
}

/*---------------------------------------------------------------------------*/

static void i_OnAnimation(View *view, Event *event)
{
    const EvTransition *params = event_params(event, EvTransition);
    VImgData *data = view_get_data(view, VImgData);
    cassert_no_null(data);
    if (params->crtime > data->ftime)
    {
        uint32_t n = image_num_frames(data->image);
        real32_t l = 0.f;
        data->frame += 1;
        if (data->frame >= n)
            data->frame = 0;
        l = image_frame_length(data->image, data->frame);
        data->ftime = params->crtime + (real64_t)l;
        view_update(view);
    }
}

/*---------------------------------------------------------------------------*/

void imageview_image(ImageView *view, const Image *image)
{
    VImgData *data = view_get_data(cast(view, View), VImgData);
    const Image *limage = _gui_respack_image((const ResId)image, NULL);
    cassert_no_null(data);
    if (data->image != limage)
    {
        Cell *cell = _component_cell(cast(view, GuiComponent));
        ptr_destopt(image_destroy, &data->image, Image);
        data->image = ptr_copyopt(image_copy, limage, Image);
        data->frame = UINT32_MAX;
        _view_delete_transition(cast(view, View));

        if (limage != NULL)
        {
            uint32_t num_frames = image_num_frames(data->image);
            if (num_frames > 1)
            {
                data->frame = 0;
                data->ftime = -1.;
                _view_add_transition(cast(view, View), obj_listener(cast(view, View), i_OnAnimation, View));
            }
        }

        i_apply_size(view, data);
        view_update(cast(view, View));

        if (cell != NULL)
            _cell_update_image(cell, limage);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnEnter(View *view, Event *e)
{
    VImgData *data = view_get_data(view, VImgData);
    data->mouse_over = TRUE;
    view_update(view);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(View *view, Event *e)
{
    VImgData *data = view_get_data(view, VImgData);
    data->mouse_over = FALSE;
    view_update(view);
    unref(e);
}

/*---------------------------------------------------------------------------*/

void imageview_OnClick(ImageView *view, Listener *listener)
{
    view_OnClick(cast(view, View), listener);
}

/*---------------------------------------------------------------------------*/

void imageview_OnOverDraw(ImageView *view, Listener *listener)
{
    VImgData *data = view_get_data(cast(view, View), VImgData);
    cassert_no_null(data);
    listener_update(&data->OnOverDraw, listener);
    view_OnEnter(cast(view, View), listener(cast(view, View), i_OnEnter, View));
    view_OnExit(cast(view, View), listener(cast(view, View), i_OnExit, View));
}
