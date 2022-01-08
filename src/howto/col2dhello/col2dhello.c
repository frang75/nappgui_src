/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: col2dhello.c
 *
 */

/* 2D collision detection demo */

#include "nappgui.h"
#include "col2dgui.h"

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Tri2Df i_triangle(void)
{
    Tri2Df tri = tri2df(-3, 4, -1, -2, 7, -2);
    cassert(tri2d_ccwf(&tri) == TRUE);
    return tri;
}

/*---------------------------------------------------------------------------*/

static Pol2Df *i_convex_pol(void)
{
    V2Df pt[] = { {4,1}, {2,5}, {-3,5}, {-4,2}, {0,-3} };
    Pol2Df *pol = NULL;
    bmem_rev_elems(pt, sizeof(pt) / sizeof(V2Df), V2Df);
    pol = pol2d_createf(pt, sizeof(pt) / sizeof(V2Df));
    cassert(pol2d_convexf(pol) == TRUE);
    cassert(pol2d_ccwf(pol) == FALSE);
    return pol;
}

/*---------------------------------------------------------------------------*/

static Pol2Df *i_simple_pol(void)
{
    V2Df pt[] = { {9.78f, 12.17f}, {-10.00f, 11.01f}, {-9.68f, 3.20f}, {-9.30f, -5.98f}, {-4.27f, -5.84f}, {-4.03f, -12.17f}, {2.72f, -12.12f}, {2.47f, -6.36f}, {2.04f, 3.26f}, {-1.45f, 3.05f}, {-1.08f, -2.08f}, {-3.98f, -2.38f}, {-4.23f, 2.88f}, {-1.45f, 3.05f}, {2.04f, 3.26f}, {10.00f, 3.75f} };
    Pol2Df *pol = NULL;
    bmem_rev_elems(pt, sizeof(pt) / sizeof(V2Df), V2Df);
    pol = pol2d_createf(pt, sizeof(pt) / sizeof(V2Df));
    cassert(pol2d_convexf(pol) == FALSE);
    cassert(pol2d_ccwf(pol) == FALSE);
    return pol;
}

/*---------------------------------------------------------------------------*/

static Shape *i_new_shape(ArrSt(Shape) *shapes, const shtype_t type)
{
    Shape *shape = arrst_new(shapes, Shape);
    shape->type = type;
    shape->mouse = FALSE;
    shape->collisions = 0;
    return shape;
}

/*---------------------------------------------------------------------------*/

static void i_new_pnt(ArrSt(Shape) *shapes, const real32_t x, const real32_t y)
{
    Shape *shape = i_new_shape(shapes, ekPOINT);
    shape->body.pnt.x = x;
    shape->body.pnt.y = y;
}

/*---------------------------------------------------------------------------*/

static void i_new_cloud(ArrSt(Shape) *shapes, const real32_t x, const real32_t y, const real32_t w, const real32_t h, const real32_t a)
{
    Shape *shape = i_new_shape(shapes, ekPOINT_CLOUD);
    shape->body.cloud.pnts = arrst_create(V2Df);
    shape->body.cloud.center.x = x;
    shape->body.cloud.center.y = y;
    shape->body.cloud.width = w;
    shape->body.cloud.height = h;
    shape->body.cloud.angle = a;
    shape->body.cloud.ctype = 0;
    shape->body.cloud.type = 0;
    (void)arrst_new_n(shape->body.cloud.pnts, POINT_CLOUD_N, V2Df);
    col2dhello_update_cloud(&shape->body.cloud);
}

/*---------------------------------------------------------------------------*/

static void i_new_seg(ArrSt(Shape) *shapes, const real32_t x, const real32_t y, const real32_t l, const real32_t a)
{
    Shape *shape = i_new_shape(shapes, ekSEGMENT);
    shape->body.seg.center.x = x;
    shape->body.seg.center.y = y;
    shape->body.seg.length = l;
    shape->body.seg.angle = a;
    col2dhello_update_seg(&shape->body.seg);
}

/*---------------------------------------------------------------------------*/

static void i_new_cir(ArrSt(Shape) *shapes, const real32_t x, const real32_t y, const real32_t r)
{
    Shape *shape = i_new_shape(shapes, ekCIRCLE);
    shape->body.cir.r = r;
    shape->body.cir.c.x = x;
    shape->body.cir.c.y = y;
}

/*---------------------------------------------------------------------------*/

static void i_new_box(ArrSt(Shape) *shapes, const real32_t x, const real32_t y, const real32_t w, const real32_t h)
{
    Shape *shape = i_new_shape(shapes, ekBOX);
    shape->body.box.center.x = x;
    shape->body.box.center.y = y;
    shape->body.box.width = w;
    shape->body.box.height = h;
    col2dhello_update_box(&shape->body.box);
}

/*---------------------------------------------------------------------------*/

static void i_new_obb(ArrSt(Shape) *shapes, const real32_t x, const real32_t y, const real32_t w, const real32_t h, const real32_t a)
{
    Shape *shape = i_new_shape(shapes, ekOBB);
    shape->body.obb.center.x = x;
    shape->body.obb.center.y = y;
    shape->body.obb.angle = a;
    shape->body.obb.width = w;
    shape->body.obb.height = h;
    shape->body.obb.obb = NULL;
    col2dhello_update_obb(&shape->body.obb);
}

/*---------------------------------------------------------------------------*/

static void i_new_tri(ArrSt(Shape) *shapes, const real32_t x, const real32_t y, const real32_t a, const real32_t s)
{
    Shape *shape = i_new_shape(shapes, ekTRIANGLE);
    shape->body.tri.center.x = x;
    shape->body.tri.center.y = y;
    shape->body.tri.angle = a;
    shape->body.tri.scale = s;
    shape->body.tri.t2d = *kT2D_IDENTf;
    shape->body.tri.tri = i_triangle();
    col2dhello_update_tri(&shape->body.tri);
}

/*---------------------------------------------------------------------------*/

static void i_new_pol(ArrSt(Shape) *shapes, const shtype_t type, const real32_t x, const real32_t y, const real32_t a, const real32_t s)
{
    Shape *shape = i_new_shape(shapes, type);
    shape->body.pol.center.x = x;
    shape->body.pol.center.y = y;
    shape->body.pol.angle = a;
    shape->body.pol.scale = s;
    shape->body.pol.t2d = *kT2D_IDENTf;
    shape->body.pol.pol = type == ekCONVEX_POLY ? i_convex_pol() : i_simple_pol();
    col2dhello_update_pol(&shape->body.pol);
}

/*---------------------------------------------------------------------------*/

static ArrSt(Shape) *i_shapes(void) 
{
    ArrSt(Shape) *shapes = arrst_create(Shape);
    i_new_pnt(shapes, 520, 230);
    i_new_pnt(shapes, 220, 205);
    i_new_seg(shapes, 420, 280, 190, 125 * kBMATH_DEG2RADf);
    i_new_cir(shapes, 100, 100, 50);
    i_new_cir(shapes, 300, 200, 20);
    i_new_box(shapes, 100, 225, 100, 50);
    i_new_obb(shapes, 150, 350, 200, 20, 200 * kBMATH_DEG2RADf);
    i_new_obb(shapes, 460, 90, 200, 60, 15 * kBMATH_DEG2RADf);
    i_new_tri(shapes, 550, 475, 75 * kBMATH_DEG2RADf, 15);
    i_new_tri(shapes, 90, 480, 355 * kBMATH_DEG2RADf, 18);
    i_new_pol(shapes, ekCONVEX_POLY, 535, 325, 30 * kBMATH_DEG2RADf, 15);
    i_new_pol(shapes, ekSIMPLE_POLY, 370, 450, 45 * kBMATH_DEG2RADf, 7);
    return shapes;
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = heap_new0(App);
    col2dhello_dbind();
    app->shapes = i_shapes();
    app->dists = arrst_create(Dist);
    app->seltype = ekOBB;
    app->selshape = UINT32_MAX;
    app->show_seg_pt = TRUE;
    app->show_triangles = FALSE;
    app->show_convex_parts = FALSE;
    app->sel_area = 0;
    app->window = col2dhello_window(app);
    window_title(app->window, "2D Collision Detection");
    window_origin(app->window, v2df(500, 200));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    col2dhello_dbind_shape(app);
    col2dhello_collisions(app);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_remove_bounds(Cloud *cloud)
{
    cassert_no_null(cloud);
    switch(cloud->ctype) {
    case 0:
    case 1:
    case 2:
        break;
    case 3:
        obb2d_destroyf(&cloud->bound.obb);
        break;
    case 4:
        pol2d_destroyf(&cloud->bound.poly);
        break;
    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_remove_shape(Shape *shape)
{
    cassert_no_null(shape);
    switch(shape->type){
    case ekPOINT_CLOUD:
        arrst_destroy(&shape->body.cloud.pnts, NULL, V2Df);
        i_remove_bounds(&shape->body.cloud);
        break;
        
    case ekOBB:
        obb2d_destroyf(&shape->body.obb.obb);
        break;

    case ekCONVEX_POLY:
    case ekSIMPLE_POLY:
        pol2d_destroyf(&shape->body.pol.pol);
        break;

    case ekPOINT:
    case ekSEGMENT:
    case ekCIRCLE:
    case ekBOX:
    case ekTRIANGLE:
        break;

    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    arrst_destroy(&(*app)->shapes, i_remove_shape, Shape);
    arrst_destroy(&(*app)->dists, NULL, Dist);
    window_destroy(&(*app)->window);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

void col2dhello_new_shape(App *app, const V2Df pos)
{
    switch(app->seltype) {
    case ekPOINT:
        i_new_pnt(app->shapes, pos.x, pos.y);
        break;

    case ekPOINT_CLOUD:
        i_new_cloud(app->shapes, pos.x, pos.y, 100, 50, 15 * kBMATH_DEG2RADf);
        break;

    case ekSEGMENT:
        i_new_seg(app->shapes, pos.x, pos.y, 100, 15 * kBMATH_DEG2RADf);
        break;

    case ekCIRCLE:
        i_new_cir(app->shapes, pos.x, pos.y, 30);
        break;

    case ekBOX:
        i_new_box(app->shapes, pos.x, pos.y, 100, 50);
        break;

    case ekOBB:
        i_new_obb(app->shapes, pos.x, pos.y, 100, 50, 15 * kBMATH_DEG2RADf);
        break;

    case ekTRIANGLE:
        i_new_tri(app->shapes, pos.x, pos.y, 15 * kBMATH_DEG2RADf, 15);
        break;

    case ekCONVEX_POLY:
        i_new_pol(app->shapes, ekCONVEX_POLY, pos.x, pos.y, 0, 10);
        break;

    case ekSIMPLE_POLY:
        i_new_pol(app->shapes, ekSIMPLE_POLY, pos.x, pos.y, 0, 10);
        break;

    cassert_default();
    }

    app->selshape = arrst_size(app->shapes, Shape) - 1;
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_gui(App *app)
{
    cassert_no_null(app);
    if (app->selshape != UINT32_MAX)
    {
        Shape *shape = arrst_get(app->shapes, app->selshape, Shape);
        switch(shape->type) {
        case ekPOINT:
        case ekPOINT_CLOUD:
        case ekSEGMENT:
            app->sel_area = 0;
            break;

        case ekCIRCLE:
            app->sel_area = cir2d_areaf(&shape->body.cir);
            break;

        case ekBOX:
            app->sel_area = box2d_areaf(&shape->body.box.box);
            break;

        case ekOBB:
            app->sel_area = obb2d_areaf(shape->body.obb.obb);
            break;

        case ekTRIANGLE:
            app->sel_area = tri2d_areaf(&shape->body.tri.tri);
            break;

        case ekCONVEX_POLY:
        case ekSIMPLE_POLY:
            app->sel_area = pol2d_areaf(shape->body.pol.pol);
            break;

        cassert_default();
        }
    }
    else
    {
        app->sel_area = 0;
    }

    layout_dbind_obj(app->main_layout, app, App);
    panel_update(app->obj_panel);
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_seg(Seg *seg)
{
    V2Df hvec;
    cassert_no_null(seg);
    hvec.x = seg->length / 2;
    hvec.y = 0;
    v2d_rotatef(&hvec, seg->angle);
    seg->seg.p0.x = seg->center.x - hvec.x;
    seg->seg.p0.y = seg->center.y - hvec.y;
    seg->seg.p1.x = seg->center.x + hvec.x;
    seg->seg.p1.y = seg->center.y + hvec.y;
}

/*---------------------------------------------------------------------------*/

Box2Df col2dhello_cloud_box(const Cloud *cloud)
{
    Box2Df box = cloud->box;
    box.min = v2d_addf(&cloud->box.min, &cloud->center);
    box.max = v2d_addf(&cloud->box.max, &cloud->center);
    return box;
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_cloud(Cloud *cloud)
{
    V2Df *pt = NULL;
    uint32_t i, n;
    real32_t hw, hh;
    cassert_no_null(cloud);
    pt = arrst_all(cloud->pnts, V2Df);
    n = arrst_size(cloud->pnts, V2Df);
    hw = cloud->width / 2;
    hh = cloud->height / 2;

    for (i = 0; i < n; ++i)
    {
        real32_t ox = bmath_randf(- .3f * hw, .3f * hw);
        real32_t oy = bmath_randf(- .3f * hh, .3f * hh);
        pt[i].x = bmath_randf(-hw, hw) + ox;
        pt[i].y = bmath_randf(-hh, hh) + oy;
    }
    
    if (cloud->angle != 0)
    {
        T2Df t2d;
        t2d_rotatef(&t2d, kT2D_IDENTf, cloud->angle);
        t2d_vmultnf(pt, &t2d, pt, n);
    }

    cloud->box = box2d_from_pointsf(pt, n);
    col2dhello_update_cloud_bounds(cloud);
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_cloud_bounds(Cloud *cloud)
{
    const V2Df *p = arrst_all(cloud->pnts, V2Df);
    uint32_t n = arrst_size(cloud->pnts, V2Df);

    i_remove_bounds(cloud);
    switch(cloud->type) {
    case 0:
        cloud->bound.cir = cir2d_from_boxf(&cloud->box);
        break;

    case 1:
        cloud->bound.cir = cir2d_from_pointsf(p, n);
        break;

    case 2:
        cloud->bound.cir = cir2d_minimumf(p, n);
        break;

    case 3:
        cloud->bound.obb = obb2d_from_pointsf(p, n);
        break;

    case 4:
        cloud->bound.poly = pol2d_convex_hullf(p, n);
        break;
    cassert_default();
    }

    cloud->ctype = cloud->type;
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_box(Box *box)
{
    cassert_no_null(box);
    box->box.min.x = box->center.x - box->width / 2;
    box->box.min.y = box->center.y - box->height / 2;
    box->box.max.x = box->center.x + box->width / 2;
    box->box.max.y = box->center.y + box->height / 2;
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_obb(OBB *obb)
{
    cassert_no_null(obb);
    if (obb->obb == NULL)
        obb->obb = obb2d_createf(&obb->center, obb->width, obb->height, obb->angle);
    else
        obb2d_updatef(obb->obb, &obb->center, obb->width, obb->height, obb->angle);
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_tri(Tri *tri)
{
    T2Df t2d, nt2d;
    cassert_no_null(tri);
    t2d_inversef(&t2d, &tri->t2d);
    t2d_movef(&nt2d, kT2D_IDENTf, tri->center.x, tri->center.y);
    t2d_rotatef(&nt2d, &nt2d, tri->angle);
    t2d_scalef(&nt2d, &nt2d, tri->scale, tri->scale);
    t2d_multf(&t2d, &nt2d, &t2d);
    tri2d_transformf(&tri->tri, &t2d);
    tri->t2d = nt2d;
}

/*---------------------------------------------------------------------------*/

void col2dhello_update_pol(Pol *pol)
{
    T2Df t2d, nt2d;
    cassert_no_null(pol);
    cassert_no_null(pol->pol);
    t2d_inversef(&t2d, &pol->t2d);
    t2d_movef(&nt2d, kT2D_IDENTf, pol->center.x, pol->center.y);
    t2d_rotatef(&nt2d, &nt2d, pol->angle);
    t2d_scalef(&nt2d, &nt2d, pol->scale, pol->scale);
    t2d_multf(&t2d, &nt2d, &t2d);
    pol2d_transformf(pol->pol, &t2d);
    pol->t2d = nt2d;
}

/*---------------------------------------------------------------------------*/

static bool_t i_mouse_inside(const Shape *shape, const real32_t mouse_x, const real32_t mouse_y)
{
    V2Df m = v2df(mouse_x, mouse_y);

    switch(shape->type) {
    case ekPOINT:
        return col2d_point_pointf(&shape->body.pnt, &m, CENTER_RADIUS, NULL);

    case ekPOINT_CLOUD:
    {
        Box2Df box = col2dhello_cloud_box(&shape->body.cloud);
        return col2d_box_pointf(&box, &m, NULL);
    }

    case ekSEGMENT:
        return col2d_segment_pointf(&shape->body.seg.seg, &m, CENTER_RADIUS, NULL);

    case ekCIRCLE:
        return col2d_circle_pointf(&shape->body.cir, &m, NULL);

    case ekBOX:
        return col2d_box_pointf(&shape->body.box.box, &m, NULL);

    case ekOBB:
        return col2d_obb_pointf(shape->body.obb.obb, &m, NULL);

    case ekTRIANGLE:
        return col2d_tri_pointf(&shape->body.tri.tri, &m, NULL);

    case ekCONVEX_POLY:
    case ekSIMPLE_POLY:
        return col2d_poly_pointf(shape->body.pol.pol, &m, NULL);

    cassert_default();
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

void col2dhello_mouse_collisions(App *app, const real32_t mouse_x, const real32_t mouse_y)
{
    arrst_foreach(shape, app->shapes, Shape)
        shape->mouse = i_mouse_inside(shape, mouse_x, mouse_y);
    arrst_end();
}

/*---------------------------------------------------------------------------*/

static void i_point_segment_dist(const Seg2Df *seg, const V2Df *pnt, ArrSt(Dist) *dists)
{
    Dist *dist = arrst_new(dists, Dist);
    real32_t t = seg2d_close_paramf(seg, pnt);
    dist->p0 = *pnt;
    dist->p1 = seg2d_evalf(seg, t);
}

/*---------------------------------------------------------------------------*/

void col2dhello_collisions(App *app)
{
    Shape *shape = arrst_all(app->shapes, Shape);
    uint32_t n = arrst_size(app->shapes, Shape);
    uint32_t i, j;

    arrst_clear(app->dists, NULL, Dist);

    for (i = 0; i < n; ++i)
        shape[i].collisions = 0;

    for (i = 0; i < n; ++i)
    for (j = i + 1; j < n; ++j)
    {
        const Shape *shape1 = shape[i].type < shape[j].type ? &shape[i] : &shape[j];
        const Shape *shape2 = shape[i].type < shape[j].type ? &shape[j] : &shape[i];
        bool_t col = FALSE;

        switch(shape1->type) {
        case ekPOINT:
            switch(shape2->type) {
            case ekPOINT:
                col = col2d_point_pointf(&shape1->body.pnt, &shape2->body.pnt, CENTER_RADIUS, NULL);
                break;

            case ekPOINT_CLOUD:
                col = FALSE;
                break;

            case ekSEGMENT:
                col = col2d_segment_pointf(&shape2->body.seg.seg, &shape1->body.pnt, CENTER_RADIUS, NULL);
                i_point_segment_dist(&shape2->body.seg.seg, &shape1->body.pnt, app->dists);
                break;

            case ekCIRCLE:
                col = col2d_circle_pointf(&shape2->body.cir, &shape1->body.pnt, NULL);
                break;

            case ekBOX:
                col = col2d_box_pointf(&shape2->body.box.box, &shape1->body.pnt, NULL);
                break;

            case ekOBB:
                col = col2d_obb_pointf(shape2->body.obb.obb, &shape1->body.pnt, NULL);
                break;

            case ekTRIANGLE:
                col = col2d_tri_pointf(&shape2->body.tri.tri, &shape1->body.pnt, NULL);
                break;

            case ekCONVEX_POLY:
            case ekSIMPLE_POLY:
                col = col2d_poly_pointf(shape2->body.pol.pol, &shape1->body.pnt, NULL);
                break;

            cassert_default();
            }
            break;

        case ekPOINT_CLOUD:
            col = FALSE;
            break;

        case ekSEGMENT:
            switch(shape2->type) {
            case ekSEGMENT:
                col = col2d_segment_segmentf(&shape1->body.seg.seg, &shape2->body.seg.seg, NULL);
                break;

            case ekCIRCLE:
                col = col2d_circle_segmentf(&shape2->body.cir, &shape1->body.seg.seg, NULL);
                break;

            case ekBOX:
                col = col2d_box_segmentf(&shape2->body.box.box, &shape1->body.seg.seg, NULL);
                break;

            case ekOBB:
                col = col2d_obb_segmentf(shape2->body.obb.obb, &shape1->body.seg.seg, NULL);
                break;

            case ekTRIANGLE:
                col = col2d_tri_segmentf(&shape2->body.tri.tri, &shape1->body.seg.seg, NULL);
                break;

            case ekCONVEX_POLY:
            case ekSIMPLE_POLY:
                col = col2d_poly_segmentf(shape2->body.pol.pol, &shape1->body.seg.seg, NULL);
                break;

            case ekPOINT:
            case ekPOINT_CLOUD:
            cassert_default();
            }
            break;

        case ekCIRCLE:
            switch(shape2->type) {
            case ekCIRCLE:
                col = col2d_circle_circlef(&shape1->body.cir, &shape2->body.cir, NULL);
                break;

            case ekBOX:
                col = col2d_box_circlef(&shape2->body.box.box, &shape1->body.cir, NULL);
                break;

            case ekOBB:
                col = col2d_obb_circlef(shape2->body.obb.obb, &shape1->body.cir, NULL);
                break;

            case ekTRIANGLE:
                col = col2d_tri_circlef(&shape2->body.tri.tri, &shape1->body.cir, NULL);
                break;

            case ekCONVEX_POLY:
            case ekSIMPLE_POLY:
                col = col2d_poly_circlef(shape2->body.pol.pol, &shape1->body.cir, NULL);
                break;

            case ekPOINT:
            case ekPOINT_CLOUD:
            case ekSEGMENT:
            cassert_default();
            }
            break;

        case ekBOX:
            switch(shape2->type) {
            case ekBOX:
                col = col2d_box_boxf(&shape1->body.box.box, &shape2->body.box.box, NULL);
                break;

            case ekOBB:
                col = col2d_obb_boxf(shape2->body.obb.obb, &shape1->body.box.box, NULL);
                break;

            case ekTRIANGLE:
                col = col2d_tri_boxf(&shape2->body.tri.tri, &shape1->body.box.box, NULL);
                break;

            case ekCONVEX_POLY:
            case ekSIMPLE_POLY:
                col = col2d_poly_boxf(shape2->body.pol.pol, &shape1->body.box.box, NULL);
                break;

            case ekPOINT:
            case ekPOINT_CLOUD:
            case ekSEGMENT:
            case ekCIRCLE:
            cassert_default();
            }
            break;

        case ekOBB:
            switch(shape2->type) {
            case ekOBB:
                col = col2d_obb_obbf(shape1->body.obb.obb, shape2->body.obb.obb, NULL);
                break;

            case ekTRIANGLE:
                col = col2d_tri_obbf(&shape2->body.tri.tri, shape1->body.obb.obb, NULL);
                break;

            case ekCONVEX_POLY:
            case ekSIMPLE_POLY:
                col = col2d_poly_obbf(shape2->body.pol.pol, shape1->body.obb.obb, NULL);
                break;

            case ekPOINT:
            case ekPOINT_CLOUD:
            case ekSEGMENT:
            case ekCIRCLE:
            case ekBOX:
            cassert_default();
            }
            break;

        case ekTRIANGLE:
            switch(shape2->type) {
            case ekTRIANGLE:
                col = col2d_tri_trif(&shape1->body.tri.tri, &shape2->body.tri.tri, NULL);
                break;

            case ekCONVEX_POLY:
            case ekSIMPLE_POLY:
                col = col2d_poly_trif(shape2->body.pol.pol, &shape1->body.tri.tri, NULL);
                break;

            case ekPOINT:
            case ekPOINT_CLOUD:
            case ekSEGMENT:
            case ekCIRCLE:
            case ekBOX:
            case ekOBB:
            cassert_default();
            }
            break;

        case ekCONVEX_POLY:
        case ekSIMPLE_POLY:
            switch(shape2->type) {
            case ekCONVEX_POLY:
            case ekSIMPLE_POLY:
                col = col2d_poly_polyf(shape1->body.pol.pol, shape2->body.pol.pol, NULL);
                break;

            case ekPOINT:
            case ekPOINT_CLOUD:
            case ekSEGMENT:
            case ekCIRCLE:
            case ekBOX:
            case ekOBB:
            case ekTRIANGLE:
            cassert_default();
            }
            break;

        cassert_default();
        }

        if (col == TRUE)
        {
            shape[i].collisions += 1;
            shape[j].collisions += 1;
        }
    }
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
