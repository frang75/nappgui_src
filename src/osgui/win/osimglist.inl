/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osimglist.inl
 *
 */

/* OSImage-HIMAGELIST binding */

#include "osgui_win.ixx"

__EXTERN_C

OSImgList *_osimglist_create(const uint32_t height);

void _osimglist_destroy(OSImgList **imglist);

int _osimglist_find(OSImgList *imglist, const Image *image);

int _osimglist_add(OSImgList *imglist, const Image *image);

void _osimglist_replace(OSImgList *imglist, const int index, const Image *image);

void _osimglist_delete(OSImgList *imglist, const int index);

uint32_t _osimglist_num_elems(const OSImgList *imglist);

HIMAGELIST _osimglist_hlist(OSImgList *imglist, const uint32_t dpi);

uint32_t _osimglist_width(const OSImgList *imglist);

uint32_t _osimglist_height(const OSImgList *imglist);

int _osimglist_index(OSImgList *imglist, HWND hwnd, const gui_type_t type, const Image *image);

__END_C
