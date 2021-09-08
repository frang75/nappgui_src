/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
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
#include "image.h"
#include "layout.h"
#include "obj.inl"

#include "bmath.h"
#include "cassert.h"
#include "color.h"
#include "draw.h"
#include "dctx.h"
#include "event.h"
#include "heap.h"
#include "ptr.h"
#include "s2d.h"
#include "t2d.h"

typedef struct _vimgdata_t VImgData;

struct _vimgdata_t
{
    Image *image;
    uint32_t frame;
    real64_t ftime;
    scale_t scale;
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

static void i_image_transform(T2Df *t2d, const scale_t scale, const real32_t view_width, const real32_t view_height, const real32_t img_width, const real32_t img_height)
{
    real32_t ratio = 1.f;    
    if (scale != ekSNONE)
    {
        real32_t ratio1 = 1.f, ratio2 = 1.f;
        ratio1 = view_width / img_width;
        ratio2 = view_height / img_height;
        ratio = (ratio1 < ratio2) ? ratio1 : ratio2;

        switch (scale)
        {
        case ekAUTO:
        case ekASPECTDW:
            if (ratio > 1.f)
                ratio = 1.f;
            break;
        case ekASPECT:
            break;

        case ekSNONE:
        cassert_default();
        }
    }

    {
        real32_t fimg_width = bmath_roundf(ratio * img_width);
        real32_t fimg_height = bmath_roundf(ratio * img_height);
        real32_t fx = bmath_floorf(.5f * (view_width - fimg_width));
        real32_t fy = bmath_floorf(.5f * (view_height - fimg_height));
        //cassert(fimg_width <= view_width);
        //cassert(fimg_height <= view_height);
        t2d_movef(t2d, kT2D_IDENTf, fx, fy);
        t2d_scalef(t2d, t2d, ratio, ratio);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnRedraw(View *view, Event *e)
{
    VImgData *data = NULL;
    const EvDraw *params = NULL;
    cassert_no_null(view);
    cassert(event_type(e) == ekEVDRAW);
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

ImageView *imageview_create(void)
{
    VImgData *data = heap_new0(VImgData);
    S2Df size = { 64.f, 64.f };
    View *view = NULL;
    data->frame = UINT32_MAX;
    data->scale = ekSNONE;
    view = view_create();
    view_data(view, &data, i_destroy_data, VImgData);
    _view_set_subtype(view, "ImageView");
    view_size(view, size);
    view_OnDraw(view, listener(view, i_OnRedraw, View));
    view_OnImage(view, (FPtr_set_image)imageview_image);
    return (ImageView*)view;
}

/*---------------------------------------------------------------------------*/

void imageview_size(ImageView *view, S2Df size)
{
    view_size((View*)view, size);
}

/*---------------------------------------------------------------------------*/

void imageview_scale(ImageView *view, const scale_t scale)
{
    VImgData *data = view_get_data((View*)view, VImgData);
    cassert_no_null(data);
    data->scale = scale;
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
    VImgData *data = view_get_data((View*)view, VImgData);
    const Image *limage = _gui_respack_image((const ResId)image, NULL);
    cassert_no_null(data);
    if (data->image != limage)
    {
        ptr_destopt(image_destroy, &data->image, Image);
        data->image = ptr_copyopt(image_copy, limage, Image);
        data->frame = UINT32_MAX;
        view_delete_transition((View*)view);
        
        if (limage != NULL)
        {
            uint32_t num_frames = image_num_frames(data->image);
            if (num_frames > 1)
            {
                data->frame = 0;
                data->ftime = -1.;
                view_add_transition((View*)view, obj_listener((View*)view, i_OnAnimation, View));
            }

            if (data->scale == ekAUTO)
            {
                S2Df size;
                size.width = (real32_t)image_width(data->image);
                size.height = (real32_t)image_height(data->image);
                view_size((View*)view, size);
            }
        }

        view_update((View*)view);
        _cell_upd_image(((GuiComponent*)view)->parent, limage);
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
    view_OnClick((View*)view, listener);
}

/*---------------------------------------------------------------------------*/

void imageview_OnOverDraw(ImageView *view, Listener *listener)
{
    VImgData *data = view_get_data((View*)view, VImgData);
    cassert_no_null(data);
    listener_update(&data->OnOverDraw, listener);
    view_OnEnter((View*)view, listener((View*)view, i_OnEnter, View));
    view_OnExit((View*)view, listener((View*)view, i_OnExit, View));
}

/*---------------------------------------------------------------------------*/

/*void imageview_frame(CView *view, const uint32_t frame);
void imageview_frame(CView *view, const uint32_t frame)
{
    i_Data *data = NULL;
    cassert_no_null(view);
    data = view_data(view, i_Data);
    cassert_no_null(data);
    data->frame = frame;    
}*/

/*---------------------------------------------------------------------------*/

/*
uint32_t imageview_get_frame(const CView *view);
uint32_t imageview_get_frame(const CView *view)
{
    i_Data *data = NULL;
    cassert_no_null(view);
    data = view_data(view, i_Data);
    cassert_no_null(data);
    return data->frame;
}*/

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

/*
void imageview_animation(CView *view, const bool_t animation, Listener *listener);
void imageview_animation(CView *view, const bool_t animation, Listener *listener)
{
    i_Data *data = NULL;
    cassert_no_null(view);
    data = view_data(view, i_Data);
    cassert_no_null(data);
    listener_update(&data->OnTransition, listener);
    view_unregister_transition(view);
    
    if (animation == TRUE)
    {
        uint32_t num_frames = 0;
        num_frames = image_num_frames(data->image);
        if (num_frames > 0)
        {
            if (data->frame == UINT32_MAX)
                data->frame = 0;
            data->frame_time_stamp = 0.;
            view_register_transition(view, obj_listener(view, i_OnAnimation, CView));            
        }
    }
}*/


