/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bmath.cpp
 *
 */

/* Math funcions */

#include "bmath.h"
#include "bmath.hpp"
#include "bmath.inl"
#include "bmem.h"
#include "cassert.h"
#include "ptr.h"

#include "nowarn.hxx"
extern "C"
{
#include <math.h>
}
#include "warn.hxx"

/*---------------------------------------------------------------------------*/

template < typename real >
static bool_t i_equals(const real value1, const real value2, const real tolerance)
{
    real diff = value1 - value2;
    cassert(tolerance >= 0);
    if (diff < 0)
        diff = -diff;
    if (diff < tolerance)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

template < typename real >
static bool_t i_less(const real value1, const real value2, const real tolerance)
{
    cassert(tolerance >= 0);
    if ((value1 + tolerance) < value2)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

template < typename real >
static bool_t i_less_eq(const real value1, const real value2, const real tolerance)
{
    cassert(tolerance >= 0);
    if (value1 < value2)
        return TRUE;
    else if (value2 - value1 < tolerance)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

template < typename real >
static bool_t i_great(const real value1, const real value2, const real tolerance)
{
    cassert(tolerance >= 0);
    if ((value1 - tolerance) > value2)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_cosf(const real32_t angle)
{
    return (real32_t)cosf((float)angle);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_cosd(const real64_t angle)
{
    return (real64_t)cos((double)angle);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_sinf(const real32_t angle)
{
    return (real32_t)sinf((float)angle);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_sind(const real64_t angle)
{
    return (real64_t)sin((double)angle);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_tanf(const real32_t angle)
{
    return (real32_t)tanf((float)angle);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_tand(const real64_t angle)
{
    return (real64_t)tan((double)angle);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_acosf(const real32_t cos)
{
    cassert(cos >= -1.f && cos <= 1.f);
    return (real32_t)acosf((float)cos);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_acosd(const real64_t cos)
{
    cassert(cos >= -1. && cos <= 1.);
    return (real64_t)acos((double)cos);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_asinf(const real32_t sin)
{
    cassert(sin >= -1.f && sin <= 1.f);
    return (real32_t)asinf((float)sin);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_asind(const real64_t sin)
{
    cassert(sin >= -1. && sin <= 1.);
    return (real64_t)asin((double)sin);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_atan2f(const real32_t y, const real32_t x)
{
    return (real32_t)atan2f((float)y, (float)x);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_atan2d(const real64_t y, const real64_t x)
{
    return (real64_t)atan2((double)y, (double)x);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_norm_anglef(const real32_t a)
{
    const real32_t twopi = 2 * kBMATH_PIf;
    real32_t nangle = a - twopi * bmath_floorf((a + kBMATH_PIf) / twopi);
    cassert(nangle > -kBMATH_PIf && nangle <= kBMATH_PIf);
    return nangle;
}

/*---------------------------------------------------------------------------*/

real64_t bmath_norm_angled(const real64_t a)
{
    const real64_t twopi = 2 * kBMATH_PId;
    real64_t nangle = a - twopi * bmath_floord((a + kBMATH_PId) / twopi);
    cassert(nangle > -kBMATH_PId && nangle <= kBMATH_PId);
    return nangle;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_sqrtf(const real32_t value)
{
    cassert(value >= 0.f);
    return (real32_t)sqrtf((float)value);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_sqrtd(const real64_t value)
{
    cassert(value >= 0.);
    return (real64_t)sqrt((double)value);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_isqrtf(const real32_t value)
{
    cassert(value > 0.f);
    return 1.f / sqrtf((float)value);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_isqrtd(const real64_t value)
{
    cassert(value > 0.);
    return 1.f / sqrt((double)value);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_logf(const real32_t value)
{
    return (real32_t)logf((float)value);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_logd(const real64_t value)
{
    return (real64_t)log((double)value);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_log10f(const real32_t value)
{
    return (real32_t)log10f((float)value);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_log10d(const real64_t value)
{
    return (real64_t)log10((double)value);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_expf(const real32_t value)
{
    return (real32_t)expf((float)value);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_expd(const real64_t value)
{
    return (real64_t)exp((double)value);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_powf(const real32_t base, const real32_t exponent)
{
    return (real32_t)powf((float)base, (float)exponent);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_powd(const real64_t base, const real64_t exponent)
{
    return (real64_t)pow((double)base, (double)exponent);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_absf(const real32_t value)
{
    if (value < 0.f)
        return -value;
    else
        return value;
}

/*---------------------------------------------------------------------------*/

real64_t bmath_absd(const real64_t value)
{
    if (value < 0.f)
        return -value;
    else
        return value;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_maxf(const real32_t value1, const real32_t value2)
{
    if (value1 > value2)
        return value1;
    else
        return value2;
}

/*---------------------------------------------------------------------------*/

real64_t bmath_maxd(const real64_t value1, const real64_t value2)
{
    if (value1 > value2)
        return value1;
    else
        return value2;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_minf(const real32_t value1, const real32_t value2)
{
    if (value1 < value2)
        return value1;
    else
        return value2;
}

/*---------------------------------------------------------------------------*/

real64_t bmath_mind(const real64_t value1, const real64_t value2)
{
    if (value1 < value2)
        return value1;
    else
        return value2;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_clampf(const real32_t value, const real32_t min, const real32_t max)
{
    cassert(min < max);

    if (__FALSE_EXPECTED(value < min))
        return min;

    else if (__FALSE_EXPECTED(value > max))
        return max;

    return value;
}

/*---------------------------------------------------------------------------*/

real64_t bmath_clampd(const real64_t value, const real64_t min, const real64_t max)
{
    cassert(min < max);

    if (__FALSE_EXPECTED(value < min))
        return min;

    else if (__FALSE_EXPECTED(value > max))
        return max;

    return value;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_modf(const real32_t num, const real32_t den)
{
    return (real32_t)fmodf((float)num, (float)den);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_modd(const real64_t value, const real64_t den)
{
    return (real64_t)fmod((double)value, (double)den);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_modff(const real32_t value, real32_t *intpart)
{
    return (real32_t)modff((float)value, (float *)intpart);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_modfd(const real64_t num, real64_t *intpart)
{
    return (real64_t)modf((double)num, (double *)intpart);
}

/*---------------------------------------------------------------------------*/

template < typename real >
uint32_t i_precision(const real value)
{
    static const real i_PREC_EPSION = 0.00000001;
    real e = 1, v = value, frac, intpart;
    uint32_t p = 0;

    if (v < 0)
        v = -v;

    frac = BMath< real >::modf(v, &intpart);

    for (;;)
    {
        if (frac < i_PREC_EPSION)
            return p;

        if (frac + i_PREC_EPSION > e)
            return p;

        e /= 10;
        p += 1;
    }
}

/*---------------------------------------------------------------------------*/

uint32_t bmath_precf(const real32_t value)
{
    return i_precision< real32_t >(value);
}

/*---------------------------------------------------------------------------*/

uint32_t bmath_precd(const real64_t value)
{
    return i_precision< real64_t >(value);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_roundf(const real32_t value)
{
    return (real32_t)floorf((float)(value + .5f));
}

/*---------------------------------------------------------------------------*/

real64_t bmath_roundd(const real64_t value)
{
    return floor((double)(value + .5));
}

/*---------------------------------------------------------------------------*/

real32_t bmath_round_stepf(const real32_t value, const real32_t step)
{
    cassert(step > 0.f);
    return step * (real32_t)floorf((float)(value / step + .5f));
}

/*---------------------------------------------------------------------------*/

real64_t bmath_round_stepd(const real64_t value, const real64_t step)
{
    cassert(step > 0.f);
    return step * (real64_t)floor((double)(value / step + .5));
}

/*---------------------------------------------------------------------------*/

real32_t bmath_floorf(const real32_t value)
{
    return (real32_t)floorf((float)value);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_floord(const real64_t value)
{
    return (real64_t)floor((double)value);
}

/*---------------------------------------------------------------------------*/

real32_t bmath_ceilf(const real32_t value)
{
    return (real32_t)ceilf((float)value);
}

/*---------------------------------------------------------------------------*/

real64_t bmath_ceild(const real64_t value)
{
    return (real64_t)ceil((double)value);
}

/*---------------------------------------------------------------------------*/

/* Random numbers */
/* Originally developed and coded by Makoto Matsumoto and Takuji
 * Nishimura.  Please mail <matumoto@math.keio.ac.jp>, if you're using
 * code from this file in your own programs or libraries.
 * Further information on the Mersenne Twister can be found at
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 */

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y) (y >> 11)
#define TEMPERING_SHIFT_S(y) (y << 7)
#define TEMPERING_SHIFT_T(y) (y << 15)
#define TEMPERING_SHIFT_L(y) (y >> 18)

struct _renv_t
{
    uint32_t mti;
    uint32_t mt[N];
};

static REnv *i_GLOBAL_RENV = NULL;

/*---------------------------------------------------------------------------*/

void _bmath_finish(void)
{
    if (i_GLOBAL_RENV != NULL)
        bmem_free(cast(i_GLOBAL_RENV, byte_t));
}

/*---------------------------------------------------------------------------*/

static void i_set_seed(REnv *env, const uint32_t seed)
{
    cassert_no_null(env);
    env->mt[0] = seed;
    for (env->mti = 1; env->mti < N; env->mti++)
        env->mt[env->mti] = (uint32_t)1812433253UL * (env->mt[env->mti - 1] ^ (env->mt[env->mti - 1] >> 30)) + env->mti;
}

/*---------------------------------------------------------------------------*/

static void i_global_env(void)
{
    if (__FALSE_EXPECTED(i_GLOBAL_RENV == NULL))
    {
        i_GLOBAL_RENV = (REnv *)bmem_malloc(sizeof(REnv));
        i_set_seed(i_GLOBAL_RENV, TEMPERING_MASK_B);
        i_GLOBAL_RENV->mti = N;
    }
}

/*---------------------------------------------------------------------------*/

void bmath_rand_seed(const uint32_t seed)
{
    i_global_env();
    i_set_seed(i_GLOBAL_RENV, seed);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_next_random(REnv *env)
{
    uint32_t y;
    static const uint32_t mag01[2] = {0x0, MATRIX_A};

    cassert_no_null(env);

    /* Generate N words at one time */
    if (env->mti >= N)
    {
        uint32_t kk;
        uint32_t *mt = env->mt;

        for (kk = 0; kk < N - M; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
        }

        for (; kk < N - 1; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (uint32_t)(M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }

        y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

        env->mti = 0;
    }

    y = env->mt[env->mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_randf(const real32_t from, const real32_t to)
{
    cassert(from < to);
    i_global_env();
    return from + (real32_t)(i_next_random(i_GLOBAL_RENV) % 1000001) * (to - from) * .000001f;
}

/*---------------------------------------------------------------------------*/

real64_t bmath_randd(const real64_t from, const real64_t to)
{
    cassert(from < to);
    i_global_env();
    return from + (real64_t)(i_next_random(i_GLOBAL_RENV) % 100000001) * (to - from) * .00000001;
}

/*---------------------------------------------------------------------------*/

uint32_t bmath_randi(const uint32_t from, const uint32_t to)
{
    cassert(from < to);
    i_global_env();
    return from + (i_next_random(i_GLOBAL_RENV) % (to - from + 1));
}

/*---------------------------------------------------------------------------*/

REnv *bmath_rand_env(const uint32_t seed)
{
    REnv *env = (REnv *)bmem_malloc(sizeof(REnv));
    env->mti = N;
    i_set_seed(env, seed);
    return env;
}

/*---------------------------------------------------------------------------*/

void bmath_rand_destroy(REnv **env)
{
    cassert_no_null(env);
    bmem_free(*dcast(env, byte_t));
    *env = NULL;
}

/*---------------------------------------------------------------------------*/

real32_t bmath_rand_mtf(REnv *env, const real32_t from, const real32_t to)
{
    cassert(from < to);
    return from + (real32_t)(i_next_random(env) % 1000001) * (to - from) * .000001f;
}

/*---------------------------------------------------------------------------*/

real64_t bmath_rand_mtd(REnv *env, const real64_t from, const real64_t to)
{
    cassert(from < to);
    return from + (real64_t)(i_next_random(env) % 100000001) * (to - from) * .00000001;
}

/*---------------------------------------------------------------------------*/

uint32_t bmath_rand_mti(REnv *env, const uint32_t from, const uint32_t to)
{
    cassert(from < to);
    return from + (i_next_random(env) % (to - from + 1));
}

/*---------------------------------------------------------------------------*/

const real32_t kBMATH_Ef = 2.7182818284590452353602874713526624977572470937000f;
const real64_t kBMATH_Ed = 2.7182818284590452353602874713526624977572470937000;
template <>
const real32_t BMath< real32_t >::kE = 2.7182818284590452353602874713526624977572470937000f;
template <>
const real64_t BMath< real64_t >::kE = 2.7182818284590452353602874713526624977572470937000;
const real32_t kBMATH_LN2f = 0.69314718055994530941723212145817656807550013436026f;
const real64_t kBMATH_LN2d = 0.69314718055994530941723212145817656807550013436026;
template <>
const real32_t BMath< real32_t >::kLN2 = 0.69314718055994530941723212145817656807550013436026f;
template <>
const real64_t BMath< real64_t >::kLN2 = 0.69314718055994530941723212145817656807550013436026;
const real32_t kBMATH_LN10f = 2.3025850929940456840179914546843642076011014886288f;
const real64_t kBMATH_LN10d = 2.3025850929940456840179914546843642076011014886288;
template <>
const real32_t BMath< real32_t >::kLN10 = 2.3025850929940456840179914546843642076011014886288f;
template <>
const real64_t BMath< real64_t >::kLN10 = 2.3025850929940456840179914546843642076011014886288;
const real32_t kBMATH_PIf = 3.1415926535897932384626433832795028841971693993751f;
const real64_t kBMATH_PId = 3.1415926535897932384626433832795028841971693993751;
template <>
const real32_t BMath< real32_t >::kPI = 3.1415926535897932384626433832795028841971693993751f;
template <>
const real64_t BMath< real64_t >::kPI = 3.1415926535897932384626433832795028841971693993751;
const real32_t kBMATH_SQRT2f = 1.4142135623730950488016887242096980785696718753769f;
const real64_t kBMATH_SQRT2d = 1.4142135623730950488016887242096980785696718753769;
template <>
const real32_t BMath< real32_t >::kSQRT2 = 1.4142135623730950488016887242096980785696718753769f;
template <>
const real64_t BMath< real64_t >::kSQRT2 = 1.4142135623730950488016887242096980785696718753769;
const real32_t kBMATH_SQRT3f = 1.732050807568877293527446f;
const real64_t kBMATH_SQRT3d = 1.732050807568877293527446;
template <>
const real32_t BMath< real32_t >::kSQRT3 = 1.732050807568877293527446f;
template <>
const real64_t BMath< real64_t >::kSQRT3 = 1.732050807568877293527446;
const real32_t kBMATH_DEG2RADf = 0.017453292519943f;
const real64_t kBMATH_DEG2RADd = 0.017453292519943;
template <>
const real32_t BMath< real32_t >::kDEG2RAD = 0.017453292519943f;
template <>
const real64_t BMath< real64_t >::kDEG2RAD = 0.017453292519943;
const real32_t kBMATH_RAD2DEGf = 57.29577951308232f;
const real64_t kBMATH_RAD2DEGd = 57.29577951308232;
template <>
const real32_t BMath< real32_t >::kRAD2DEG = 57.29577951308232f;
template <>
const real64_t BMath< real64_t >::kRAD2DEG = 57.29577951308232;
const real32_t kBMATH_INFINITYf = REAL32_MAX;
const real64_t kBMATH_INFINITYd = REAL64_MAX;
template <>
const real32_t BMath< real32_t >::kINFINITY = REAL32_MAX;
template <>
const real64_t BMath< real64_t >::kINFINITY = REAL64_MAX;

/*---------------------------------------------------------------------------*/

template <>
real32_t (*BMath< real32_t >::cos)(const real32_t) = bmath_cosf;

template <>
real64_t (*BMath< real64_t >::cos)(const real64_t) = bmath_cosd;

template <>
real32_t (*BMath< real32_t >::sin)(const real32_t) = bmath_sinf;

template <>
real64_t (*BMath< real64_t >::sin)(const real64_t) = bmath_sind;

template <>
real32_t (*BMath< real32_t >::tan)(const real32_t) = bmath_tanf;

template <>
real64_t (*BMath< real64_t >::tan)(const real64_t) = bmath_tand;

template <>
real32_t (*BMath< real32_t >::acos)(const real32_t) = bmath_acosf;

template <>
real64_t (*BMath< real64_t >::acos)(const real64_t) = bmath_acosd;

template <>
real32_t (*BMath< real32_t >::asin)(const real32_t) = bmath_asinf;

template <>
real64_t (*BMath< real64_t >::asin)(const real64_t) = bmath_asind;

template <>
real32_t (*BMath< real32_t >::atan2)(const real32_t, const real32_t) = bmath_atan2f;

template <>
real64_t (*BMath< real64_t >::atan2)(const real64_t, const real64_t) = bmath_atan2d;

template <>
real32_t (*BMath< real32_t >::norm_angle)(const real32_t) = bmath_norm_anglef;

template <>
real64_t (*BMath< real64_t >::norm_angle)(const real64_t) = bmath_norm_angled;

template <>
real32_t (*BMath< real32_t >::sqrt)(const real32_t) = bmath_sqrtf;

template <>
real64_t (*BMath< real64_t >::sqrt)(const real64_t) = bmath_sqrtd;

template <>
real32_t (*BMath< real32_t >::isqrt)(const real32_t) = bmath_isqrtf;

template <>
real64_t (*BMath< real64_t >::isqrt)(const real64_t) = bmath_isqrtd;

template <>
real32_t (*BMath< real32_t >::log)(const real32_t) = bmath_logf;

template <>
real64_t (*BMath< real64_t >::log)(const real64_t) = bmath_logd;

template <>
real32_t (*BMath< real32_t >::log10)(const real32_t) = bmath_log10f;

template <>
real64_t (*BMath< real64_t >::log10)(const real64_t) = bmath_log10d;

template <>
real32_t (*BMath< real32_t >::exp)(const real32_t) = bmath_expf;

template <>
real64_t (*BMath< real64_t >::exp)(const real64_t) = bmath_expd;

template <>
real32_t (*BMath< real32_t >::abs)(const real32_t) = bmath_absf;

template <>
real64_t (*BMath< real64_t >::abs)(const real64_t) = bmath_absd;

template <>
real32_t (*BMath< real32_t >::max)(const real32_t, const real32_t) = bmath_maxf;

template <>
real64_t (*BMath< real64_t >::max)(const real64_t, const real64_t) = bmath_maxd;

template <>
real32_t (*BMath< real32_t >::min)(const real32_t, const real32_t) = bmath_minf;

template <>
real64_t (*BMath< real64_t >::min)(const real64_t, const real64_t) = bmath_mind;

template <>
real32_t (*BMath< real32_t >::clamp)(const real32_t, const real32_t, const real32_t) = bmath_clampf;

template <>
real64_t (*BMath< real64_t >::clamp)(const real64_t, const real64_t, const real64_t) = bmath_clampd;

template <>
real32_t (*BMath< real32_t >::mod)(const real32_t, const real32_t) = bmath_modf;

template <>
real64_t (*BMath< real64_t >::mod)(const real64_t, const real64_t) = bmath_modd;

template <>
real32_t (*BMath< real32_t >::modf)(const real32_t, real32_t *) = bmath_modff;

template <>
real64_t (*BMath< real64_t >::modf)(const real64_t, real64_t *) = bmath_modfd;

template <>
uint32_t (*BMath< real32_t >::prec)(const real32_t) = bmath_precf;

template <>
uint32_t (*BMath< real64_t >::prec)(const real64_t) = bmath_precd;

template <>
real32_t (*BMath< real32_t >::round)(const real32_t) = bmath_roundf;

template <>
real64_t (*BMath< real64_t >::round)(const real64_t) = bmath_roundd;

template <>
real32_t (*BMath< real32_t >::round_step)(const real32_t, const real32_t) = bmath_round_stepf;

template <>
real64_t (*BMath< real64_t >::round_step)(const real64_t, const real64_t) = bmath_round_stepd;

template <>
real32_t (*BMath< real32_t >::floor)(const real32_t) = bmath_floorf;

template <>
real64_t (*BMath< real64_t >::floor)(const real64_t) = bmath_floord;

template <>
real32_t (*BMath< real32_t >::ceil)(const real32_t) = bmath_ceilf;

template <>
real64_t (*BMath< real64_t >::ceil)(const real64_t) = bmath_ceild;
