/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscombo.inl
 *
 */

/* Operating System native combo box */

#include "osgui_win.ixx"

__EXTERN_C

void _oscombo_detach_and_destroy(OSCombo **combo, OSPanel *panel);

void _oscombo_command(OSCombo *combo, WPARAM wParam);

HWND _oscombo_focus(OSCombo *combo);

void _oscombo_elem(HWND hwnd, OSImgList *imglist, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image);

void _oscombo_set_list_height(HWND hwnd, HWND combo_hwnd, const uint32_t image_height, uint32_t num_elems);

__END_C

