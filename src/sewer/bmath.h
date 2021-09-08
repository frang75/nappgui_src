/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bmath.h
 * https://nappgui.com/en/sewer/bmath.html
 *
 */

/* Math funcions */

#include "sewer.hxx"

__EXTERN_C

real32_t bmath_cosf(const real32_t angle);

real64_t bmath_cosd(const real64_t angle);

real32_t bmath_sinf(const real32_t angle);

real64_t bmath_sind(const real64_t angle);

real32_t bmath_tanf(const real32_t angle);

real64_t bmath_tand(const real64_t angle);

real32_t bmath_acosf(const real32_t cos);

real64_t bmath_acosd(const real64_t cos);

real32_t bmath_asinf(const real32_t sin);

real64_t bmath_asind(const real64_t sin);

real32_t bmath_atan2f(const real32_t y, const real32_t x);

real64_t bmath_atan2d(const real64_t y, const real64_t x);

real32_t bmath_norm_anglef(const real32_t a);

real64_t bmath_norm_angled(const real64_t a);

real32_t bmath_sqrtf(const real32_t value);

real64_t bmath_sqrtd(const real64_t value);

real32_t bmath_isqrtf(const real32_t value);

real64_t bmath_isqrtd(const real64_t value);

real32_t bmath_logf(const real32_t value);

real64_t bmath_logd(const real64_t value);

real32_t bmath_log10f(const real32_t value);

real64_t bmath_log10d(const real64_t value);

real32_t bmath_expf(const real32_t value);

real64_t bmath_expd(const real64_t value);

real32_t bmath_powf(const real32_t base, const real32_t exponent);

real64_t bmath_powd(const real64_t base, const real64_t exponent);


real32_t bmath_absf(const real32_t value);

real64_t bmath_absd(const real64_t value);

real32_t bmath_maxf(const real32_t value1, const real32_t value2);

real64_t bmath_maxd(const real64_t value1, const real64_t value2);

real32_t bmath_minf(const real32_t value1, const real32_t value2);

real64_t bmath_mind(const real64_t value1, const real64_t value2);

real32_t bmath_clampf(const real32_t value, const real32_t min, const real32_t max);

real64_t bmath_clampd(const real64_t value, const real64_t min, const real64_t max);

real32_t bmath_modf(const real32_t num, const real32_t den);

real64_t bmath_modd(const real64_t num, const real64_t den);

real32_t bmath_modff(const real32_t value, real32_t *intpart);

real64_t bmath_modfd(const real64_t value, real64_t *intpart);

uint32_t bmath_precf(const real32_t value);

uint32_t bmath_precd(const real64_t value);

real32_t bmath_roundf(const real32_t value);

real64_t bmath_roundd(const real64_t value);

real32_t bmath_round_stepf(const real32_t value, const real32_t step);

real64_t bmath_round_stepd(const real64_t value, const real64_t step);

real32_t bmath_floorf(const real32_t value);

real64_t bmath_floord(const real64_t value);

real32_t bmath_ceilf(const real32_t value);

real64_t bmath_ceild(const real64_t value);


void bmath_rand_seed(const uint32_t seed);

real32_t bmath_randf(const real32_t from, const real32_t to);

real64_t bmath_randd(const real64_t from, const real64_t to);

uint32_t bmath_randi(const uint32_t from, const uint32_t to);


REnv *bmath_rand_env(const uint32_t seed);

void bmath_rand_destroy(REnv **env);

real32_t bmath_rand_mtf(REnv *env, const real32_t from, const real32_t to);

real64_t bmath_rand_mtd(REnv *env, const real64_t from, const real64_t to);

uint32_t bmath_rand_mti(REnv *env, const uint32_t from, const uint32_t to);

extern const real32_t kBMATH_Ef;
extern const real64_t kBMATH_Ed;
extern const real32_t kBMATH_LN2f;
extern const real64_t kBMATH_LN2d;
extern const real32_t kBMATH_LN10f;
extern const real64_t kBMATH_LN10d;
extern const real32_t kBMATH_PIf;
extern const real64_t kBMATH_PId;
extern const real32_t kBMATH_SQRT2f;
extern const real64_t kBMATH_SQRT2d;
extern const real32_t kBMATH_SQRT3f;
extern const real64_t kBMATH_SQRT3d;
extern const real32_t kBMATH_DEG2RADf;
extern const real64_t kBMATH_DEG2RADd;
extern const real32_t kBMATH_RAD2DEGf;
extern const real64_t kBMATH_RAD2DEGd;
extern const real32_t kBMATH_INFINITYf;
extern const real64_t kBMATH_INFINITYd;

__END_C

