/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: col2dhello.hxx
 *
 */

/* 2D collision detection demo */

#ifndef __COL2DHELLO_HXX__
#define __COL2DHELLO_HXX__

#include "gui.hxx"

#define CENTER_RADIUS       3
#define POINT_CLOUD_N       100

typedef struct _cloud_t Cloud;
typedef struct _seg_t Seg;
typedef struct _box_t Box;
typedef struct _obb_t OBB;
typedef struct _tri_t Tri;
typedef struct _pol_t Pol;
typedef struct _shape_t Shape;
typedef struct _dist_t Dist;
typedef struct _app_t App;

typedef enum _shtype_t
{
    ekPOINT,
    ekPOINT_CLOUD,
    ekSEGMENT,
    ekCIRCLE,
    ekBOX,
    ekOBB,
    ekTRIANGLE,
    ekCONVEX_POLY,
    ekSIMPLE_POLY
} shtype_t;

struct _cloud_t
{
    ArrSt(V2Df) *pnts;
    Box2Df box;
    V2Df center;
    real32_t width;
    real32_t height;
    real32_t angle;
    uint32_t ctype, type;

    union
    {
        Cir2Df cir;
        OBB2Df *obb;
        Pol2Df *poly;
    } bound;
};

struct _seg_t
{
    V2Df center;
    real32_t length;
    real32_t angle;
    Seg2Df seg;
};

struct _box_t
{
    V2Df center;
    real32_t width;
    real32_t height;
    Box2Df box;
};

struct _obb_t
{
    V2Df center;
    real32_t width;
    real32_t height;
    real32_t angle;
    OBB2Df *obb;
};

struct _tri_t
{
    V2Df center;
    real32_t angle;
    real32_t scale;
    T2Df t2d;
    Tri2Df tri;
};

struct _pol_t
{
    V2Df center;
    real32_t angle;
    real32_t scale;
    T2Df t2d;
    Pol2Df *pol;
};

struct _shape_t
{
    shtype_t type;
    bool_t mouse;
    uint32_t collisions;

    union {
        V2Df pnt;
        Cloud cloud;
        Seg seg;
        Cir2Df cir;
        Box box;
        OBB obb;
        Tri tri;
        Pol pol;
    } body;
};

struct _dist_t
{
    V2Df p0;
    V2Df p1;
};

struct _app_t
{
    Window *window;
    View *view;
    Layout *main_layout;
    Layout *pnt_layout;
    Layout *cld_layout;
    Layout *seg_layout;
    Layout *cir_layout;
    Layout *box_layout;
    Layout *obb_layout;
    Layout *tri_layout;
    Layout *pol_layout;
    Panel *obj_panel;
    ArrSt(Shape) *shapes;
    ArrSt(Dist) *dists;
    shtype_t seltype;
    uint32_t selshape;
    bool_t show_seg_pt;
    bool_t show_triangles;
    bool_t show_convex_parts;
    real32_t sel_area;
    V2Df mouse_pos;
    V2Df obj_pos;
};

DeclSt(Shape);
DeclSt(Dist);

#endif
