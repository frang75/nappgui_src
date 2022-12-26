/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawhello.c
 *
 */

/* Drawing primitives */

#include "nappgui.h"
#include "res_drawhello.h"

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    View *view;
    Label *label;
    Cell *slider;
    uint32_t option;
    real32_t gradient;
};

/*---------------------------------------------------------------------------*/

static void i_draw_lines(DCtx *ctx)
{
    const V2Df poly1[] = { { 10, 190}, { 90, 110}, {110, 190}, {190, 110}, {210, 190}, {290, 110} };
    const V2Df poly2[] = { {310, 190}, {390, 110}, {410, 190}, {490, 110}, {510, 190}, {590, 110} };
    const V2Df poly3[] = { { 10, 290}, { 90, 210}, {110, 290}, {190, 210}, {210, 290}, {290, 210} };
    const real32_t pattern1[] = { 5, 5, 10, 5 };
    const real32_t pattern2[] = { 1, 1 };
    const real32_t pattern3[] = { 2, 1 };
    const real32_t pattern4[] = { 1, 2 };

    /* Line widths */
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_line_width(ctx, 5);
    draw_line(ctx, 10, 90, 90, 10);
    draw_line_width(ctx, 10);
    draw_line(ctx, 110, 90, 190, 10);
    draw_line_width(ctx, 15);
    draw_line(ctx, 210, 90, 290, 10);

    /* Line caps */
    draw_line_cap(ctx, ekLCFLAT);
    draw_line(ctx, 310, 90, 390, 10);
    draw_line_cap(ctx, ekLCSQUARE);
    draw_line(ctx, 410, 90, 490, 10);
    draw_line_cap(ctx, ekLCROUND);
    draw_line(ctx, 510, 90, 590, 10);

    /* Line joins */
    draw_line_width(ctx, 15);
    draw_line_cap(ctx, ekLCFLAT);
    draw_line_join(ctx, ekLJMITER);
    draw_polyline(ctx, FALSE, poly1, 6);
    draw_line_cap(ctx, ekLCSQUARE);
    draw_line_join(ctx, ekLJROUND);
    draw_polyline(ctx, FALSE, poly2, 6);
    draw_line_cap(ctx, ekLCROUND);
    draw_line_join(ctx, ekLJBEVEL);
    draw_polyline(ctx, FALSE, poly3, 6);

    /* Line colors */
    draw_line_width(ctx, 10);
    draw_line_cap(ctx, ekLCFLAT);
    draw_line_color(ctx, kCOLOR_RED);
    draw_line(ctx, 310, 215, 590, 215);
    draw_line_color(ctx, kCOLOR_GREEN);
    draw_line(ctx, 310, 235, 590, 235);
    draw_line_color(ctx, kCOLOR_BLUE);
    draw_line(ctx, 310, 255, 590, 255);
    draw_line_width(ctx, 5);
    draw_line_color(ctx, kCOLOR_YELLOW);
    draw_line(ctx, 310, 270, 590, 270);
    draw_line_color(ctx, kCOLOR_CYAN);
    draw_line(ctx, 310, 280, 590, 280);
    draw_line_color(ctx, kCOLOR_MAGENTA);
    draw_line(ctx, 310, 290, 590, 290);

    /* Line patterns */
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_line_width(ctx, 5);
    draw_line_cap(ctx, ekLCFLAT);
    draw_line_dash(ctx, pattern1, 4);
    draw_line(ctx, 10, 310, 590, 310);
    draw_line_dash(ctx, pattern2, 2);
    draw_line(ctx, 10, 330, 590, 330);
    draw_line_dash(ctx, pattern3, 2);
    draw_line(ctx, 10, 350, 590, 350);
    draw_line_dash(ctx, pattern4, 2);
    draw_line_width(ctx, 2);
    draw_line(ctx, 10, 365, 590, 365);
    draw_line_dash(ctx, pattern1, 4);
    draw_line_width(ctx, 1);
    draw_line(ctx, 10, 375, 590, 375);
    draw_line_dash(ctx, NULL, 0);
    draw_line(ctx, 10, 385, 590, 385);

    /* Thin lines in centers */
    draw_line_dash(ctx, NULL, 0);
    draw_line_color(ctx, color_rgb(255, 255, 255));
    draw_line_width(ctx, 1);
    draw_line(ctx, 10, 90, 90, 10);
    draw_line(ctx, 110, 90, 190, 10);
    draw_line(ctx, 210, 90, 290, 10);
    draw_line(ctx, 310, 90, 390, 10);
    draw_line(ctx, 410, 90, 490, 10);
    draw_line(ctx, 510, 90, 590, 10);
    draw_polyline(ctx, FALSE, poly1, 6);
    draw_polyline(ctx, FALSE, poly2, 6);
    draw_polyline(ctx, FALSE, poly3, 6);
}

/*---------------------------------------------------------------------------*/

static void i_draw_shapes_row(DCtx *ctx, const drawop_t op, const T2Df *origin)
{
    const V2Df poly[] = { {40, 0}, {12.36f, 38.04f}, {-32.36f, 23.52f},
                          {-32.36f, -23.52f}, {12.36f, -38.04f} };
    T2Df matrix;
    draw_rect(ctx, op, 10, 10, 110, 75);
    draw_rndrect(ctx, op, 140, 10, 110, 75, 20);
    draw_circle(ctx, op, 312, 50, 40);
    draw_ellipse(ctx, op, 430, 50, 55, 37);
    t2d_movef(&matrix, origin, 547, 50);
    t2d_rotatef(&matrix, &matrix, - kBMATH_PIf / 10);
    draw_matrixf(ctx, &matrix);
    draw_polygon(ctx, op, poly, 5);
}

/*---------------------------------------------------------------------------*/

static void i_draw_shapes(DCtx *ctx, const bool_t grad)
{
    T2Df origin = *kT2D_IDENTf;
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_line_width(ctx, 10);
    draw_matrixf(ctx, &origin);
    i_draw_shapes_row(ctx, grad ? ekSKFILL : ekSTROKE, &origin);
    t2d_movef(&origin, &origin, 0, 100);
    draw_matrixf(ctx, &origin);
    i_draw_shapes_row(ctx, grad ? ekSKFILL : ekFILL, &origin);
    t2d_movef(&origin, &origin, 0, 100);
    draw_matrixf(ctx, &origin);
    i_draw_shapes_row(ctx, grad ? ekSKFILL : ekSKFILL, &origin);
    t2d_movef(&origin, &origin, 0, 100);
    draw_matrixf(ctx, &origin);
    i_draw_shapes_row(ctx, grad ? ekSKFILL : ekFILLSK, &origin);
}

/*---------------------------------------------------------------------------*/

static void i_draw_gradient(DCtx *ctx, const real32_t gradient, const bool_t back, const bool_t shapes)
{
    color_t c[2];
    real32_t stop[2] = {0, 1};
    real32_t gpos;
    real32_t gx, gy;
    c[0] = kCOLOR_RED;
    c[1] = kCOLOR_BLUE;

    gpos = gradient * (600 + 400);

    if (gpos < 400)
    {
        gx = 600;
        gy = gpos;
    }
    else
    {
        gx = 600 - (gpos - 400);
        gy = 400;
    }

    draw_fill_linear(ctx, c, stop, 2, 0, 0, gx, gy);

    if (back == TRUE)
        draw_rect(ctx, ekFILL, 0, 0, 600, 400);

    if (shapes == TRUE)
        i_draw_shapes(ctx, TRUE);

    draw_matrixf(ctx, kT2D_IDENTf);
    draw_line_width(ctx, 3);
    draw_line_color(ctx, color_rgb(200, 200, 200));
    draw_line(ctx, 3, 3, gx + 3, gy + 3);
}

/*---------------------------------------------------------------------------*/

static void i_draw_lines_gradient(DCtx *ctx, const real32_t gradient)
{
    color_t c[2];
    real32_t stop[2] = {0, 1};
    real32_t gpos;
    real32_t gx, gy;
    const real32_t pattern1[] = { 5, 5, 10, 5 };
    const real32_t pattern2[] = { 1, 1 };
    const real32_t pattern3[] = { 2, 1 };
    const real32_t pattern4[] = { 1, 2 };

    c[0] = kCOLOR_RED;
    c[1] = kCOLOR_BLUE;

    gpos = gradient * (600 + 400);

    if (gpos < 400)
    {
        gx = 600;
        gy = gpos;
    }
    else
    {
        gx = 600 - (gpos - 400);
        gy = 400;
    }

    draw_line_width(ctx, 10);
    draw_line_fill(ctx);
    draw_fill_linear(ctx, c, stop, 2, 0, 0, gx, gy);
    i_draw_shapes_row(ctx, ekSTROKE, kT2D_IDENTf);

    draw_matrixf(ctx, kT2D_IDENTf);
    draw_line_width(ctx, 1);
    draw_bezier(ctx, 30, 190, 140, 50, 440, 110, 570, 190);
    draw_line_width(ctx, 4);
    draw_bezier(ctx, 30, 210, 140, 70, 440, 130, 570, 210);
    draw_line_width(ctx, 7);
    draw_bezier(ctx, 30, 230, 140, 90, 440, 150, 570, 230);
    draw_line_width(ctx, 10);
    draw_bezier(ctx, 30, 250, 140, 110, 440, 170, 570, 250);

    draw_line_width(ctx, 8);
    draw_arc(ctx, 100, 280, 60, 0, - kBMATH_PIf / 2);
    draw_arc(ctx, 250, 280, 60, kBMATH_PIf, kBMATH_PIf / 2);
    draw_arc(ctx, 300, 220, 60, kBMATH_PIf / 2, - kBMATH_PIf / 2);
    draw_arc(ctx, 450, 220, 60, kBMATH_PIf / 2, kBMATH_PIf / 2);

    draw_line_width(ctx, 5);
    draw_line_cap(ctx, ekLCFLAT);
    draw_line_dash(ctx, pattern1, 4);
    draw_line(ctx, 10, 310, 590, 310);
    draw_line_dash(ctx, pattern2, 2);
    draw_line(ctx, 10, 330, 590, 330);
    draw_line_dash(ctx, pattern3, 2);
    draw_line(ctx, 10, 350, 590, 350);
    draw_line_dash(ctx, pattern4, 2);
    draw_line_width(ctx, 2);
    draw_line(ctx, 10, 365, 590, 365);
    draw_line_dash(ctx, pattern1, 4);
    draw_line_width(ctx, 1);
    draw_line(ctx, 10, 375, 590, 375);
    draw_line_dash(ctx, NULL, 0);
    draw_line(ctx, 10, 385, 590, 385);

    draw_line_width(ctx, 1);
    draw_line_color(ctx, color_rgb(50, 50, 50));
    draw_line(ctx, 3, 3, gx + 3, gy + 3);
}

/*---------------------------------------------------------------------------*/

static void i_draw_local_gradient(DCtx *ctx, const real32_t gradient)
{
    color_t c[2];
    real32_t stop[2] = {0, 1};
    real32_t gpos;
    real32_t gx, gy;
    T2Df matrix;

    c[0] = kCOLOR_RED;
    c[1] = kCOLOR_BLUE;

    gpos = gradient * (200 + 100);

    if (gpos < 100)
    {
        gx = 200;
        gy = gpos;
    }
    else
    {
        gx = 200 - (gpos - 100);
        gy = 100;
    }

    draw_line_join(ctx, ekLJROUND);
    draw_fill_linear(ctx, c, stop, 2, 0, 0, gx, gy);

    t2d_movef(&matrix, kT2D_IDENTf, 50, 40);
    draw_matrixf(ctx, &matrix);
    draw_fill_matrix(ctx, &matrix);
    draw_line_width(ctx, 10);
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_rect(ctx, ekSKFILL, 0, 0, 200, 100);
    draw_line_width(ctx, 3);
    draw_line_color(ctx, color_rgb(200, 200, 200));
    draw_line(ctx, 0, 0, gx, gy);

    t2d_movef(&matrix, kT2D_IDENTf, 400, 40);
    t2d_rotatef(&matrix, &matrix, kBMATH_PIf / 6);
    draw_matrixf(ctx, &matrix);
    draw_fill_matrix(ctx, &matrix);
    draw_line_width(ctx, 10);
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_rect(ctx, ekSKFILL, 0, 0, 200, 100);
    draw_line_width(ctx, 3);
    draw_line_color(ctx, color_rgb(200, 200, 200));
    draw_line(ctx, 0, 0, gx, gy);

    t2d_movef(&matrix, kT2D_IDENTf, 250, 280);
    t2d_rotatef(&matrix, &matrix, - kBMATH_PIf / 10);
    draw_matrixf(ctx, &matrix);
    t2d_movef(&matrix, &matrix, -100, -50);
    draw_fill_matrix(ctx, &matrix);
    draw_line_width(ctx, 10);
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_ellipse(ctx, ekSKFILL, 0, 0, 100, 50);
    draw_matrixf(ctx, &matrix);
    draw_line_width(ctx, 3);
    draw_line_color(ctx, color_rgb(200, 200, 200));
    draw_line(ctx, 0, 0, gx, gy);
}

/*---------------------------------------------------------------------------*/

static void i_draw_wrap_gradient(DCtx *ctx)
{
    color_t c[2];
    real32_t stop[2] = {0, 1};
    c[0] = kCOLOR_RED;
    c[1] = kCOLOR_BLUE;
    draw_line_width(ctx, 2);
    draw_fill_linear(ctx, c, stop, 2, 200, 0, 400, 0);
    draw_fill_wrap(ctx, ekFCLAMP);
    draw_rect(ctx, ekFILLSK, 50, 25, 500, 100);
    draw_fill_wrap(ctx, ekFTILE);
    draw_rect(ctx, ekFILLSK, 50, 150, 500, 100);
    draw_fill_wrap(ctx, ekFFLIP);
    draw_rect(ctx, ekFILLSK, 50, 275, 500, 100);
}

/*---------------------------------------------------------------------------*/

static void i_text_single(DCtx *ctx)
{
    Font *font = font_system(20, 0);
    const char_t *text = "Text 文本 Κείμενο";
    real32_t width, height;
    T2Df matrix;

    draw_font(ctx, font);
    draw_text_extents(ctx, text, -1, &width, &height);
    draw_text_color(ctx, kCOLOR_BLUE);
    draw_text_align(ctx, ekLEFT, ekTOP);
    draw_text(ctx, text, 25, 25);
    draw_text_align(ctx, ekCENTER, ekTOP);
    draw_text(ctx, text, 300, 25);
    draw_text_align(ctx, ekRIGHT, ekTOP);
    draw_text(ctx, text, 575, 25);
    draw_text_align(ctx, ekLEFT, ekCENTER);
    draw_text(ctx, text, 25, 100);
    draw_text_align(ctx, ekCENTER, ekCENTER);
    draw_text(ctx, text, 300, 100);
    draw_text_align(ctx, ekRIGHT, ekCENTER);
    draw_text(ctx, text, 575, 100);
    draw_text_align(ctx, ekLEFT, ekBOTTOM);
    draw_text(ctx, text, 25, 175);
    draw_text_align(ctx, ekCENTER, ekBOTTOM);
    draw_text(ctx, text, 300, 175);
    draw_text_align(ctx, ekRIGHT, ekBOTTOM);
    draw_text(ctx, text, 575, 175);

    draw_line_color(ctx, kCOLOR_RED);
    draw_fill_color(ctx, kCOLOR_RED);
    draw_circle(ctx, ekFILL, 25, 25, 3);
    draw_circle(ctx, ekFILL, 300, 25, 3);
    draw_circle(ctx, ekFILL, 575, 25, 3);
    draw_circle(ctx, ekFILL, 25, 100, 3);
    draw_circle(ctx, ekFILL, 300, 100, 3);
    draw_circle(ctx, ekFILL, 575, 100, 3);
    draw_circle(ctx, ekFILL, 25, 175, 3);
    draw_circle(ctx, ekFILL, 300, 175, 3);
    draw_circle(ctx, ekFILL, 575, 175, 3);
    draw_circle(ctx, ekFILL, 25, 200, 3);
    draw_circle(ctx, ekFILL, 300, 250, 3);
    draw_circle(ctx, ekFILL, 25, 325, 3);
    draw_circle(ctx, ekFILL, 575, 200, 3);
    draw_circle(ctx, ekFILL, 575, 230, 3);
    draw_circle(ctx, ekFILL, 575, 260, 3);
    draw_rect(ctx, ekSTROKE, 25, 25, width, height);
    draw_rect(ctx, ekSTROKE, 300 - (width / 2), 25, width, height);
    draw_rect(ctx, ekSTROKE, 575 - width, 25, width, height);
    draw_rect(ctx, ekSTROKE, 25, 100 - (height / 2), width, height);
    draw_rect(ctx, ekSTROKE, 300 - (width / 2), 100 - (height / 2), width, height);
    draw_rect(ctx, ekSTROKE, 575 - width, 100 - (height / 2), width, height);
    draw_rect(ctx, ekSTROKE, 25, 175 - height, width, height);
    draw_rect(ctx, ekSTROKE, 300 - (width / 2), 175 - height, width, height);
    draw_rect(ctx, ekSTROKE, 575 - width, 175 - height, width, height);

    draw_fill_color(ctx, kCOLOR_BLUE);
    t2d_movef(&matrix, kT2D_IDENTf, 25, 200);
    t2d_rotatef(&matrix, &matrix, kBMATH_PIf / 8);
    draw_matrixf(ctx, &matrix);
    draw_text_align(ctx, ekLEFT, ekTOP);
    draw_text(ctx, text, 0, 0);

    t2d_movef(&matrix, kT2D_IDENTf, 300, 250);
    t2d_rotatef(&matrix, &matrix, - kBMATH_PIf / 8);
    draw_matrixf(ctx, &matrix);
    draw_text_align(ctx, ekCENTER, ekCENTER);
    draw_text(ctx, text, 0, 0);

    t2d_movef(&matrix, kT2D_IDENTf, 25, 325);
    t2d_scalef(&matrix, &matrix, 3, 1);
    draw_matrixf(ctx, &matrix);
    draw_text_align(ctx, ekLEFT, ekTOP);
    draw_text(ctx, text, 0, 0);

    t2d_movef(&matrix, kT2D_IDENTf, 575, 200);
    t2d_scalef(&matrix, &matrix, .5f, 1);
    draw_matrixf(ctx, &matrix);
    draw_text_align(ctx, ekRIGHT, ekTOP);
    draw_text(ctx, text, 0, 0);

    t2d_movef(&matrix, kT2D_IDENTf, 575, 230);
    t2d_scalef(&matrix, &matrix, .75f, 1);
    draw_matrixf(ctx, &matrix);
    draw_text_align(ctx, ekRIGHT, ekTOP);
    draw_text(ctx, text, 0, 0);

    t2d_movef(&matrix, kT2D_IDENTf, 575, 260);
    t2d_scalef(&matrix, &matrix, 1.25f, 1);
    draw_matrixf(ctx, &matrix);
    draw_text_align(ctx, ekRIGHT, ekTOP);
    draw_text(ctx, text, 0, 0);

    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static void i_text_newline(DCtx *ctx)
{
    Font *font = font_system(20, 0);
    const char_t *text = "Text new line\n文字换行\nΓραμμή κειμένου";
    real32_t width, height;
    draw_font(ctx, font);
    draw_text_extents(ctx, text, -1, &width, &height);

    draw_text_color(ctx, kCOLOR_BLUE);
    draw_text_align(ctx, ekLEFT, ekTOP);
    draw_text_halign(ctx, ekLEFT);
    draw_text(ctx, text, 25, 25);
    draw_text_align(ctx, ekCENTER, ekTOP);
    draw_text_halign(ctx, ekCENTER);
    draw_text(ctx, text, 300, 25);

    draw_text_align(ctx, ekRIGHT, ekTOP);
    draw_text_halign(ctx, ekRIGHT);
    draw_text(ctx, text, 575, 25);
    draw_text_align(ctx, ekLEFT, ekCENTER);
    draw_text_halign(ctx, ekLEFT);
    draw_text(ctx, text, 25, 175);
    draw_text_align(ctx, ekCENTER, ekCENTER);
    draw_text_halign(ctx, ekCENTER);
    draw_text(ctx, text, 300, 175);
    draw_text_align(ctx, ekRIGHT, ekCENTER);
    draw_text_halign(ctx, ekRIGHT);
    draw_text(ctx, text, 575, 175);
    draw_text_align(ctx, ekLEFT, ekBOTTOM);
    draw_text_halign(ctx, ekLEFT);
    draw_text(ctx, text, 25, 325);
    draw_text_align(ctx, ekCENTER, ekBOTTOM);
    draw_text_halign(ctx, ekCENTER);
    draw_text(ctx, text, 300, 325);
    draw_text_align(ctx, ekRIGHT, ekBOTTOM);
    draw_text_halign(ctx, ekRIGHT);
    draw_text(ctx, text, 575, 325);

    draw_line_color(ctx, kCOLOR_RED);
    draw_fill_color(ctx, kCOLOR_RED);
    draw_circle(ctx, ekFILL, 25, 25, 3);
    draw_circle(ctx, ekFILL, 300, 25, 3);
    draw_circle(ctx, ekFILL, 575, 25, 3);
    draw_circle(ctx, ekFILL, 25, 175, 3);
    draw_circle(ctx, ekFILL, 300, 175, 3);
    draw_circle(ctx, ekFILL, 575, 175, 3);
    draw_circle(ctx, ekFILL, 25, 325, 3);
    draw_circle(ctx, ekFILL, 300, 325, 3);
    draw_circle(ctx, ekFILL, 575, 325, 3);
    draw_rect(ctx, ekSTROKE, 25, 25, width, height);
    draw_rect(ctx, ekSTROKE, 300 - (width / 2), 25, width, height);
    draw_rect(ctx, ekSTROKE, 575 - width, 25, width, height);
    draw_rect(ctx, ekSTROKE, 25, 175 - (height / 2), width, height);
    draw_rect(ctx, ekSTROKE, 300 - (width / 2), 175 - (height / 2), width, height);
    draw_rect(ctx, ekSTROKE, 575 - width, 175 - (height / 2), width, height);
    draw_rect(ctx, ekSTROKE, 25, 325 - height, width, height);
    draw_rect(ctx, ekSTROKE, 300 - (width / 2), 325 - height, width, height);
    draw_rect(ctx, ekSTROKE, 575 - width, 325 - height, width, height);
    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static void i_text_block(DCtx *ctx)
{
    const char_t *text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";
    real32_t dash[2] = {1, 1};
    real32_t width1, height1;
    real32_t width2, height2;
    real32_t width3, height3;
    real32_t width4, height4;

    draw_text_color(ctx, kCOLOR_BLUE);
    draw_text_align(ctx, ekLEFT, ekTOP);
    draw_text_halign(ctx, ekLEFT);
    draw_text_width(ctx, 200);
    draw_text_extents(ctx, text, 200, &width1, &height1);
    draw_text(ctx, text, 25, 25);
    draw_text_width(ctx, 300);
    draw_text_extents(ctx, text, 300, &width2, &height2);
    draw_text(ctx, text, 250, 25);
    draw_text_width(ctx, 400);
    draw_text_extents(ctx, text, 400, &width3, &height3);
    draw_text(ctx, text, 25, 200);
    draw_text_width(ctx, 500);
    draw_text_extents(ctx, text, 500, &width4, &height4);
    draw_text(ctx, text, 25, 315);

    draw_line_color(ctx, kCOLOR_RED);
    draw_fill_color(ctx, kCOLOR_RED);
    draw_circle(ctx, ekFILL, 25, 25, 3);
    draw_circle(ctx, ekFILL, 250, 25, 3);
    draw_circle(ctx, ekFILL, 25, 200, 3);
    draw_circle(ctx, ekFILL, 25, 315, 3);
    draw_rect(ctx, ekSTROKE, 25, 25, 200, height1);
    draw_rect(ctx, ekSTROKE, 250, 25, 300, height2);
    draw_rect(ctx, ekSTROKE, 25, 200, 400, height3);
    draw_rect(ctx, ekSTROKE, 25, 315, 500, height4);    
    draw_line_dash(ctx, dash, 2);
    draw_rect(ctx, ekSTROKE, 25, 25, width1, height1);
    draw_rect(ctx, ekSTROKE, 250, 25, width2, height2);
    draw_rect(ctx, ekSTROKE, 25, 200, width3, height3);
    draw_rect(ctx, ekSTROKE, 25, 315, width4, height4);
}

/*---------------------------------------------------------------------------*/

static void i_text_art(DCtx *ctx)
{
    Font *font = font_system(50, 0);
    color_t c[2];
    real32_t stop[2] = {0, 1};
    real32_t dash[2] = {1, 1};
    real32_t width, height;
    c[0] = kCOLOR_BLUE;
    c[1] = kCOLOR_RED;
    draw_font(ctx, font);
    draw_line_width(ctx, 2);
    draw_line_color(ctx, kCOLOR_WHITE);
    draw_fill_color(ctx, kCOLOR_BLUE);
    draw_text_path(ctx, ekFILLSK, "Fill and Stoke text", 25, 25);
    draw_text_extents(ctx, "Gradient fill text", -1, &width, &height);
    draw_fill_linear(ctx, c, stop, 2, 25, 0, 25 + width, 0);
    draw_fill_matrix(ctx, kT2D_IDENTf);
    draw_text_path(ctx, ekFILL, "Gradient fill text", 25, 100);
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_line_dash(ctx, dash, 2);
    draw_text_path(ctx, ekSTROKE, "Dashed stroke text", 25, 175);
    draw_line_color(ctx, kCOLOR_GREEN);
    draw_text_extents(ctx, "Gradient dashed text", -1, &width, &height);
    draw_fill_linear(ctx, c, stop, 2, 25, 0, 25 + width, 0);
    draw_text_path(ctx, ekFILLSK, "Gradient dashed text", 25, 250);
    draw_line_color(ctx, kCOLOR_BLACK);
    draw_line_width(ctx, .5f);
    draw_line_dash(ctx, NULL, 0);
    draw_text_path(ctx, ekSTROKE, "Thin stroke text", 25, 325);
    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static void i_image(DCtx *ctx)
{
    ResPack *pack = res_drawhello_respack("");
    const Image *image = image_from_resource(pack, IMAGE_PNG);
    T2Df matrix;

    draw_image_align(ctx, ekLEFT, ekTOP);
    draw_image(ctx, image, 25, 25);
    t2d_movef(&matrix, kT2D_IDENTf, 300, 200);
    t2d_rotatef(&matrix, &matrix, kBMATH_PIf / 8);
    draw_image_align(ctx, ekCENTER, ekCENTER);
    draw_matrixf(ctx, &matrix);
    draw_image(ctx, image, 0, 0);
    draw_matrixf(ctx, kT2D_IDENTf);
    draw_image_align(ctx, ekRIGHT, ekTOP);
    draw_image(ctx, image, 575, 25);
    draw_image_align(ctx, ekLEFT, ekBOTTOM);
    draw_image(ctx, image, 25, 375);
    draw_image_align(ctx, ekRIGHT, ekBOTTOM);
    draw_image(ctx, image, 575, 375);

    draw_fill_color(ctx, kCOLOR_BLUE);
    draw_circle(ctx, ekFILL, 25, 25, 3);
    draw_circle(ctx, ekFILL, 300, 200, 3);
    draw_circle(ctx, ekFILL, 575, 25, 3);
    draw_circle(ctx, ekFILL, 25, 375, 3);
    draw_circle(ctx, ekFILL, 575, 375, 3);
    respack_destroy(&pack);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(App *app, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    draw_clear(p->ctx, color_rgb(200, 200, 200));
    switch (app->option) {
    case 0:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Different line styles: width, join, cap, dash...");
        i_draw_lines(p->ctx);
        break;
    case 1:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Basic shapes filled and stroke.");
        draw_fill_color(p->ctx, kCOLOR_BLUE);
        i_draw_shapes(p->ctx, FALSE);
        break;
    case 2:
        cell_enabled(app->slider, TRUE);
        label_text(app->label, "Global linear gradient.");
        i_draw_gradient(p->ctx, app->gradient, TRUE, FALSE);
        break;
    case 3:
        cell_enabled(app->slider, TRUE);
        label_text(app->label, "Shapes filled with global (identity) linear gradient.");
        i_draw_gradient(p->ctx, app->gradient, TRUE, TRUE);
        break;
    case 4:
        cell_enabled(app->slider, TRUE);
        label_text(app->label, "Shapes filled with global (identity) linear gradient.");
        i_draw_gradient(p->ctx, app->gradient, FALSE, TRUE);
        break;
    case 5:
        cell_enabled(app->slider, TRUE);
        label_text(app->label, "Lines with global (identity) linear gradient.");
        i_draw_lines_gradient(p->ctx, app->gradient);
        break;
    case 6:
        cell_enabled(app->slider, TRUE);
        label_text(app->label, "Shapes filled with local (transformed) gradient.");
        i_draw_local_gradient(p->ctx, app->gradient);
        break;
    case 7:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Gradient wrap modes.");
        i_draw_wrap_gradient(p->ctx);
        break;
    case 8:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Single line text with alignment and transforms");
        i_text_single(p->ctx);
        break;
    case 9:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Text with newline '\\n' character and internal alignment");
        i_text_newline(p->ctx);
        break;
    case 10:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Text block in a constrained width area");
        i_text_block(p->ctx);
        break;
    case 11:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Artistic text filled and stroke");
        i_text_art(p->ctx);
        break;
    case 12:
        cell_enabled(app->slider, FALSE);
        label_text(app->label, "Drawing images with alignment");
        i_image(p->ctx);
        break;
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    app->option = p->index;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnSlider(App *app, Event *e)
{
    const EvSlider *p = event_params(e, EvSlider);
    app->gradient = p->pos;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(1, 3);
    Layout *layout2 = layout_create(4, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_multiline();
    PopUp *popup = popup_create();
    Slider *slider = slider_create();
    View *view = view_create();
    label_text(label1, "Select primitives:");
    label_text(label2, "Gradient angle");
    popup_add_elem(popup, "Lines", NULL);
    popup_add_elem(popup, "Shapes", NULL);
    popup_add_elem(popup, "Gradient-1", NULL);
    popup_add_elem(popup, "Gradient-2", NULL);
    popup_add_elem(popup, "Gradient-3", NULL);
    popup_add_elem(popup, "Gradient-4", NULL);
    popup_add_elem(popup, "Gradient-5", NULL);
    popup_add_elem(popup, "Gradient-6", NULL);
    popup_add_elem(popup, "Text-1", NULL);
    popup_add_elem(popup, "Text-2", NULL);
    popup_add_elem(popup, "Text-3", NULL);
    popup_add_elem(popup, "Text-4", NULL);
    popup_add_elem(popup, "Image", NULL);
    popup_list_height(popup, 6);
    popup_OnSelect(popup, listener(app, i_OnSelect, App));
    slider_OnMoved(slider, listener(app, i_OnSlider, App));
    view_size(view, s2df(600, 400));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    layout_label(layout2, label1, 0, 0);
    layout_popup(layout2, popup, 1, 0);
    layout_label(layout2, label2, 2, 0);
    layout_slider(layout2, slider, 3, 0);
    layout_layout(layout1, layout2, 0, 0);
    layout_label(layout1, label3, 0, 1);
    layout_view(layout1, view, 0, 2);
    layout_margin(layout1, 5);
    layout_hmargin(layout2, 0, 10);
    layout_hmargin(layout2, 1, 10);
    layout_hmargin(layout2, 2, 10);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_halign(layout1, 0, 1, ekJUSTIFY);
    layout_hexpand(layout2, 3);
    panel_layout(panel, layout1);
    app->slider = layout_cell(layout2, 3, 0);
    app->view = view;
    app->label = label3;
    return panel;
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = heap_new0(App);
    Panel *panel = i_panel(app);
    app->window = window_create(ekWINDOW_STD);
    app->gradient = 0;
    app->option = 0;
    window_panel(app->window, panel);
    window_title(app->window, "Drawing primitives");
    window_origin(app->window, v2df(500, 200));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
