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
#include <core/heap.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#include <sewer/nowarn.hxx>
#include <Commctrl.h>
#include <sewer/warn.hxx>

struct _osimglist_t
{
    HIMAGELIST hlist;
    uint32_t img_width;
    uint32_t img_height;
    ArrPt(Image) *images;
};

/*---------------------------------------------------------------------------*/

OSImgList *_osimglist_create(const uint32_t height)
{
    OSImgList *imglist = heap_new(OSImgList);
    imglist->hlist = NULL;
    imglist->img_width = UINT32_MAX;
    imglist->img_height = height;
    imglist->images = arrpt_create(Image);
    return imglist;
}

/*---------------------------------------------------------------------------*/

void _osimglist_destroy(OSImgList **imglist)
{
    cassert_no_null(imglist);
    cassert_no_null(*imglist);

    if ((*imglist)->hlist != NULL)
    {
        BOOL ok = ImageList_Destroy((*imglist)->hlist);
        cassert_unref(ok != 0, ok);
    }

    arrpt_destroy(&(*imglist)->images, image_destroy, Image);
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

int _osimglist_add(OSImgList *imglist, const Image *image, uint8_t *result)
{
    int index;
    Image *scaled_image = NULL;
    cassert_no_null(imglist);
    cassert_no_null(image);
    cassert_no_null(result);
    cassert(arrpt_find(imglist->images, image, Image) == UINT32_MAX);
    *result = 0;
    scaled_image = image_scale(image, imglist->img_width, imglist->img_height);
    if (imglist->hlist == NULL)
    {
        HBITMAP transparent = NULL;
        int img_index = -1;
        BOOL ok = FALSE;
        cassert(imglist->img_width == UINT32_MAX);
        imglist->img_width = image_width(scaled_image);
        cassert(imglist->img_width != UINT32_MAX);
        imglist->hlist = ImageList_Create((int)imglist->img_width, (int)imglist->img_height, ILC_COLOR32 /*| ILC_MASK*/, 0, 4);
        transparent = _osimg_hbitmap_transparent(imglist->img_width, imglist->img_height);
        img_index = ImageList_Add(imglist->hlist, transparent, NULL);
        cassert_unref(img_index == 0, img_index);
        ok = DeleteObject(transparent);
        cassert_unref(ok != 0, ok);
        *result = HIMAGELIST_CREATED;
    }
    else
    {
        /* HIMAGELIST: All images same size */
        cassert(imglist->img_width != UINT32_MAX);
    }

    {
        Image *img_copy;
        HBITMAP bitmap;
        BOOL ok = FALSE;
        img_copy = image_copy(image);
        arrpt_append(imglist->images, img_copy, Image);
        bitmap = _osimg_hbitmap(scaled_image, 0);
        index = ImageList_Add(imglist->hlist, bitmap, NULL);
        cassert(index > 0);
        cassert((uint32_t)index == arrpt_size(imglist->images, Image));
        ok = DeleteObject(bitmap);
        cassert_unref(ok != 0, ok);
    }

    image_destroy(&scaled_image);
    return index;
}

/*---------------------------------------------------------------------------*/

void _osimglist_replace(OSImgList *imglist, const int index, const Image *image)
{
    Image **limage = NULL;
    Image *scaled_image = NULL;
    HBITMAP bitmap = NULL;
    BOOL ok = FALSE;
    cassert_no_null(imglist);
    cassert_no_null(imglist->hlist);
    cassert_no_null(image);
    cassert(index > 0);
    cassert(arrpt_find(imglist->images, image, Image) == UINT32_MAX);
    limage = arrpt_all(imglist->images, Image) + (uint32_t)(index - 1);
    image_destroy(limage);
    *limage = image_copy(image);
    cassert(imglist->img_width != UINT32_MAX);
    cassert(imglist->img_height != UINT32_MAX);
    scaled_image = image_scale(image, imglist->img_width, imglist->img_height);
    bitmap = _osimg_hbitmap(image, 0);
    ok = ImageList_Replace(imglist->hlist, index, bitmap, NULL);
    cassert(ok != 0);
    ok = DeleteObject(bitmap);
    cassert_unref(ok != 0, ok);
    image_destroy(&scaled_image);
}

/*---------------------------------------------------------------------------*/

void _osimglist_delete(OSImgList *imglist, const int index, uint8_t *result)
{
    BOOL ok;
    cassert_no_null(imglist);
    cassert_no_null(imglist->hlist);
    cassert(index > 0);
    cassert_no_null(result);
    *result = 0;
    arrpt_delete(imglist->images, (uint32_t)(index - 1), image_destroy, Image);
    ok = ImageList_Remove(imglist->hlist, index);
    cassert_unref(ok != 0, ok);
    if (ImageList_GetImageCount(imglist->hlist) == 1)
    {
        cassert(arrpt_size(imglist->images, Image) == 0);
        ok = ImageList_Destroy(imglist->hlist);
        cassert_unref(ok != 0, ok);
        imglist->hlist = NULL;
        *result = HIMAGELIST_DELETED;
    }
    else
    {
        *result = HIMAGELIST_REORDER;
    }
}

/*---------------------------------------------------------------------------*/

uint32_t _osimglist_num_elems(const OSImgList *imglist)
{
    cassert_no_null(imglist);
    if (imglist->hlist != NULL)
    {
        uint32_t num_elems = (uint32_t)ImageList_GetImageCount(imglist->hlist);
        cassert(num_elems == arrpt_size(imglist->images, Image) + 1);
        return num_elems;
    }
    else
    {
        return 0;
    }
}

/*---------------------------------------------------------------------------*/

HIMAGELIST _osimglist_hlist(const OSImgList *imglist)
{
    cassert_no_null(imglist);
    return imglist->hlist;
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
        {
            uint8_t result = 0;
            index = _osimglist_add(imglist, image, &result);
            if (result == HIMAGELIST_CREATED)
            {
                HIMAGELIST hlist = _osimglist_hlist(imglist);
                if (type == ekGUI_TYPE_COMBOBOX)
                {
                    HIMAGELIST previous = (HIMAGELIST)SendMessage(hwnd, CBEM_SETIMAGELIST, 0, (LPARAM)hlist);
                    cassert_unref(previous == NULL, previous);
                }
                else
                {
                    HIMAGELIST previous = (HIMAGELIST)SendMessage(hwnd, TCM_SETIMAGELIST, 0, (LPARAM)hlist);
                    cassert(type == ekGUI_TYPE_TABLIST);
                    cassert_unref(previous == NULL, previous);
                }
            }
        }
    }

    return index;
}
