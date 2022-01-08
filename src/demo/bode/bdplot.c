/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdplot.c
 *
 */

/* Bode plot drawing */

#include "bdplot.h"
#include "bdcalc.h"
#include "guiall.h"

typedef struct _graph_t Graph;
typedef struct _graphs_t Graphs;

struct _graph_t
{
    Box2Df box;
    V2Df *points;
    V2Df *spoints;
    uint32_t n;
    uint32_t dn;
};

struct _graphs_t
{
    Graph db;
    Graph phase;
    Graph simu;
};

struct _plot_t
{
    Graphs graph;
    real32_t mouse1_x;
    real32_t mouse2_x;
    bool_t snapshot;
    color_t colbg1;
    color_t colbg2;
    color_t colbg3;
    color_t colaxis1;
    color_t colaxis2;
    color_t colgrap11;
    color_t colgrap12;
    color_t colgrap21;
    color_t colgrap22;
};

/*---------------------------------------------------------------------------*/

static const real32_t VMARGIN = 20;
static const real32_t LMARGIN = 25;
static const real32_t RMARGIN = 30;

/*---------------------------------------------------------------------------*/

static void i_init_graph(Graph *graph, const uint32_t npoints)
{
    cassert_no_null(graph);
    graph->points = heap_new_n(npoints, V2Df);
    graph->spoints = heap_new_n(npoints, V2Df);
    graph->n = npoints;
    graph->dn = npoints;
    graph->box = kBOX2D_NULLf;
}

/*---------------------------------------------------------------------------*/

static void i_remove_graph(Graph *graph)
{
    cassert_no_null(graph);
    heap_delete_n(&graph->points, graph->n, V2Df);
    heap_delete_n(&graph->spoints, graph->n, V2Df);
}

/*---------------------------------------------------------------------------*/

static void i_graph_take_snap(Graph *graph)
{
    cassert_no_null(graph);
    bmem_copy_n(graph->spoints, graph->points, graph->n, V2Df);
}

/*---------------------------------------------------------------------------*/

static void i_graph_restore_snap(Graph *graph)
{
    cassert_no_null(graph);
    bmem_copy_n(graph->points, graph->spoints, graph->n, V2Df);
}

/*---------------------------------------------------------------------------*/

Plot *plot_create(const uint32_t npoints, const uint32_t simulation_npoints)
{
    Plot *plot = heap_new0(Plot);
    i_init_graph(&plot->graph.db, npoints);
    i_init_graph(&plot->graph.phase, npoints);
    i_init_graph(&plot->graph.simu, simulation_npoints);
    plot->colbg1 = 0xFF1E1E1E;
    plot->colbg2 = 0xFF505050;
    plot->colbg3 = 0xFF323232;
    plot->colaxis1 = 0xFFEEEEEE;
    plot->colaxis2 = 0xFF555555;
    plot->colgrap11 = 0xFFFEBE43;
    plot->colgrap12 = 0xFFA66B34;
    plot->colgrap21 = 0xFF00F586;
    plot->colgrap22 = 0xFF2BA762;
    return plot;
}

/*---------------------------------------------------------------------------*/

void plot_destroy(Plot **plot)
{
    cassert_no_null(plot);
    cassert_no_null(*plot);
    i_remove_graph(&(*plot)->graph.db);
    i_remove_graph(&(*plot)->graph.phase);
    i_remove_graph(&(*plot)->graph.simu);
    heap_delete(plot, Plot);
}

/*---------------------------------------------------------------------------*/

static void i_clear(Plot *plot, DCtx *ctx, const R2Df *rect, const real32_t width, const real32_t height)
{
    color_t c[2];
    real32_t stop[2] = {0, 1};
    cassert_no_null(rect);
    c[0] = plot->colbg2;
    c[1] = plot->colbg3;
    draw_fill_linear(ctx, c, stop, 2, 0, 0, 0, height);
    draw_rect(ctx, ekFILL, 0.f, 0.f, width, height);
    c[0] = plot->colbg2;
    c[1] = plot->colbg1;
    draw_fill_linear(ctx, c, stop, 2, 0.f, rect->pos.y, 0.f, rect->pos.y + rect->size.height);
    draw_rect(ctx, ekFILL, rect->pos.x, rect->pos.y, rect->size.width, rect->size.height);
}

/*---------------------------------------------------------------------------*/

void plot_update(Plot *plot, const real32_t T, const uint32_t simulation_n)
{
    cassert_no_null(plot);
    plot->graph.db.box = kBOX2D_NULLf;
    plot->graph.phase.box = kBOX2D_NULLf;
    plot->graph.simu.box = kBOX2D_NULLf;
    plot->graph.simu.dn = simulation_n == UINT32_MAX ? plot->graph.simu.n : simulation_n;
    bode_db_graph(plot->graph.db.points, plot->graph.db.n);
    bode_phase_graph(plot->graph.phase.points, plot->graph.phase.n);
    bode_sim_graph(plot->graph.simu.points, plot->graph.simu.n);
    box2d_addnf(&plot->graph.db.box, plot->graph.db.points, plot->graph.db.n);
    box2d_addnf(&plot->graph.phase.box, plot->graph.phase.points, plot->graph.phase.n);
    box2d_addnf(&plot->graph.simu.box, plot->graph.simu.points, plot->graph.simu.n);
    plot->graph.phase.box.min.y = -365.f;
    plot->graph.phase.box.max.y = 5.f;

    if (simulation_n != UINT32_MAX)
    {
        real32_t y0 = plot->graph.simu.points[0].y;
        real32_t y1 = plot->graph.simu.points[simulation_n].y;
        plot->graph.simu.box.min.x = 0.f;
        plot->graph.simu.box.max.x = T;
        plot->graph.simu.box.min.y = min_r32(y0, y1);
        plot->graph.simu.box.max.y = max_r32(y0, y1);
    }
}

/*---------------------------------------------------------------------------*/

void plot_take_snap(Plot *plot)
{
    cassert_no_null(plot);
    i_graph_take_snap(&plot->graph.db);
    i_graph_take_snap(&plot->graph.phase);
    i_graph_take_snap(&plot->graph.simu);
    plot->snapshot = TRUE;
}

/*---------------------------------------------------------------------------*/

void plot_restore_snap(Plot *plot)
{
    cassert_no_null(plot);
    i_graph_restore_snap(&plot->graph.db);
    i_graph_restore_snap(&plot->graph.phase);
    i_graph_restore_snap(&plot->graph.simu);
    plot->snapshot = FALSE;
}

/*---------------------------------------------------------------------------*/

void plot_clear_snap(Plot *plot)
{
    cassert_no_null(plot);
    plot->snapshot = FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_x_steps(const Graph *graph, const R2Df *rect, const real32_t x_increment_step, const real32_t min_canvas_separation, real32_t *min_x_value, real32_t *x_increment, uint32_t *num_x_steps)
{
    real32_t num_x_cuts = 0.f;
    real32_t num_steps = 0.f;
    cassert_no_null(graph);
    cassert_no_null(rect);
    cassert(box2d_is_nullf(&graph->box) == FALSE);
    cassert_no_null(min_x_value);
    cassert_no_null(x_increment);
    cassert_no_null(num_x_steps);
    num_x_cuts = rect->size.width / min_canvas_separation;
    num_x_cuts = bmath_floorf(num_x_cuts);
    if (num_x_cuts == 0.f)
        num_x_cuts = 1.f;
    
    *x_increment = (graph->box.max.x - graph->box.min.x) / num_x_cuts;
    *x_increment = bmath_round_stepf(*x_increment, x_increment_step);
    if (*x_increment == .0f)
        *x_increment = x_increment_step;
    cassert(*x_increment >= x_increment_step);
    
    *min_x_value = bmath_round_stepf(graph->box.min.x, *x_increment);
    
    if (*min_x_value < graph->box.min.x)
        *min_x_value += *x_increment;
    
    cassert(*min_x_value >= graph->box.min.x);
    
    num_steps = (graph->box.max.x - *min_x_value) / *x_increment;
    *num_x_steps = (uint32_t)bmath_floorf(num_steps);
}

/*---------------------------------------------------------------------------*/

static void i_y_steps(const Graph *graph, const R2Df *rect, const real32_t y_increment_step, const real32_t min_canvas_separation, real32_t *min_y_value, real32_t *y_increment, uint32_t *num_y_steps)
{
    real32_t num_y_cuts = 0.f;
    real32_t num_steps = 0.f;
    cassert_no_null(graph);
    cassert_no_null(rect);
    cassert_no_null(min_y_value);
    cassert_no_null(y_increment);
    cassert_no_null(num_y_steps);
    num_y_cuts = rect->size.height / min_canvas_separation;
    num_y_cuts = bmath_floorf(num_y_cuts);
    if (num_y_cuts == 0.f)
        num_y_cuts = 1.f;
    
    *y_increment = (graph->box.max.y - graph->box.min.y) / num_y_cuts;
    *y_increment = bmath_round_stepf(*y_increment, y_increment_step);
    if (*y_increment == .0f)
        *y_increment = y_increment_step;
    cassert(*y_increment >= y_increment_step);

    *min_y_value = bmath_round_stepf(graph->box.min.y, *y_increment);
    
    if (*min_y_value < graph->box.min.y)
        *min_y_value += *y_increment;
    
    cassert(*min_y_value >= graph->box.min.y);
    
    num_steps = (graph->box.max.y - *min_y_value) / *y_increment;
    *num_y_steps = (uint32_t)bmath_floorf(num_steps);
}

/*---------------------------------------------------------------------------*/

static bool_t i_xy_from_canvas(const Graph *graph, const R2Df *rect, const real32_t canvas_x, V2Df *p)
{
    cassert_no_null(graph);
    cassert_no_null(rect);
    cassert_no_null(p);
    cassert(box2d_is_nullf(&graph->box) == FALSE);
    if (canvas_x < rect->pos.x || canvas_x > rect->pos.x + rect->size.width)
    {
        return FALSE;
    }
    else
    {
        real32_t norm_x, xsize;
        register uint32_t index;
        norm_x = (canvas_x - rect->pos.x) / rect->size.width;
        xsize = graph->box.max.x - graph->box.min.x;
        p->x = graph->box.min.x + norm_x * xsize;
        cassert(p->x >= graph->points[0].x);
        for (index = 1; index < graph->n; ++index)
        {
            if (p->x < graph->points[index].x)
            {
                index -= 1;
                break;
            }
        }
        
        // p->x == v2d[0].x
        if (index == 0)
        {
            p->y = graph->points[0].y;
            return TRUE;
        }
        // v2d[index-1].x < p->x < v2d[index].x
        else if (index < graph->n - 1)
        {
            register real32_t x0 = graph->points[index-1].x;
            register real32_t x1 = graph->points[index].x;
            register real32_t y0 = graph->points[index-1].y;
            register real32_t y1 = graph->points[index].y;
            p->y = y0 + (p->x - x0) * ((y1 - y0) / (x1 - x0));
            return TRUE;
        }
        // p->x is out of graph limits
        else
        {
            return FALSE;
        }
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_xy_to_canvas(const Graph *graph, const R2Df *rect, const V2Df *p, V2Df *canvas_p)
{
    V2Df norm;
    cassert_no_null(graph);
    cassert_no_null(rect);
    cassert_no_null(p);
    cassert_no_null(canvas_p);
    cassert(box2d_is_nullf(&graph->box) == FALSE);
    norm.x = (p->x - graph->box.min.x) / (graph->box.max.x - graph->box.min.x);
    if (norm.x < 0.f || norm.x > 1.f)
        return FALSE;
    
    norm.y = (p->y - graph->box.min.y) / (graph->box.max.y - graph->box.min.y);
    if (norm.y < 0.f || norm.y > 1.f)
        return FALSE;
    
    canvas_p->x = rect->pos.x + norm.x * rect->size.width;
    canvas_p->y = rect->pos.y + norm.y * rect->size.height;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_x_to_canvas(const Graph *graph, const R2Df *rect, const real32_t x, real32_t *canvas_x)
{
    real32_t norm;
    cassert_no_null(graph);
    cassert_no_null(rect);
    cassert_no_null(canvas_x);
    cassert(box2d_is_nullf(&graph->box) == FALSE);
    norm = (x - graph->box.min.x) / (graph->box.max.x - graph->box.min.x);
    if (norm < 0.f || norm > 1.f)
        return FALSE;
    
    *canvas_x = rect->pos.x + norm * rect->size.width;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_y_to_canvas(const Graph *graph, const R2Df *rect, const real32_t y, real32_t *canvas_y)
{
    real32_t norm;
    cassert_no_null(graph);
    cassert_no_null(rect);
    cassert_no_null(canvas_y);
    cassert(box2d_is_nullf(&graph->box) == FALSE);
    norm = (y - graph->box.min.y) / (graph->box.max.y - graph->box.min.y);
    if (norm < 0.f || norm > 1.f)
        return FALSE;
    
    *canvas_y = rect->pos.y + norm * rect->size.height;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_eval_mouse(Graph *graph, real32_t mouse_x, const R2Df *rect, V2Df *graph_point, V2Df *canvas_point)
{
    cassert_no_null(rect);
    cassert(mouse_x >= 0.f);
    cassert_no_null(graph_point);
    cassert_no_null(canvas_point);
    if (i_xy_from_canvas(graph, rect, mouse_x, graph_point) == TRUE)
    {
        if (i_xy_to_canvas(graph, rect, graph_point, canvas_point) == TRUE)
        {
        }
        else
        {
            canvas_point->x = REAL32_MAX;
            canvas_point->y = REAL32_MAX;
        }
    }
    else
    {
        graph_point->x = REAL32_MAX;
        graph_point->y = REAL32_MAX;
        canvas_point->x = REAL32_MAX;
        canvas_point->y = REAL32_MAX;
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_eval_inverse(const Graph *graph, const real32_t y, real32_t *x)
{
    register const V2Df *p;
    register uint32_t i;
    cassert_no_null(graph);
    cassert(graph->n > 1);
    cassert_no_null(x);
    p = graph->points + 1;
    for (i = 1; i < graph->n; ++i, ++p)
    {
        if ((p[-1].y <= y && p->y >= y) || (p[-1].y >= y && p->y <= y))
        {
            *x = p[-1].x + (y - p[-1].y) * ((p->x - p[-1].x) / (p->y - p[-1].y));
            return TRUE;
        }
    }
    
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_draw_graph(DCtx *ctx, Graph *graph, const bool_t main_graph, const uint32_t color, const R2Df *rect, const real32_t height)
{
    const V2Df *points = main_graph ? graph->points : graph->spoints;
    real32_t p0x, p0y, p1x, p1y;
    register uint32_t i, total;
    total = min_u32(graph->dn + 1, graph->n);
    draw_line_color(ctx, color);
    p0x = rect->pos.x + (rect->size.width * (points[0].x - graph->box.min.x) / (graph->box.max.x - graph->box.min.x));
    p0y = height - (rect->pos.y + (rect->size.height * (points[0].y - graph->box.min.y) / (graph->box.max.y - graph->box.min.y)));
    for (i = 1; i < total; ++i)
    {
        p1x = rect->pos.x + (rect->size.width * (points[i].x - graph->box.min.x) / (graph->box.max.x - graph->box.min.x));
        p1y = height - (rect->pos.y + (rect->size.height * (points[i].y - graph->box.min.y) / (graph->box.max.y - graph->box.min.y)));
        draw_line(ctx, p0x, p0y, p1x, p1y);
        p0x = p1x;
        p0y = p1y;
    }
}

/*---------------------------------------------------------------------------*/

void plot_mouse1_x(Plot *plot, const real32_t x)
{
    cassert_no_null(plot);
    cassert(x >= 0.f && x <= 1.f);
    plot->mouse1_x = x;
}

/*---------------------------------------------------------------------------*/

void plot_mouse2_x(Plot *plot, const real32_t x)
{
    cassert_no_null(plot);
    cassert(x >= 0.f && x <= 1.f);
    plot->mouse2_x = x;
}

/*---------------------------------------------------------------------------*/

void plot_draw_graph1(Plot *plot, DCtx *ctx, const real32_t width, const real32_t height)
{
    R2Df iframe;
    real32_t min_db_grid_x_value, x_db_grid_increment;
    uint32_t num_db_grid_x_steps;
    real32_t min_db_grid_y_value, y_db_grid_increment;
    uint32_t num_db_grid_y_steps;
    real32_t min_phase_grid_y_value, y_phase_grid_increment;
    uint32_t num_phase_grid_y_steps;
    V2Df db_point, db_canvas_point;
    V2Df phase_point, phase_canvas_point;

    iframe.pos.x = LMARGIN;
    iframe.pos.y = VMARGIN;
    iframe.size.width = width - LMARGIN - RMARGIN;
    iframe.size.height = height - 2.f * VMARGIN;

    /* Axes steps */
    i_x_steps(&plot->graph.db, &iframe, 0.5f, 50.f, &min_db_grid_x_value, &x_db_grid_increment, &num_db_grid_x_steps);
    i_y_steps(&plot->graph.db, &iframe, 5.f, 40.f, &min_db_grid_y_value, &y_db_grid_increment, &num_db_grid_y_steps);
    i_y_steps(&plot->graph.phase, &iframe, 45.f, 40.f, &min_phase_grid_y_value, &y_phase_grid_increment, &num_phase_grid_y_steps);

    /* Graphs evaluation */
    {
        real32_t mouse_x = plot->mouse1_x * width;
        i_eval_mouse(&plot->graph.db, mouse_x, &iframe, &db_point, &db_canvas_point);
        i_eval_mouse(&plot->graph.phase, mouse_x, &iframe, &phase_point, &phase_canvas_point);
    }

    /* Background */
    i_clear(plot, ctx, &iframe, width, height);

    /* Background grid */
    {
        real32_t graph_x, graph_y;
        V2Df p0, p1;
        real32_t pattern[2] = {2.f, 2.f};
        register uint32_t i;

        draw_line_color(ctx, plot->colaxis2);
        draw_line_dash(ctx, pattern, 2);
        
        graph_x = min_db_grid_x_value;
        p0.y = iframe.pos.y;
        p1.y = iframe.pos.y + iframe.size.height - 10.f;
        for (i = 0; i <= num_db_grid_x_steps; ++i)
        {
            real32_t canvas_x;
            if (i_x_to_canvas(&plot->graph.db, &iframe, graph_x, &canvas_x) == TRUE)
            {
                p0.x = bmath_roundf(canvas_x);
                p1.x = p0.x;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_x += x_db_grid_increment;
        }
       
        if (db_canvas_point.x != REAL32_MAX)
        {
            p0.x = p1.x = db_canvas_point.x;
            p1.y = iframe.pos.y + iframe.size.height;
            draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
        }
        
        graph_y = min_db_grid_y_value;
        p0.x = iframe.pos.x + 10.f;
        p1.x = iframe.pos.x + iframe.size.width;
        for (i = 0; i <= num_db_grid_y_steps; ++i)
        {
            real32_t canvas_y;
            if (i_y_to_canvas(&plot->graph.db, &iframe, graph_y, &canvas_y) == TRUE)
            {
                p0.y = height - bmath_roundf(canvas_y);
                p1.y = p0.y;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_y += y_db_grid_increment;
        }
        
        draw_line_dash(ctx, NULL, 0);
    }

    /* 0 dB line */
    {
        real32_t canvas_y = 0.f;
        if (i_y_to_canvas(&plot->graph.db, &iframe, 0.f, &canvas_y) == TRUE)
        {
            V2Df p0, p1;
            real32_t x_0db;
            p0.x = iframe.pos.x;
            p0.y = height - bmath_roundf(canvas_y);
            p1.x = iframe.pos.x + iframe.size.width;
            p1.y = p0.y;
            draw_line_color(ctx, plot->colgrap12);
            draw_line(ctx, p0.x, p0.y, p1.x, p1.y);

            /* Vertical 0 dB cutting edge */
            if (i_eval_inverse(&plot->graph.db, 0.f, &x_0db) == TRUE)
            {
                real32_t canvas_x = 0;
                i_x_to_canvas(&plot->graph.db, &iframe, x_0db, &canvas_x);
                p0.x = canvas_x;
                p0.y = iframe.pos.y;
                p1.x = p0.x;
                p1.y = iframe.pos.y + iframe.size.height;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
            }
        }
    }

    /* -180º line */
    {
        real32_t canvas_y = 0.f;
        if (i_y_to_canvas(&plot->graph.phase, &iframe, -180.f, &canvas_y) == TRUE)
        {
            V2Df p0, p1;
            real32_t x_180;
            p0.x = iframe.pos.x;
            p0.y = bmath_roundf(canvas_y);
            p1.x = iframe.pos.x + iframe.size.width;
            p1.y = p0.y;
            draw_line_color(ctx, plot->colgrap22);
            draw_line(ctx, p0.x, p0.y, p1.x, p1.y);

            // Vertical -180º cutting edge
            if (i_eval_inverse(&plot->graph.phase, -180.f, &x_180) == TRUE)
            {
                real32_t canvas_x = 0;
                i_x_to_canvas(&plot->graph.phase, &iframe, x_180, &canvas_x);
                p0.x = canvas_x;
                p0.y = iframe.pos.y;
                p1.x = p0.x;
                p1.y = iframe.pos.y + iframe.size.height;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
            }
        }
    }

    /* Graphs */
    if (plot->snapshot == TRUE)
    {
        draw_line_width(ctx, 2.f);
        i_draw_graph(ctx, &plot->graph.db, FALSE, plot->colgrap12, &iframe, height);
        i_draw_graph(ctx, &plot->graph.phase, FALSE, plot->colgrap22, &iframe, height);
        draw_line_width(ctx, 1.f);
    }

    i_draw_graph(ctx, &plot->graph.db, TRUE, plot->colgrap11, &iframe, height);
    i_draw_graph(ctx, &plot->graph.phase, TRUE, plot->colgrap21, &iframe, height);

    /* Graph mouse points */
    if (db_canvas_point.x != REAL32_MAX)
    {
        char_t text[16];
        align_t align = db_canvas_point.x < width / 2 ? ekLEFT : ekRIGHT;
        draw_fill_color(ctx, plot->colgrap11);
        draw_text_color(ctx, plot->colgrap11);
        draw_circle(ctx, ekFILL, db_canvas_point.x, height - db_canvas_point.y, 4);
        bstd_sprintf(text, sizeof(text), "%.3f", db_point.y);
        draw_text_align(ctx, align, ekBOTTOM);
        draw_text(ctx, text, db_canvas_point.x, height - db_canvas_point.y);
    }
        
    if (phase_canvas_point.x != REAL32_MAX)
    {
        char_t text[16];
        align_t align = db_canvas_point.x < width / 2 ? ekLEFT : ekRIGHT;
        draw_fill_color(ctx, plot->colgrap21);
        draw_text_color(ctx, plot->colgrap21);
        draw_circle(ctx, ekFILL, phase_canvas_point.x, height - phase_canvas_point.y, 4);
        bstd_sprintf(text, sizeof(text), "%.3f", phase_point.y);
        draw_text_align(ctx, align, ekBOTTOM);
        draw_text(ctx, text, phase_canvas_point.x, height - phase_canvas_point.y);
    }

    /* Axes */
    {
        //Font font;
        real32_t graph_x, graph_y;
        V2Df p0, p1;
        register uint32_t i;
        
        draw_line_color(ctx, plot->colaxis1);
        draw_text_color(ctx, plot->colaxis1);
        
        {
            V2Df pa0, pa1, pa2, pa3;
            pa0.x = iframe.pos.x;
            pa0.y = height - iframe.pos.y;
            pa1.x = iframe.pos.x + iframe.size.width;
            pa1.y = pa0.y;
            pa2.x = pa0.x;
            pa2.y = height - (iframe.pos.y + iframe.size.height);
            pa3.x = pa1.x;
            pa3.y = pa2.y;
            draw_line(ctx, pa0.x, pa0.y, pa1.x, pa1.y);
            draw_line(ctx, pa0.x, pa0.y, pa2.x, pa2.y);
            draw_line(ctx, pa1.x, pa1.y, pa3.x, pa3.y);
        }
        
        if (db_canvas_point.x != REAL32_MAX)
        {
            char_t text[16];
            align_t align = db_canvas_point.x < width / 2 ? ekLEFT : ekRIGHT;
            p0.x = bmath_roundf(db_canvas_point.x);
            p0.y = height - (iframe.pos.y + 30.f);
            bstd_sprintf(text, sizeof(text), "%.3f", db_point.x);
            draw_text_align(ctx, align, ekTOP);
        }
        
        graph_x = min_db_grid_x_value;
        p0.y = height - iframe.pos.y;
        p1.y = height - (iframe.pos.y + 10.f);
        draw_text_align(ctx, ekCENTER, ekTOP);

        for (i = 0; i <= num_db_grid_x_steps; ++i)
        {
            real32_t canvas_x = 0.f;
            if (i_x_to_canvas(&plot->graph.db, &iframe, graph_x, &canvas_x) == TRUE)
            {
                char_t text[16];
                p0.x = bmath_roundf(canvas_x);
                p1.x = p0.x;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
                bstd_sprintf(text, sizeof(text), "%.1f", graph_x);
                draw_text(ctx, text, p0.x, p0.y);
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_x += x_db_grid_increment;
        }
        
        graph_y = min_db_grid_y_value;
        p0.x = iframe.pos.x;
        p1.x = iframe.pos.x + 10.f;
        draw_text_align(ctx, ekRIGHT, ekCENTER);
        
        for (i = 0; i <= num_db_grid_y_steps; ++i)
        {
            real32_t canvas_y = 0.f;
            if (i_y_to_canvas(&plot->graph.db, &iframe, graph_y, &canvas_y) == TRUE)
            {
                char_t text[16];
                p0.y = height - bmath_roundf(canvas_y);
                p1.y = p0.y;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
                p0.x -= 2.f;
                bstd_sprintf(text, sizeof(text), "%.0f", graph_y);
                draw_text(ctx, text, p0.x, p0.y);
                p0.x = iframe.pos.x;
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_y += y_db_grid_increment;
        }
        
        graph_y = min_phase_grid_y_value;
        p0.x = iframe.pos.x + iframe.size.width;
        p1.x = p0.x - 10.f;
        draw_text_align(ctx, ekLEFT, ekCENTER);
        
        for (i = 0; i <= num_phase_grid_y_steps; ++i)
        {
            real32_t canvas_y = 0.f;
            if (i_y_to_canvas(&plot->graph.phase, &iframe, graph_y, &canvas_y) == TRUE)
            {
                char_t text[16];
                p0.y = height - bmath_roundf(canvas_y);
                p1.y = p0.y;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
                bstd_sprintf(text, sizeof(text), "%.0f", graph_y);
                draw_text(ctx, text, p0.x, p0.y);
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_y += y_phase_grid_increment;
        }
    }

    //if (width > 230.f)
    {
        V2Df text_pos;
        text_pos.x = LMARGIN;
        text_pos.y = 0.f;
        draw_text_align(ctx, ekRIGHT, ekTOP);
        draw_text_color(ctx, plot->colgrap11);
        draw_text(ctx, "dB", text_pos.x, text_pos.y);

        text_pos.x = width - RMARGIN;
        draw_text_color(ctx, plot->colgrap21);
        draw_text(ctx, "Phase", text_pos.x, text_pos.y);
    }
}

/*---------------------------------------------------------------------------*/

void plot_draw_graph2(Plot *plot, DCtx *ctx, const real32_t width, const real32_t height)
{
    R2Df iframe;
    real32_t min_simu_grid_x_value, x_simu_grid_increment;
    uint32_t num_simu_grid_x_steps;
    real32_t min_simu_grid_y_value, y_simu_grid_increment;
    uint32_t num_simu_grid_y_steps;
    V2Df simu_point, simu_canvas_point;

    iframe.pos.x = LMARGIN;
    iframe.pos.y = VMARGIN;
    iframe.size.width = width - LMARGIN - RMARGIN;
    iframe.size.height = height - 2.f * VMARGIN;

    /* Axes steps */
    i_x_steps(&plot->graph.simu, &iframe, 0.25f, 50.f, &min_simu_grid_x_value, &x_simu_grid_increment, &num_simu_grid_x_steps);
    i_y_steps(&plot->graph.simu, &iframe, 1.f, 40.f, &min_simu_grid_y_value, &y_simu_grid_increment, &num_simu_grid_y_steps);

    /* Graphs evaluation */
    {
        real32_t mouse_x = plot->mouse2_x * width;
        i_eval_mouse(&plot->graph.simu, mouse_x, &iframe, &simu_point, &simu_canvas_point);
    }

    /* Background */
    i_clear(plot, ctx, &iframe, width, height);

    /* Background grid */
    {
        real32_t graph_x, graph_y;
        V2Df p0, p1;
        real32_t pattern[2] = {2.f, 2.f};
        register uint32_t i;
        
        draw_line_color(ctx, plot->colaxis2);
        draw_line_dash(ctx, pattern, 2);
        
        graph_x = min_simu_grid_x_value;
        p0.y = iframe.pos.y;
        p1.y = iframe.pos.y + iframe.size.height - 10.f;
        for (i = 0; i <= num_simu_grid_x_steps; ++i)
        {
            real32_t canvas_x;
            if (i_x_to_canvas(&plot->graph.simu, &iframe, graph_x, &canvas_x) == TRUE)
            {
                p0.x = bmath_roundf(canvas_x);
                p1.x = p0.x;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_x += x_simu_grid_increment;
        }
        
        if (simu_canvas_point.x != REAL32_MAX)
        {
            p0.x = p1.x = simu_canvas_point.x;
            p1.y = iframe.pos.y + iframe.size.height;
            draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
        }
        
        graph_y = min_simu_grid_y_value;
        p0.x = iframe.pos.x + 10.f;
        p1.x = iframe.pos.x + iframe.size.width;
        for (i = 0; i <= num_simu_grid_y_steps; ++i)
        {
            real32_t canvas_y;
            if (i_y_to_canvas(&plot->graph.simu, &iframe, graph_y, &canvas_y) == TRUE)
            {
                p0.y = height - bmath_roundf(canvas_y);
                p1.y = p0.y;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_y += y_simu_grid_increment;
        }
        
        draw_line_dash(ctx, NULL, 0);
    }

    /* Graphs */
    if (plot->snapshot == TRUE)
    {
        draw_line_width(ctx, 2.f);
        i_draw_graph(ctx, &plot->graph.simu, FALSE, plot->colgrap12, &iframe, height);
        draw_line_width(ctx, 1.f);
    }

    i_draw_graph(ctx, &plot->graph.simu, TRUE, plot->colgrap11, &iframe, height);

    /* Graph mouse points */
    if (simu_canvas_point.x != REAL32_MAX)
    {
        char_t text[16];
        align_t align = simu_canvas_point.x < width / 2 ? ekLEFT : ekRIGHT;
        draw_fill_color(ctx, plot->colgrap11);
        draw_text_color(ctx, plot->colgrap11);
        draw_circle(ctx, ekFILL, simu_canvas_point.x, height - simu_canvas_point.y, 4);
        bstd_sprintf(text, sizeof(text), "%.3f", simu_point.y);
        draw_text_align(ctx, align, ekBOTTOM);
        draw_text(ctx, text, simu_canvas_point.x, height - simu_canvas_point.y);
    }

    /* Axes */
    {
        real32_t graph_x, graph_y;
        V2Df p0, p1;
        register uint32_t i;
        
        draw_line_color(ctx, plot->colaxis1);
        draw_text_color(ctx, plot->colaxis1);
        
        {
            V2Df pa0, pa1, pa2, pa3;
            pa0.x = iframe.pos.x;
            pa0.y = height - iframe.pos.y;
            pa1.x = iframe.pos.x + iframe.size.width;
            pa1.y = pa0.y;
            pa2.x = pa0.x;
            pa2.y = height - (iframe.pos.y + iframe.size.height);
            pa3.x = pa1.x;
            pa3.y = pa2.y;
            draw_line(ctx, pa0.x, pa0.y, pa1.x, pa1.y);
            draw_line(ctx, pa0.x, pa0.y, pa2.x, pa2.y);
            draw_line(ctx, pa1.x, pa1.y, pa3.x, pa3.y);
        }

        {
            V2Df pa0, pa1, pa2, pa3;
            pa0.x = iframe.pos.x;
            pa0.y = height - iframe.pos.y;
            pa1.x = iframe.pos.x + iframe.size.width;
            pa1.y = pa0.y;
            pa2.x = pa0.x;
            pa2.y = height - (iframe.pos.y + iframe.size.height);
            pa3.x = pa1.x;
            pa3.y = pa2.y;
            draw_line(ctx, pa0.x, pa0.y, pa1.x, pa1.y);
            draw_line(ctx, pa0.x, pa0.y, pa2.x, pa2.y);
            draw_line(ctx, pa1.x, pa1.y, pa3.x, pa3.y);
        }
        
        if (simu_canvas_point.x != REAL32_MAX)
        {
            char_t text[16];
            align_t align = simu_canvas_point.x < width / 2 ? ekLEFT : ekRIGHT;
            p0.x = bmath_roundf(simu_canvas_point.x);
            p0.y = height - (iframe.pos.y + 30.f);
            bstd_sprintf(text, sizeof(text), "%.3f", simu_point.x);
            draw_text_align(ctx, align, ekTOP);
            draw_text(ctx, text, p0.x, p0.y);
        }

        graph_x = min_simu_grid_x_value;
        p0.y = height - iframe.pos.y;
        p1.y = height - (iframe.pos.y + 10.f);
        draw_text_align(ctx, ekCENTER, ekTOP);

        for (i = 0; i <= num_simu_grid_x_steps; ++i)
        {
            real32_t canvas_x = 0.f;
            if (i_x_to_canvas(&plot->graph.simu, &iframe, graph_x, &canvas_x) == TRUE)
            {
                char_t text[16];
                p0.x = bmath_roundf(canvas_x);
                p1.x = p0.x;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
                bstd_sprintf(text, sizeof(text), "%.1f", graph_x);
                draw_text(ctx, text, p0.x, p0.y);
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_x += x_simu_grid_increment;
        }

        graph_y = min_simu_grid_y_value;
        p0.x = iframe.pos.x;
        p1.x = iframe.pos.x + 10.f;
        draw_text_align(ctx, ekRIGHT, ekCENTER);
        
        for (i = 0; i <= num_simu_grid_y_steps; ++i)
        {
            real32_t canvas_y = 0.f;
            if (i_y_to_canvas(&plot->graph.simu, &iframe, graph_y, &canvas_y) == TRUE)
            {
                char_t text[16];
                p0.y = height - bmath_roundf(canvas_y);
                p1.y = p0.y;
                draw_line(ctx, p0.x, p0.y, p1.x, p1.y);
                p0.x -= 2.f;
                bstd_sprintf(text, sizeof(text), "%.0f", graph_y);
                draw_text(ctx, text, p0.x, p0.y);
                p0.x = iframe.pos.x;
            }
            else
            {
                cassert(FALSE);
            }
            
            graph_y += y_simu_grid_increment;
        }
    }

    {
        V2Df text_pos;
        text_pos.x = 5.f;
        text_pos.y = 0.f;
        draw_text_color(ctx, plot->colgrap11);
        draw_text_align(ctx, ekLEFT, ekTOP);
        draw_text(ctx, "y", text_pos.x, text_pos.y);
    }
}

/*---------------------------------------------------------------------------*/

