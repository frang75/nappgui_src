/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdcalc.h
 * https://nappgui.com/en/demo/bode/bdcalc.html
 *
 */

/* Bode Algorithm */

#include "geom2d.hxx"

void bode_set_P_coeffs(const real32_t *p);

void bode_set_Q_coeffs(const real32_t *q);

void bode_set_K(const real32_t *k);

void bode_set_T(const real32_t t);

void bode_set_R(const real32_t r);

void bode_update(uint32_t *last_sim_i);

uint32_t bode_npoints(void);

void bode_db_graph(V2Df *v2d, const uint32_t n);

void bode_phase_graph(V2Df *v2d, const uint32_t n);

uint32_t bode_sim_npoints(void);

void bode_sim_graph(V2Df *v2d, const uint32_t n);
