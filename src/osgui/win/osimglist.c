/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osimglist.c
 *
 */

/* OSImage-HIMAGELIST binding */

#include "osimglist.inl"
#include "osgui_win.inl"
#include "osimg.inl"
#include <draw2d/image.h>
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/heap.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#include <sewer/nowarn.hxx>
#include <Commctrl.h>
#include <sewer/warn.hxx>

typedef struct _imgdpi_t ImgDpi;

struct _imgdpi_t
{
    uint32_t dpi;
    bool_t invalid;
    HIMAGELIST hlist;
};

struct _osimglist_t
{
    uint32_t img_width;
    uint32_t img_height;
    ArrPt(Image) *images;
    ArrSt(ImgDpi) *hdpi;
};

DeclSt(ImgDpi);

/*---------------------------------------------------------------------------*/

OSImgList *_osimglist_create(const uint32_t height)
{
    OSImgList *imglist = heap_new(OSImgList);
    imglist->img_width = UINT32_MAX;
    imglist->img_height = height;
    imglist->images = arrpt_create(Image);
    imglist->hdpi = arrst_create(ImgDpi);
    return imglist;
}

/*---------------------------------------------------------------------------*/

static void i_remove_imgdpi(ImgDpi *dpi)
{
    BOOL ok;
    cassert_no_null(dpi);
    ok = ImageList_Destroy(dpi->hlist);
    cassert_unref(ok != 0, ok);
}

/*---------------------------------------------------------------------------*/

void _osimglist_destroy(OSImgList **imglist)
{
    cassert_no_null(imglist);
    cassert_no_null(*imglist);
    arrpt_destroy(&(*imglist)->images, image_destroy, Image);
    arrst_destroy(&(*imglist)->hdpi, i_remove_imgdpi, ImgDpi);
    heap_delete(imglist, OSImgList);
}

/*---------------------------------------------------------------------------*/

int _osimglist_find(OSImgList *imglist, const Image *image)
{
    uint32_t index;
    cassert_no_null(imglist);
    cassert_no_null(image);
    index = arrpt_find(imglist->images, image, Image);
    if (index == UINT32_MAX)
        return -1;
    else
        return (int)(index + 1);
}

/*---------------------------------------------------------------------------*/

static void i_invalidate_all(OSImgList *imglist)
{
    cassert_no_null(imglist);
    arrst_foreach(dpi, imglist->hdpi, ImgDpi)
        dpi->invalid = TRUE;
    arrst_end()
}

/*---------------------------------------------------------------------------*/

int _osimglist_add(OSImgList *imglist, const Image *image)
{
    Image *img_copy = NULL;
    cassert_no_null(imglist);
    cassert_no_null(image);
    cassert(arrpt_find(imglist->images, image, Image) == UINT32_MAX);

    if (imglist->img_width == UINT32_MAX)
    {
        Image *scaled_image = image_scale(image, UINT32_MAX, imglist->img_height);
        imglist->img_width = image_width(scaled_image);
        cassert(imglist->img_width != UINT32_MAX);
        image_destroy(&scaled_image);
    }

    img_copy = image_copy(image);
    arrpt_append(imglist->images, img_copy, Image);
    i_invalidate_all(imglist);
    return (int)arrpt_size(imglist->images, Image);
}

/*---------------------------------------------------------------------------*/

void _osimglist_replace(OSImgList *imglist, const int index, const Image *image)
{
    Image **limage = NULL;
    cassert_no_null(imglist);
    cassert_no_null(image);
    cassert(index > 0);
    cassert(arrpt_find(imglist->images, image, Image) == UINT32_MAX);
    limage = arrpt_all(imglist->images, Image) + (uint32_t)(index - 1);
    image_destroy(limage);
    *limage = image_copy(image);
    i_invalidate_all(imglist);
}

/*---------------------------------------------------------------------------*/

void _osimglist_delete(OSImgList *imglist, const int index)
{
    cassert_no_null(imglist);
    cassert(index > 0);
    arrpt_delete(imglist->images, (uint32_t)(index - 1), image_destroy, Image);
    i_invalidate_all(imglist);
}

/*---------------------------------------------------------------------------*/

uint32_t _osimglist_num_elems(const OSImgList *imglist)
{
    uint32_t num_images;
    cassert_no_null(imglist);
    num_images = arrpt_size(imglist->images, Image);
    if (num_images > 0)
        return num_images + 1;
    else
        return 0;
}

/*---------------------------------------------------------------------------*/

static void i_populate_hlist(OSImgList *imglist, ImgDpi *dpi)
{
    HBITMAP transparent = NULL;
    int img_index = -1;
    BOOL ok = FALSE;
    real32_t scale = (real32_t)dpi->dpi / (real32_t)USER_DEFAULT_SCREEN_DPI;
    uint32_t pxwidth = (uint32_t)bmath_roundf((real32_t)imglist->img_width * scale);
    uint32_t pxheight = (uint32_t)bmath_roundf((real32_t)imglist->img_height * scale);

    ok = ImageList_RemoveAll(dpi->hlist);
    cassert_unref(ok != 0, ok);

    transparent = _osimg_hbitmap_transparent(pxwidth, pxheight);
    img_index = ImageList_Add(dpi->hlist, transparent, NULL);
    cassert_unref(img_index == 0, img_index);
    ok = DeleteObject(transparent);
    cassert_unref(ok != 0, ok);

    arrpt_foreach(image, imglist->images, Image)
        Image *scaled_image = image_scale(image, pxwidth, pxheight);
        HBITMAP bitmap = _osimg_hbitmap(scaled_image, 0);
        int index = ImageList_Add(dpi->hlist, bitmap, NULL);
        cassert_unref(index > 0, index);
        ok = DeleteObject(bitmap);
        cassert_unref(ok != 0, ok);
        image_destroy(&scaled_image);
    arrpt_end()

    dpi->invalid = FALSE;
}

/*---------------------------------------------------------------------------*/

HIMAGELIST _osimglist_hlist(OSImgList *imglist, const uint32_t dpi)
{
    cassert_no_null(imglist);
    cassert(imglist->img_width != UINT32_MAX);

    arrst_foreach(entry, imglist->hdpi, ImgDpi)
        if (entry->dpi == dpi)
        {
            if (entry->invalid == TRUE)
                i_populate_hlist(imglist, entry);
            return entry->hlist;
        }
    arrst_end()

    {
        ImgDpi *entry = arrst_new(imglist->hdpi, ImgDpi);
        real32_t scale = (real32_t)dpi / (real32_t)USER_DEFAULT_SCREEN_DPI;
        uint32_t pxwidth = (uint32_t)bmath_roundf((real32_t)imglist->img_width * scale);
        uint32_t pxheight = (uint32_t)bmath_roundf((real32_t)imglist->img_height * scale);
        entry->dpi = dpi;
        entry->invalid = TRUE;
        entry->hlist = ImageList_Create((int)pxwidth, (int)pxheight, ILC_COLOR32 /*| ILC_MASK*/, 0, 4);
        i_populate_hlist(imglist, entry);
        return entry->hlist;
    }
}

/*---------------------------------------------------------------------------*/

uint32_t _osimglist_width(const OSImgList *imglist)
{
    cassert_no_null(imglist);
    return imglist->img_width;
}

/*---------------------------------------------------------------------------*/

uint32_t _osimglist_height(const OSImgList *imglist)
{
    cassert_no_null(imglist);
    cassert(imglist->img_height != UINT32_MAX);
    return imglist->img_height;
}

/*---------------------------------------------------------------------------*/

int _osimglist_index(OSImgList *imglist, HWND hwnd, const gui_type_t type, const Image *image)
{
    int index = 0;

    if (image != NULL)
    {
        /* Image exits in list --> rehuse */
        index = _osimglist_find(imglist, image);

        /* Check for unused image --> replace */
        if (index == -1)
        {
            uint32_t num_images = _osimglist_num_elems(imglist);
            if (num_images > 0)
            {
                uint32_t i = 0, num_elems = 0;
                bool_t *exists = heap_new_n0(num_images, bool_t);

                if (type == ekGUI_TYPE_COMBOBOX)
                {
                    num_elems = (uint32_t)SendMessage(hwnd, CB_GETCOUNT, 0, 0);
                }
                else
                {
                    cassert(type == ekGUI_TYPE_TABLIST);
                    num_elems = (uint32_t)SendMessage(hwnd, TCM_GETITEMCOUNT, 0, 0);
                }

                for (i = 0; i < num_elems; ++i)
                {
                    if (type == ekGUI_TYPE_COMBOBOX)
                    {
                        COMBOBOXEXITEM cbbi = {0};
                        LRESULT res;
                        cbbi.iItem = (INT_PTR)i;
                        cbbi.mask = CBEIF_IMAGE;
                        res = SendMessage(hwnd, CBEM_GETITEM, (WPARAM)0, (LPARAM)&cbbi);
                        cassert_unref(res != 0, res);
                        exists[cbbi.iImage] = TRUE;
                    }
                    else
                    {
                        TCITEM tci = {0};
                        LRESULT res;
                        cassert(type == ekGUI_TYPE_TABLIST);
                        tci.mask = CBEIF_IMAGE;
                        res = SendMessage(hwnd, TCM_GETITEM, (WPARAM)i, (LPARAM)&tci);
                        cassert_unref(res != 0, res);
                        exists[tci.iImage] = TRUE;
                    }
                }

                for (i = 1; i < num_images && index == -1; ++i)
                {
                    if (exists[i] == FALSE)
                    {
                        index = (int)i;
                        _osimglist_replace(imglist, index, image);
                    }
                }

                heap_delete_n(&exists, num_images, bool_t);
            }
        }

        /* Add a new image */
        if (index == -1)
            index = _osimglist_add(imglist, image);
    }

    return index;
}
