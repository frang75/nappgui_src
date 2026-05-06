/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw2d_win.ixx
 *
 */

/* 2D drawing support */

#ifndef __OSDRAW_WIN_IXX__
#define __OSDRAW_WIN_IXX__

#include "draw2d_gdi.ixx"
#include <sewer/nowarn.hxx>
#include <gdiplus.h>
#if defined(_MSC_VER)
#include <gdipluspixelformats.h>
#else
#include <gdiplus/gdipluspixelformats.h>
#endif
#include <sewer/warn.hxx>

struct _dctx_t
{
    HDC hdc;
    bool_t gdi_mode;
    uint32_t width;
    uint32_t height;
    pixformat_t format;
    color_t line_color;
    color_t text_color;
    color_t fill_color;
    HPEN gdi_pen;
    HBRUSH gdi_sbrush;
    COLORREF background_color;
    Gdiplus::REAL offset_x;
    Gdiplus::REAL offset_y;
    Gdiplus::Pen *pen;
    Gdiplus::Pen *fpen;
    Gdiplus::Pen *current_pen;
    Gdiplus::SolidBrush *tbrush;
    Gdiplus::SolidBrush *sbrush;
    Gdiplus::LinearGradientBrush *lbrush;
    Gdiplus::PathGradientBrush *pbrush;
    Gdiplus::Brush *current_brush;
    Gdiplus::GraphicsPath *path;
    Gdiplus::PointF path_current;
    bool_t path_has_current;
    Gdiplus::Bitmap *bitmap;
    Gdiplus::Graphics *graphics;
    Gdiplus::Matrix *gradient_matrix;
    Gdiplus::Color gradient_colors[18];
    Gdiplus::REAL gradient_stops[18];
    INT gradient_n;
    Gdiplus::REAL gradient_x;
    Gdiplus::REAL gradient_y;
    Gdiplus::REAL gradient_scale;
    Gdiplus::REAL gradient_angle;
    Gdiplus::REAL radial_x0;
    Gdiplus::REAL radial_y0;
    Gdiplus::REAL radial_r0;
    Gdiplus::REAL radial_x1;
    Gdiplus::REAL radial_y1;
    Gdiplus::REAL radial_r1;
    Gdiplus::WrapMode gradient_wrap;
    fillrule_t fill_rule;
    Gdiplus::REAL dash_offset;
    Gdiplus::REAL miter_limit;
    real32_t text_width;
    align_t text_halign;
    align_t text_valign;
    align_t text_intalign;
    ellipsis_t text_ellipsis;
    Font *font;
    Gdiplus::Font *ffont;
    Gdiplus::FontFamily *ffamily;
    INT fstyle;
    Gdiplus::REAL fsize;
    Gdiplus::REAL fintleading;
    align_t image_halign;
    align_t image_valign;
    void *data;
    FPtr_destroy func_destroy_data;
};

#endif
