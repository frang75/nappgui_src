/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osimg.cpp
 *
 */

/* GDI Image support */

#include "osimg.inl"
#include <draw2d/image.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__WINDOWS__)
#error This file is only for WINDOWS
#endif

#include <sewer/nowarn.hxx>
#include <gdiplus.h>
#include <sewer/warn.hxx>

struct _osimgdata_t
{
    HBITMAP hbitmap;
    COLORREF hbitmap_background;
    LONG hbitmap_width;
    LONG hbitmap_height;
};

/*---------------------------------------------------------------------------*/

static void i_imgdata_destroy(OSImgData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);

    if ((*data)->hbitmap != NULL)
    {
        BOOL ret = DeleteObject((*data)->hbitmap);
        cassert_unref(ret != 0, ret);
    }

    heap_delete(data, OSImgData);
}

/*---------------------------------------------------------------------------*/

HBITMAP _osimg_hbitmap(const Image *image, COLORREF background)
{
    Gdiplus::Bitmap *bitmap = cast(image_native(image), Gdiplus::Bitmap);
    Gdiplus::Color c;
    HBITMAP hbitmap;
    c.SetFromCOLORREF(background);
    Gdiplus::Status status = bitmap->GetHBITMAP(c, &hbitmap);
    cassert_unref(status == Gdiplus::Ok, status);
    return hbitmap;
}

/*---------------------------------------------------------------------------*/

HBITMAP _osimg_hbitmap_cache(const Image *image, COLORREF background, LONG *width, LONG *height)
{
    OSImgData *data = image_get_data(image, OSImgData);

    if (data == NULL)
    {
        data = heap_new0(OSImgData);
        image_data(cast(image, Image), &data, i_imgdata_destroy, OSImgData);
        data = image_get_data(image, OSImgData);
    }

    cassert_no_null(data);

    if (data->hbitmap != NULL && data->hbitmap_background != background)
    {
        BOOL ret = DeleteObject(data->hbitmap);
        cassert_unref(ret != 0, ret);
        data->hbitmap = NULL;
    }

    if (data->hbitmap == NULL)
    {
        BITMAP bm;
        HBITMAP hbitmap = _osimg_hbitmap(image, background);
        data->hbitmap = hbitmap;
        GetObject(hbitmap, sizeof(bm), &bm);
        data->hbitmap_width = bm.bmWidth;
        data->hbitmap_height = bm.bmHeight;
        data->hbitmap_background = background;
    }

    if (width != NULL)
        *width = data->hbitmap_width;

    if (height != NULL)
        *height = data->hbitmap_height;

    return data->hbitmap;
}

/*---------------------------------------------------------------------------*/

HBITMAP _osimg_hbitmap_transparent(const uint32_t width, const uint32_t height)
{
    HBITMAP hbitmap = NULL;
    uint32_t size = 4 * width * height;
    BYTE *pixel_data = cast(heap_malloc(size, "OSImageTHBitmap"), BYTE);
    Gdiplus::Bitmap *bitmap = NULL;
    uint32_t i;

    for (i = 0; i < size; i += 4)
        *cast(pixel_data + i, uint32_t) = 0x000000FF;

    bitmap = new Gdiplus::Bitmap((INT)width, (INT)height, (INT)width * 4, PixelFormat32bppARGB, pixel_data);
    cassert_no_null(bitmap);
    heap_free(dcast(&pixel_data, byte_t), size, "OSImageTHBitmap");

    {
        Gdiplus::Status status = bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hbitmap);
        cassert_unref(status == Gdiplus::Ok, status);
    }

    delete bitmap;
    return hbitmap;
}

/*---------------------------------------------------------------------------*/

HCURSOR _osimg_hcursor(const Image *image, const uint32_t hot_x, const uint32_t hot_y)
{
    /*
     * HCURSOR direct from Gdiplus::Bitmap ;-)
     * http://csharphelper.com/blog/2017/01/convert-a-bitmap-into-a-cursor-in-c/
     */
    Gdiplus::Bitmap *bitmap = cast(image_native(image), Gdiplus::Bitmap);
    HICON icon = NULL;
    ICONINFO info;
    HCURSOR hcursor = NULL;
    BOOL ret = FALSE;
    Gdiplus::Status st = bitmap->GetHICON(&icon);
    cassert_unref(st == Gdiplus::Ok, st);
    GetIconInfo(icon, &info);
    info.xHotspot = (DWORD)hot_x;
    info.yHotspot = (DWORD)hot_y;
    info.fIcon = FALSE;
    hcursor = CreateIconIndirect(&info);
    ret = DeleteObject(info.hbmColor);
    cassert_unref(ret != 0, ret);
    ret = DeleteObject(info.hbmMask);
    cassert_unref(ret != 0, ret);
    ret = DestroyIcon(icon);
    cassert_unref(ret != 0, ret);
    return hcursor;
}

/*---------------------------------------------------------------------------*/

void _osimg_draw(const Image *image, HDC hdc, const uint32_t frame_index, const real32_t x, const real32_t y, const real32_t width, const real32_t height, const BOOL gray)
{
    Gdiplus::Bitmap *bitmap = cast(image_native(image), Gdiplus::Bitmap);
    Gdiplus::Graphics graphics(hdc);

    if (frame_index != UINT32_MAX)
    {
        Gdiplus::Status status = bitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, (UINT)frame_index);
        cassert_unref(status == Gdiplus::Ok, status);
    }

    if (gray == TRUE)
    {
        /* Gray scale conversion (.4 = Alpha) */
        Gdiplus::ColorMatrix matrix =
            {
                .299f, .299f, .299f, 0, 0,
                .587f, .587f, .587f, 0, 0,
                .114f, .114f, .114f, 0, 0,
                0, 0, 0, .4, 0,
                0, 0, 0, 0, 1};
        Gdiplus::ImageAttributes attr;
        Gdiplus::RectF rect((Gdiplus::REAL)x, (Gdiplus::REAL)y, (Gdiplus::REAL)width, (Gdiplus::REAL)height);
        attr.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
        graphics.DrawImage(bitmap, rect, (Gdiplus::REAL)0, (Gdiplus::REAL)0, (Gdiplus::REAL)width, (Gdiplus::REAL)height, Gdiplus::UnitPixel, &attr);
    }
    else
    {
        graphics.DrawImage(bitmap, (Gdiplus::REAL)x, (Gdiplus::REAL)y, (Gdiplus::REAL)width, (Gdiplus::REAL)height);
    }
}
