/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osimage.c
 *
 */

/* Images */

#include "image.inl"
#include "osimage.inl"
#include "dctx.inl"
#include "bmem.h"
#include "buffer.h"
#include "cassert.h"
#include "heap.h"
#include "pixbuf.h"
#include "ptr.h"
#include "stream.h"
#include "unicode.h"
#include "draw2d_gtk.ixx"
#include <gdk/gdk.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif


/*---------------------------------------------------------------------------*/

void osimage_alloc_globals(void)
{
}

/*---------------------------------------------------------------------------*/

void osimage_dealloc_globals(void)
{
}

/*---------------------------------------------------------------------------*/

static void i_destroy_pixbuf(guchar *pixels, gpointer data)
{
    uint32_t size = (uint32_t)(intptr_t)data;
    heap_delete_n(&pixels, size, guchar);
}

/*---------------------------------------------------------------------------*/

OSImage *osimage_create_from_pixels(const uint32_t width, const uint32_t height, const pixformat_t format, const byte_t *pixel_data)
{
    GdkPixbuf *bitmap = NULL;
    GdkColorspace colorspace = GDK_COLORSPACE_RGB;
    gboolean has_alpha = FALSE;
    int bits_per_sample = 8;
    int rowstride = 0;
    int offset = 0;
    uint32_t size = 0;
    guchar *data = NULL;

    switch(format) {
    case ekRGBA32:
        colorspace = GDK_COLORSPACE_RGB;
        has_alpha = TRUE;
        rowstride = width * 4;
        size = width * height * 4;
        data = heap_new_n(size, guchar);
        bmem_copy(data, pixel_data, size);
        break;

    case ekRGB24:
    {
        register uint32_t i, j;
        guchar *dest = NULL;
        colorspace = GDK_COLORSPACE_RGB;
        has_alpha = FALSE;
        rowstride = width * 3;
        offset = (rowstride % 4) == 0 ? 0 : 4 - (rowstride % 4);
        rowstride += offset;
        size = rowstride * height;
        data = heap_new_n(size, guchar);
        dest = data;
        bmem_copy(data, pixel_data, size);
        for (j = 0; j < height; ++j)
        {
            for (i = 0; i < width; ++i, dest += 3)
            {
                dest[0] = pixel_data[0];
                dest[1] = pixel_data[1];
                dest[2] = pixel_data[2];
                pixel_data += 3;
            }

            dest += offset;
        }

        break;
    }

    case ekGRAY8:
    {
        register uint32_t i, j;
        guchar *dest = NULL;
        colorspace = GDK_COLORSPACE_RGB;
        has_alpha = FALSE;
        rowstride = width * 3;
        offset = (rowstride % 4) == 0 ? 0 : 4 - (rowstride % 4);
        rowstride += offset;
        size = rowstride * height;
        data = heap_new_n(size, guchar);
        dest = data;
        for (j = 0; j < height; ++j)
        {
            for (i = 0; i < width; ++i, dest += 3)
            {
                dest[0] = *pixel_data;
                dest[1] = *pixel_data;
                dest[2] = *pixel_data;
                pixel_data += 1;
            }

            dest += offset;
        }
        break;
    }

    cassert_default();
    }

    bitmap = gdk_pixbuf_new_from_data((const guchar*)data, colorspace, has_alpha, bits_per_sample, (int)width, (int)height, rowstride, i_destroy_pixbuf, (gpointer)(intptr_t)size);
    heap_auditor_add("GdkPixbuf");
    return (OSImage*)bitmap;
}

/*---------------------------------------------------------------------------*/

OSImage *osimage_create_from_data(const byte_t *data, const uint32_t size)
{
    GInputStream *stream = NULL;
    GdkPixbuf *bitmap = NULL;
    gboolean ok;
    cassert_no_null(data);
    cassert(size > 0);
    stream = g_memory_input_stream_new_from_data ((const void*)data, (gssize)size, NULL);
    bitmap = gdk_pixbuf_new_from_stream(stream, NULL, NULL);
    ok = g_input_stream_close(stream, NULL, NULL);
    cassert_unref(ok == TRUE, ok);
    heap_auditor_add("GdkPixbuf");
    return (OSImage*)bitmap;
}

/*---------------------------------------------------------------------------*/

OSImage *osimage_create_from_type(const char_t *file_type)
{
    unref(file_type);
    cassert(FALSE);
    return NULL;
//    WCHAR wextension[64];
//    DWORD dwFileAttributes;
//    uint32_t num_bytes;
//
//    if (strcmp(file_type, ".") == 0)
//    {
//        num_bytes = unicode_convers("\fdsfg", (char_t*)wextension, ekUTF8, ekUTF16, sizeof(wextension));
//        dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
//    }
//    else
//    {
//        num_bytes = unicode_convers(file_type, (char_t*)wextension, ekUTF8, ekUTF16, sizeof(wextension));
//        dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
//    }
//
//    if (num_bytes < sizeof(wextension))
//    {
//        SHFILEINFO psfi;
//        DWORD_PTR ok;
//        Gdiplus::Bitmap *bitmap = NULL;
//        ok = SHGetFileInfo(wextension, dwFileAttributes, &psfi, sizeof(psfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
//        cassert(ok != 0);
//        bitmap = Gdiplus::Bitmap::FromHICON(psfi.hIcon);
//        heap_auditor_add("Gdiplus::Bitmap");
//        return (OSImage*)bitmap;
//    }
//    else
//    {
//        cassert(FALSE);
//        return NULL;
//    }
}

/*---------------------------------------------------------------------------*/

OSImage *osimage_create_scaled(const OSImage *image, const uint32_t new_width, const uint32_t new_height)
{
    GdkPixbuf *bitmap = gdk_pixbuf_scale_simple((const GdkPixbuf*)image, (int)new_width, (int)new_height, GDK_INTERP_BILINEAR);
    heap_auditor_add("GdkPixbuf");
    return (OSImage*)bitmap;
}

/*---------------------------------------------------------------------------*/

//static cairo_status_t i_write_png(void *stm, const unsigned char *data, unsigned int size)
//{
//    stm_write((Stream*)stm, (const byte_t*)data, (const uint32_t)size);
//    return CAIRO_STATUS_SUCCESS;
//}

/*---------------------------------------------------------------------------*/

//static void i_read_png(png_structp png, png_bytep data, png_size_t size)
//{
//    Stream *stm = (Stream*)png_get_io_ptr(png);
//    stm_read(stm, (byte_t*)data, (const uint32_t) size);
//}

/*---------------------------------------------------------------------------*/

//static void i_convert_png(Stream *stm, const pixformat_t format)
//{
//    png_structp png = NULL;
//    png_infop info = NULL;
//
//    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//    cassert_no_null(png);
//
//    png_set_read_fn(png, stm, i_read_png);
//    //png_set_sig_bytes(png, 8);
//
//    info = png_create_info_struct(png);
//    cassert_no_null(info);
//
//    png_read_info(png, info);
//
//    png_uint_32 w = 0, h = 0, ret;
//    int bitd;
//    int color;
//    ret = png_get_IHDR(png, info, &w, &h, &bitd, &color, NULL, NULL, NULL);
//    cassert(ret == 1);
//    cassert(bitd == 8);
//    cassert(color == PNG_COLOR_TYPE_RGB || color == PNG_COLOR_TYPE_RGB_ALPHA);
//    png_destroy_read_struct(&png, &info, NULL);
//
//    // TODO
//    cassert(FALSE);
//}

/*---------------------------------------------------------------------------*/

OSImage *osimage_from_context(DCtx **ctx)
{
    gint w, h;
    GdkPixbuf *bitmap = NULL;
    cassert_no_null(ctx);
    cassert_no_null(*ctx);

    w = cairo_image_surface_get_width((*ctx)->surface);
    h = cairo_image_surface_get_height((*ctx)->surface);
    bitmap = gdk_pixbuf_get_from_surface((*ctx)->surface, 0, 0, w, h);

    if ((*ctx)->format == ekGRAY8)
    {
        guchar *pixels = gdk_pixbuf_get_pixels(bitmap);
        gboolean alpha = gdk_pixbuf_get_has_alpha(bitmap);
        register uint32_t i, j, offset = alpha ? 4 : 3;
        int stride = gdk_pixbuf_get_rowstride(bitmap) - (offset * w);
        for (j = 0; j < h; ++j)
        {
            for (i = 0; i < w; ++i)
            {
                guchar gray = (guchar)((77 * pixels[0] + 148 * pixels[1] + 30 * pixels[2]) / 255);
                pixels[0] = gray;
                pixels[1] = gray;
                pixels[2] = gray;
                pixels += offset;
            }

            pixels += stride;
        }
    }

    heap_auditor_add("GdkPixbuf");
    dctx_destroy(ctx);
    return (OSImage*)bitmap;
}

/*---------------------------------------------------------------------------*/

void osimage_destroy(OSImage **image)
{
    cassert_no_null(image);
    cassert_no_null(*image);
    g_object_unref((gpointer)*image);
    heap_auditor_delete("GdkPixbuf");
    *image = NULL;
}

/*---------------------------------------------------------------------------*/

static bool_t i_gray_image(const byte_t *data, const uint32_t width, const uint32_t height, const uint32_t bpp, const uint32_t stride)
{
    uint32_t i = 0, j = 0;
    for (j = 0; j < height; ++j)
    {
        for (i = 0; i < width; ++i)
        {
            if (data[0] != data[1] || data[0] != data[2])
                return FALSE;

            data += bpp;
        }

        data += stride;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_has_alpha(const byte_t *data, const uint32_t width, const uint32_t height, const uint32_t bpp, const uint32_t stride)
{
    uint32_t i = 0, j = 0;
    cassert(bpp == 4);
    cassert(stride == 0);
    for (j = 0; j < height; ++j)
    {
        for (i = 0; i < width; ++i)
        {
            if (data[3] != 255)
                return TRUE;

            data += bpp;
        }

        data += stride;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static Pixbuf *i_bitmap_pixels(const byte_t *data, const uint32_t width, const uint32_t height, const uint32_t bpp, const uint32_t stride, const pixformat_t format)
{
    Pixbuf *pixels = pixbuf_create(width, height, format);
    byte_t *pdata = pixbuf_data(pixels);
    register uint32_t n = width * height, i = 0, j = 0;
    if (format == ekRGBA32)
    {
        cassert(bpp == 4);
        cassert(stride == 0);
        bmem_copy(pdata, data, n * 4);
    }
    else if (format == ekRGB24)
    {
        cassert(bpp == 3 || bpp == 4);
        for (j = 0; j < height; ++j)
        {
            for (i = 0; i < width; ++i)
            {
                pdata[0] = data[0];
                pdata[1] = data[1];
                pdata[2] = data[2];
                pdata += 3;
                data += bpp;
            }

            data += stride;
        }
    }
    else
    {
        cassert(bpp == 1 || bpp == 3 || bpp == 4);
        cassert(format == ekGRAY8);
        for (j = 0; j < height; ++j)
        {
            for (i = 0; i < width; ++i)
            {
                pdata[0] = data[0];
                pdata += 1;
                data += bpp;
            }

            data += stride;
        }
    }

    return pixels;
}

/*---------------------------------------------------------------------------*/

void osimage_info(const OSImage *image, uint32_t *width, uint32_t *height, pixformat_t *format, Pixbuf **pixels)
{
    cassert_no_null(image);

    if (width != NULL)
        *width = (uint32_t)gdk_pixbuf_get_width((GdkPixbuf*)image);

    if (height != NULL)
        *height = (uint32_t)gdk_pixbuf_get_height((GdkPixbuf*)image);

    if (format != NULL || pixels != NULL)
    {
        pixformat_t lformat = ENUM_MAX(pixformat_t);
        const byte_t *buffer = (const byte_t*)gdk_pixbuf_get_pixels((GdkPixbuf*)image);
        uint32_t w = gdk_pixbuf_get_width((GdkPixbuf*)image);
        uint32_t h = gdk_pixbuf_get_height((GdkPixbuf*)image);
        uint32_t bits_per_pixel = gdk_pixbuf_get_n_channels((GdkPixbuf*)image) * 8;
        uint32_t offset = bits_per_pixel / 8;
        uint32_t stride = gdk_pixbuf_get_rowstride((GdkPixbuf*)image) - (offset * w);

        cassert(bits_per_pixel == 24 || bits_per_pixel == 32);
        if (bits_per_pixel == 24 && gdk_pixbuf_get_has_alpha((GdkPixbuf*)image) == TRUE)
        {
            bits_per_pixel = 32;
            offset = 4;
        }

        if (bits_per_pixel == 8)
        {
            lformat = ekGRAY8;
        }
        else if (bits_per_pixel == 24)
        {
            if (i_gray_image(buffer, w, h, 3, stride) == TRUE)
                lformat = ekGRAY8;
            else
                lformat = ekRGB24;
        }
        else if (bits_per_pixel == 32)
        {
            if (i_has_alpha(buffer, w, h, 4, stride) == TRUE)
                lformat = ekRGBA32;
            else if (i_gray_image(buffer, w, h, 4, stride) == TRUE)
                lformat = ekGRAY8;
            else
                lformat = ekRGB24;
        }

        ptr_assign(format, lformat);

        if (pixels != NULL)
        {
            if (lformat != ENUM_MAX(pixformat_t))
                *pixels = i_bitmap_pixels(buffer, w, h, (uint32_t)(bits_per_pixel / 8), stride, lformat);
            else
                *pixels = NULL;
        }
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_encode(const gchar *data, gsize size, GError **error, gpointer stream)
{
    stm_write((Stream*)stream, (const byte_t*)data, (uint32_t)size);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

bool_t osimage_available_codec(const OSImage *image, const codec_t codec)
{
    unref(image);
    unref(codec);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

void osimage_write(const OSImage *image, const codec_t codec, Stream *stream)
{
    const char *type="";
    gboolean ok;
    switch (codec)
    {
        case ekJPG:
            type = "jpeg";
            break;
        case ekPNG:
            type = "png";
            break;
        case ekBMP:
            type = "bmp";
            break;
        case ekGIF:
            type = "gif";
            break;
        cassert_default();
    }

    ok = gdk_pixbuf_save_to_callback((GdkPixbuf*)image, i_encode, (gpointer)stream, type, NULL, NULL);
    cassert_unref(ok == TRUE, ok);
}

/*---------------------------------------------------------------------------*/

void osimage_frames(const OSImage *image, uint32_t *num_frames, uint32_t *num_loops)
{
    unref(image);
    unref(num_frames);
    unref(num_loops);
    ptr_assign(num_frames, 1);
    //cassert(FALSE);

//    GUID dimension;
//    cassert_no_null(image);
//    cassert_no_null(num_frames);
//    unref(num_loops);
//    ((Gdiplus::Bitmap*)image)->GetFrameDimensionsList(&dimension, 1);
//    *num_frames = (uint32_t)((Gdiplus::Bitmap*)image)->GetFrameCount(&dimension);
}

/*---------------------------------------------------------------------------*/

void osimage_frame(const OSImage *image, const uint32_t frame_index, real32_t *frame_length)
{
    unref(image);
    unref(frame_index);
    unref(frame_length);
    cassert(FALSE);

//    UINT property_size;
//    Gdiplus::PropertyItem *property_item = NULL;
//    cassert_no_null(image);
//    cassert_no_null(frame_length);
//    property_size = ((Gdiplus::Bitmap*)image)->GetPropertyItemSize(PropertyTagFrameDelay);
//    property_item = (Gdiplus::PropertyItem*)heap_malloc(property_size, "ImgImpPropItem");
//    ((Gdiplus::Bitmap*)image)->GetPropertyItem(PropertyTagFrameDelay, property_size, property_item);
//    cassert(property_item->type == PropertyTagTypeLong);
//    *frame_length = 0.1f * (real32_t)(((long*)property_item->value)[frame_index]);
//    heap_free((byte_t**)&property_item, property_size, "ImgImpPropItem");
}

/*---------------------------------------------------------------------------*/

GdkPixbuf *osimage_pixbuf(OSImage *image)
{
    cassert_no_null(image);
    return (GdkPixbuf*)image;
}

