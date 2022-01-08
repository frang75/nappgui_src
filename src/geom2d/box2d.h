/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: box2d.h
 * https://nappgui.com/en/geom2d/box2d.html
 *
 */

/* Box 2d */

#include "geom2d.hxx"

__EXTERN_C

Box2Df box2df(const real32_t minX, const real32_t minY, const real32_t maxX, const real32_t maxY);

Box2Dd box2dd(const real64_t minX, const real64_t minY, const real64_t maxX, const real64_t maxY);

Box2Df box2d_from_pointsf(const V2Df *p, const uint32_t n);

Box2Dd box2d_from_pointsd(const V2Dd *p, const uint32_t n);

V2Df box2d_centerf(const Box2Df *box);

V2Dd box2d_centerd(const Box2Dd *box);

void box2d_addf(Box2Df *box, const V2Df *p);

void box2d_addd(Box2Dd *box, const V2Dd *p);

void box2d_addnf(Box2Df *box, const V2Df *p, const uint32_t n);

void box2d_addnd(Box2Dd *box, const V2Dd *p, const uint32_t n);

void box2d_add_circlef(Box2Df *box, const Cir2Df *cir);

void box2d_add_circled(Box2Dd *box, const Cir2Dd *cir);

void box2d_mergef(Box2Df *dest, const Box2Df *src);

void box2d_merged(Box2Dd *dest, const Box2Dd *src);

void box2d_segmentsf(const Box2Df *box, Seg2Df *segs);

void box2d_segmentsd(const Box2Dd *box, Seg2Dd *segs);

real32_t box2d_areaf(const Box2Df *box);

real64_t box2d_aread(const Box2Dd *box);

bool_t box2d_is_nullf(const Box2Df *box);

bool_t box2d_is_nulld(const Box2Dd *box);

extern const Box2Df kBOX2D_NULLf;
extern const Box2Dd kBOX2D_NULLd;

__END_C

