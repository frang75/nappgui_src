/* Drawing primitives */

#include "res_drawhello.h"
#include <nappgui.h>
#include <gui/view.inl>
#include <gui/drawctrl.inl>

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    View *view;
    Label *label;
    Cell *slider1;
    Cell *slider2;
    Cell *popup_trim;
    Cell *popup_align;
    uint32_t option;
};

/*---------------------------------------------------------------------------*/

static void i_draw_lines(DCtx *ctx)
{
    const V2Df poly1[] = {{10, 190}, {90, 110}, {110, 190}, {190, 110}, {210, 190}, {290, 110}};
    const V2Df poly2[] = {{310, 190}, {390, 110}, {410, 190}, {490, 110}, {510, 190}, {590, 110}};
    const V2Df poly3[] = {{10, 290}, {90, 210}, {110, 290}, {190, 210}, {210, 290}, {290, 210}};
    const real32_t pattern1[] = {5, 5, 10, 5};
    const real32_t pattern2[] = {1, 1};
    const real32_t pattern3[] = {2, 1};
    const real32_t pattern4[] = {1, 2};

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
    const V2Df poly[] = {{40, 0}, {12.36f, 38.04f}, {-32.36f, 23.52f}, {-32.36f, -23.52f}, {12.36f, -38.04f}};
    T2Df matrix;
    draw_rect(ctx, op, 10, 10, 110, 75);
    draw_rndrect(ctx, op, 140, 10, 110, 75, 20);
    draw_circle(ctx, op, 312, 50, 40);
    draw_ellipse(ctx, op, 430, 50, 55, 37);
    t2d_movef(&matrix, origin, 547, 50);
    t2d_rotatef(&matrix, &matrix, -kBMATH_PIf / 10);
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
    const real32_t pattern1[] = {5, 5, 10, 5};
    const real32_t pattern2[] = {1, 1};
    const real32_t pattern3[] = {2, 1};
    const real32_t pattern4[] = {1, 2};

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
    draw_arc(ctx, 100, 280, 60, 0, -kBMATH_PIf / 2);
    draw_arc(ctx, 250, 280, 60, kBMATH_PIf, kBMATH_PIf / 2);
    draw_arc(ctx, 300, 220, 60, kBMATH_PIf / 2, -kBMATH_PIf / 2);
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
    t2d_rotatef(&matrix, &matrix, -kBMATH_PIf / 10);
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

static void i_draw_text_rect(DCtx *ctx, const real32_t x, const real32_t y, const real32_t w, const real32_t h, const align_t halign, const align_t valign)
{
    real32_t rx = x, ry = y;
    switch (halign)
    {
    case ekLEFT:
    case ekJUSTIFY:
        break;
    case ekCENTER:
        rx -= w / 2;
        break;
    case ekRIGHT:
        rx -= w;
        break;
    default:
        cassert_default(halign);
    }

    switch (valign)
    {
    case ekTOP:
    case ekJUSTIFY:
        break;
    case ekCENTER:
        ry -= h / 2;
        break;
    case ekBOTTOM:
        ry -= h;
        break;
    default:
        cassert_default(valign);
    }

    draw_rect(ctx, ekSTROKE, rx, ry, w, h);
}

/*---------------------------------------------------------------------------*/

static void i_draw_text(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y, const real32_t w, const real32_t h, const align_t halign, const align_t valign)
{
    draw_text_align(ctx, halign, valign);

    if (w > 20)
        draw_text(ctx, text, x, y);

    draw_circle(ctx, ekFILL, x, y, 3);
    i_draw_text_rect(ctx, x, y, w, h, halign, valign);
}

/*---------------------------------------------------------------------------*/

static void i_text_single(DCtx *ctx, const real32_t xscale, const real32_t text_width, const uint32_t text_trim, const uint32_t text_align)
{
    Font *bfont = font_system(20, 0);
    Font *font = font_with_xscale(bfont, xscale);
    const char_t *text = "Text 文本 Κείμενο";
    real32_t w, h;
    T2Df matrix;
    draw_font(ctx, font);
    draw_text_extents(ctx, text, -1, &w, &h);
    draw_text_color(ctx, kCOLOR_BLUE);
    draw_line_color(ctx, kCOLOR_RED);
    draw_fill_color(ctx, kCOLOR_RED);
    draw_text_halign(ctx, (align_t)(text_align + 1));

    if (text_trim > 0)
    {
        draw_text_width(ctx, text_width);
        if (text_trim == 1)
            draw_text_trim(ctx, ekELLIPBEGIN);
        else if (text_trim == 2)
            draw_text_trim(ctx, ekELLIPMIDDLE);
        else
            draw_text_trim(ctx, ekELLIPEND);
        w = text_width;
    }
    else
    {
        draw_text_width(ctx, -1);
    }

    i_draw_text(ctx, text, 25, 25, w, h, ekLEFT, ekTOP);
    i_draw_text(ctx, text, 300, 25, w, h, ekCENTER, ekTOP);
    i_draw_text(ctx, text, 575, 25, w, h, ekRIGHT, ekTOP);
    i_draw_text(ctx, text, 25, 100, w, h, ekLEFT, ekCENTER);
    i_draw_text(ctx, text, 300, 100, w, h, ekCENTER, ekCENTER);
    i_draw_text(ctx, text, 575, 100, w, h, ekRIGHT, ekCENTER);
    i_draw_text(ctx, text, 25, 175, w, h, ekLEFT, ekBOTTOM);
    i_draw_text(ctx, text, 300, 175, w, h, ekCENTER, ekBOTTOM);
    i_draw_text(ctx, text, 575, 175, w, h, ekRIGHT, ekBOTTOM);

    t2d_movef(&matrix, kT2D_IDENTf, 25, 200);
    t2d_rotatef(&matrix, &matrix, kBMATH_PIf / 8);
    draw_matrixf(ctx, &matrix);
    i_draw_text(ctx, text, 0, 0, w, h, ekLEFT, ekTOP);

    t2d_movef(&matrix, kT2D_IDENTf, 300, 250);
    t2d_rotatef(&matrix, &matrix, -kBMATH_PIf / 8);
    draw_matrixf(ctx, &matrix);
    i_draw_text(ctx, text, 0, 0, w, h, ekCENTER, ekCENTER);

    t2d_movef(&matrix, kT2D_IDENTf, 25, 325);
    t2d_scalef(&matrix, &matrix, 3, 1);
    draw_matrixf(ctx, &matrix);
    i_draw_text(ctx, text, 0, 0, w, h, ekLEFT, ekTOP);

    t2d_movef(&matrix, kT2D_IDENTf, 575, 200);
    t2d_scalef(&matrix, &matrix, .5f, 1);
    draw_matrixf(ctx, &matrix);
    i_draw_text(ctx, text, 0, 0, w, h, ekRIGHT, ekTOP);

    t2d_movef(&matrix, kT2D_IDENTf, 575, 230);
    t2d_scalef(&matrix, &matrix, .75f, 1);
    draw_matrixf(ctx, &matrix);
    i_draw_text(ctx, text, 0, 0, w, h, ekRIGHT, ekTOP);

    t2d_movef(&matrix, kT2D_IDENTf, 575, 260);
    t2d_scalef(&matrix, &matrix, 1.25f, 1);
    draw_matrixf(ctx, &matrix);
    i_draw_text(ctx, text, 0, 0, w, h, ekRIGHT, ekTOP);

    font_destroy(&bfont);
    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static void i_text_newline(DCtx *ctx, const real32_t xscale, const uint32_t align)
{
    Font *bfont = font_system(20, 0);
    Font *font = font_with_xscale(bfont, xscale);
    const char_t *text = "Text new line\n文字换行\nΓραμμή κειμένου";
    const char_t *stext = "Text single line";
    real32_t w1, h1, w2, h2;
    draw_font(ctx, font);
    draw_text_extents(ctx, text, -1, &w1, &h1);
    draw_text_extents(ctx, stext, -1, &w2, &h2);
    draw_text_color(ctx, kCOLOR_BLUE);
    draw_line_color(ctx, kCOLOR_RED);
    draw_fill_color(ctx, kCOLOR_RED);
    draw_text_width(ctx, -1);
    draw_text_halign(ctx, (align_t)(align + 1));
    i_draw_text(ctx, text, 25, 25, w1, h1, ekLEFT, ekTOP);
    i_draw_text(ctx, text, 300, 25, w1, h1, ekCENTER, ekTOP);
    i_draw_text(ctx, text, 575, 25, w1, h1, ekRIGHT, ekTOP);
    i_draw_text(ctx, text, 25, 175, w1, h1, ekLEFT, ekCENTER);
    i_draw_text(ctx, text, 300, 175, w1, h1, ekCENTER, ekCENTER);
    i_draw_text(ctx, text, 575, 175, w1, h1, ekRIGHT, ekCENTER);
    i_draw_text(ctx, text, 25, 325, w1, h1, ekLEFT, ekBOTTOM);
    i_draw_text(ctx, text, 300, 325, w1, h1, ekCENTER, ekBOTTOM);
    i_draw_text(ctx, text, 575, 325, w1, h1, ekRIGHT, ekBOTTOM);
    i_draw_text(ctx, stext, 25, 350, w2, h2, ekLEFT, ekTOP);
    i_draw_text(ctx, stext, 300, 350, w2, h2, ekCENTER, ekTOP);
    i_draw_text(ctx, stext, 575, 350, w2, h2, ekRIGHT, ekTOP);
    font_destroy(&bfont);
    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static void i_draw_text_block(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y, const real32_t max_width, const real32_t w, const real32_t h, const align_t halign, const align_t valign)
{
    real32_t dash[2] = {1, 1};
    i_draw_text(ctx, text, x, y, max_width, h, halign, valign);
    draw_line_dash(ctx, dash, 2);
    i_draw_text_rect(ctx, x, y, w, h, halign, valign);
    draw_line_dash(ctx, NULL, 0);
}

/*---------------------------------------------------------------------------*/

static void i_text_block(DCtx *ctx, const real32_t xscale, const real32_t text_width, const uint32_t align)
{
    const char_t *text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";
    Font *bfont = font_system(font_regular_size(), 0);
    Font *font = font_with_xscale(bfont, xscale);
    real32_t w, h;
    draw_font(ctx, font);
    draw_text_color(ctx, kCOLOR_BLUE);
    draw_line_color(ctx, kCOLOR_RED);
    draw_fill_color(ctx, kCOLOR_RED);
    draw_text_halign(ctx, (align_t)(align + 1));
    draw_text_width(ctx, text_width);
    draw_text_extents(ctx, text, text_width, &w, &h);
    i_draw_text_block(ctx, text, 25, 25, text_width, w, h, ekLEFT, ekTOP);
    i_draw_text_block(ctx, text, 400, 25, text_width, w, h, ekCENTER, ekTOP);
    i_draw_text_block(ctx, text, 500, 200, text_width, w, h, ekRIGHT, ekTOP);
    font_destroy(&bfont);
    font_destroy(&font);
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

static void i_draw_text_raster(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y, const real32_t w, const real32_t h, const align_t halign)
{
    draw_text_halign(ctx, halign);
    drawctrl_text(ctx, text, (int32_t)x, (int32_t)y, ekCTRL_STATE_NORMAL);
    draw_circle(ctx, ekFILL, x, y, 3);
    draw_rect(ctx, ekSTROKE, x, y, w, h);
}

/*---------------------------------------------------------------------------*/

static void i_text_raster(DCtx *ctx, const real32_t xscale, const real32_t text_width, const uint32_t text_trim, const uint32_t align)
{
    Font *bfont = font_system(20, 0);
    Font *font = font_with_xscale(bfont, xscale);
    align_t halign = (align_t)(align + 1);

    draw_font(ctx, font);
    draw_line_color(ctx, kCOLOR_RED);
    draw_fill_color(ctx, kCOLOR_RED);

    /* Single line with text width and trim (ellipsis) */
    {
        const char_t *text = "This is a text line";
        real32_t w, h;
        draw_text_color(ctx, kCOLOR_DEFAULT);
        font_extents(font, text, -1, &w, &h);

        if (text_trim > 0)
        {
            draw_text_width(ctx, text_width);
            if (text_trim == 1)
                draw_text_trim(ctx, ekELLIPBEGIN);
            else if (text_trim == 2)
                draw_text_trim(ctx, ekELLIPMIDDLE);
            else
                draw_text_trim(ctx, ekELLIPEND);
            w = text_width;
        }
        else
        {
            draw_text_width(ctx, -1);
        }

        i_draw_text_raster(ctx, text, 25, 25, w, h, halign);
    }

    /* Text block with newlines */
    {
        const char_t *text = "Text new line\n文字换行\nΓραμμή κειμένου";
        real32_t w, h;
        draw_text_width(ctx, -1);
        draw_text_color(ctx, kCOLOR_BLUE);
        font_extents(font, text, -1, &w, &h);
        i_draw_text_raster(ctx, text, 200, 25, w, h, halign);
    }

    /* Text block with fixed width */
    {
        const char_t *text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";
        real32_t w, h, tw = 200 + text_width;
        draw_text_width(ctx, tw);
        draw_text_trim(ctx, ekELLIPMLINE);
        font_extents(font, text, tw, &w, &h);
        i_draw_text_raster(ctx, text, 25, 125, tw, h, halign);
    }

    font_destroy(&bfont);
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
    Slider *slider1 = cell_slider(app->slider1);
    Slider *slider2 = cell_slider(app->slider2);
    PopUp *popup1 = cell_popup(app->popup_trim);
    PopUp *popup2 = cell_popup(app->popup_align);
    real32_t slider1_pos = slider_get_value(slider1);
    real32_t slider2_pos = slider_get_value(slider2);
    uint32_t text_trim = popup_get_selected(popup1);
    uint32_t text_align = popup_get_selected(popup2);

    draw_clear(p->ctx, color_rgb(200, 200, 200));

    switch (app->option)
    {
    case 0:
        i_draw_lines(p->ctx);
        break;
    case 1:
        draw_fill_color(p->ctx, kCOLOR_BLUE);
        i_draw_shapes(p->ctx, FALSE);
        break;
    case 2:
        i_draw_gradient(p->ctx, slider1_pos, TRUE, FALSE);
        break;
    case 3:
        i_draw_gradient(p->ctx, slider1_pos, TRUE, TRUE);
        break;
    case 4:
        i_draw_gradient(p->ctx, slider1_pos, FALSE, TRUE);
        break;
    case 5:
        i_draw_lines_gradient(p->ctx, slider1_pos);
        break;
    case 6:
        i_draw_local_gradient(p->ctx, slider1_pos);
        break;
    case 7:
        i_draw_wrap_gradient(p->ctx);
        break;
    case 8:
        i_text_single(p->ctx, slider1_pos + .5f, 100 + 200 * (-.5f + slider2_pos), text_trim, text_align);
        break;
    case 9:
        i_text_newline(p->ctx, slider1_pos + .5f, text_align);
        break;
    case 10:
        i_text_block(p->ctx, slider1_pos + .5f, 200 + 200 * (-.5f + slider2_pos), text_align);
        break;
    case 11:
        i_text_art(p->ctx);
        break;
    case 12:
        i_text_raster(p->ctx, slider1_pos + .5f, 100 + 200 * (-.5f + slider2_pos), text_trim, text_align);
        break;
    case 13:
        i_image(p->ctx);
        break;
    default:
        cassert_default(app->option);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnAcceptFocus(App *app, Event *e)
{
    bool_t *r = event_result(e, bool_t);
    unref(app);
    *r = FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_set_demo(App *app, const uint32_t option)
{
    Slider *slider1 = cell_slider(app->slider1);
    Slider *slider2 = cell_slider(app->slider2);
    cassert_no_null(app);
    app->option = option;
    slider_value(slider1, 0.5f);
    slider_value(slider2, 0.5f);
    cell_enabled(app->slider1, FALSE);
    cell_enabled(app->slider2, FALSE);
    cell_enabled(app->popup_trim, FALSE);
    cell_enabled(app->popup_align, FALSE);

    switch (app->option)
    {
    case 0:
        label_text(app->label, "Different line styles: width, join, cap, dash...");
        break;
    case 1:
        label_text(app->label, "Basic shapes filled and stroke.");
        break;
    case 2:
        cell_enabled(app->slider1, TRUE);
        label_text(app->label, "Global linear gradient.");
        break;
    case 3:
        cell_enabled(app->slider1, TRUE);
        label_text(app->label, "Shapes filled with global (identity) linear gradient (with background).");
        break;
    case 4:
        cell_enabled(app->slider1, TRUE);
        label_text(app->label, "Shapes filled with global (identity) linear gradient.");
        break;
    case 5:
        cell_enabled(app->slider1, TRUE);
        label_text(app->label, "Lines with global (identity) linear gradient.");
        break;
    case 6:
        cell_enabled(app->slider1, TRUE);
        label_text(app->label, "Shapes filled with local (transformed) gradient.");
        break;
    case 7:
        label_text(app->label, "Gradient wrap modes.");
        break;
    case 8:
        label_text(app->label, "Single line text with alignment, transforms and trimming");
        cell_enabled(app->slider1, TRUE);
        cell_enabled(app->slider2, TRUE);
        cell_enabled(app->popup_trim, TRUE);
        cell_enabled(app->popup_align, TRUE);
        break;
    case 9:
        label_text(app->label, "Text with newline '\\n' character and internal alignment");
        cell_enabled(app->slider1, TRUE);
        cell_enabled(app->popup_align, TRUE);
        break;
    case 10:
        label_text(app->label, "Text block in a constrained width area");
        cell_enabled(app->slider1, TRUE);
        cell_enabled(app->slider2, TRUE);
        cell_enabled(app->popup_align, TRUE);
        break;
    case 11:
        label_text(app->label, "Artistic text filled and stroke");
        break;
    case 12:
        label_text(app->label, "Rasterized text with alignment and trimming");
        cell_enabled(app->slider1, TRUE);
        cell_enabled(app->slider2, TRUE);
        cell_enabled(app->popup_trim, TRUE);
        cell_enabled(app->popup_align, TRUE);
        break;
    case 13:
        label_text(app->label, "Drawing images with alignment");
        break;
    default:
        cassert_default(app->option);
    }

    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect1(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    i_set_demo(app, p->index);
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect2(App *app, Event *e)
{
    unref(e);
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnSlider(App *app, Event *e)
{
    unref(e);
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(1, 3);
    Layout *layout2 = layout_create(4, 3);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    PopUp *popup1 = popup_create();
    PopUp *popup2 = popup_create();
    PopUp *popup3 = popup_create();
    Slider *slider1 = slider_create();
    Slider *slider2 = slider_create();
    View *view = _view_create(ekVIEW_CONTROL);
    label_text(label1, "Select primitives:");
    label_text(label2, "Gradient/scale:");
    label_text(label3, "Text trim:");
    label_text(label4, "Text width:");
    label_text(label5, "Text halign:");
    label_multiline(label6, TRUE);
    popup_add_elem(popup1, "Lines", NULL);
    popup_add_elem(popup1, "Shapes", NULL);
    popup_add_elem(popup1, "Gradient-1", NULL);
    popup_add_elem(popup1, "Gradient-2", NULL);
    popup_add_elem(popup1, "Gradient-3", NULL);
    popup_add_elem(popup1, "Gradient-4", NULL);
    popup_add_elem(popup1, "Gradient-5", NULL);
    popup_add_elem(popup1, "Gradient-6", NULL);
    popup_add_elem(popup1, "Text-1", NULL);
    popup_add_elem(popup1, "Text-2", NULL);
    popup_add_elem(popup1, "Text-3", NULL);
    popup_add_elem(popup1, "Text-4", NULL);
    popup_add_elem(popup1, "Text-5", NULL);
    popup_add_elem(popup1, "Image", NULL);
    popup_list_height(popup1, 6);
    popup_add_elem(popup2, "No trim", NULL);
    popup_add_elem(popup2, "Ellip begin", NULL);
    popup_add_elem(popup2, "Ellip middle", NULL);
    popup_add_elem(popup2, "Ellip end", NULL);
    popup_add_elem(popup3, "Left", NULL);
    popup_add_elem(popup3, "Center", NULL);
    popup_add_elem(popup3, "Right", NULL);
    popup_OnSelect(popup1, listener(app, i_OnSelect1, App));
    popup_OnSelect(popup2, listener(app, i_OnSelect2, App));
    popup_OnSelect(popup3, listener(app, i_OnSelect2, App));
    slider_OnMoved(slider1, listener(app, i_OnSlider, App));
    slider_OnMoved(slider2, listener(app, i_OnSlider, App));
    view_size(view, s2df(600, 400));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    view_OnAcceptFocus(view, listener(app, i_OnAcceptFocus, App));
    layout_label(layout2, label1, 0, 0);
    layout_label(layout2, label2, 2, 0);
    layout_label(layout2, label3, 0, 1);
    layout_label(layout2, label4, 2, 1);
    layout_label(layout2, label5, 0, 2);
    layout_popup(layout2, popup1, 1, 0);
    layout_popup(layout2, popup2, 1, 1);
    layout_popup(layout2, popup3, 1, 2);
    layout_slider(layout2, slider1, 3, 0);
    layout_slider(layout2, slider2, 3, 1);
    layout_layout(layout1, layout2, 0, 0);
    layout_label(layout1, label6, 0, 1);
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
    app->slider1 = layout_cell(layout2, 3, 0);
    app->slider2 = layout_cell(layout2, 3, 1);
    app->popup_trim = layout_cell(layout2, 1, 1);
    app->popup_align = layout_cell(layout2, 1, 2);
    app->view = view;
    app->label = label6;
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
    i_set_demo(app, 0);
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

#include <osapp/osmain.h>
osmain(i_create, i_destroy, "", App)
