/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdplot.h
 * https://nappgui.com/en/demo/bode/bdplot.html
 *
 */

/* Bode plot drawing */

#include "bode.hxx"

Plot *plot_create(const uint32_t npoints, const uint32_t simulation_npoints);

void plot_destroy(Plot **plot);

void plot_update(Plot *plot, const real32_t T, const uint32_t last_sim_i);

void plot_take_snap(Plot *plot);

void plot_restore_snap(Plot *plot);

void plot_clear_snap(Plot *plot);

void plot_mouse1_x(Plot *plot, const real32_t x);

void plot_mouse2_x(Plot *plot, const real32_t x);

void plot_draw_graph1(Plot *plot, DCtx *ctx, const real32_t width, const real32_t height);

void plot_draw_graph2(Plot *plot, DCtx *ctx, const real32_t width, const real32_t height);
