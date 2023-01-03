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

#include "osgui_gtk.ixx"

__EXTERN_C

void _oscombo_detach_and_destroy(OSCombo **combo, OSPanel *panel);

void _oscombo_set_focus(OSCombo *combo);

void _oscombo_unset_focus(OSCombo *combo);

GtkWidget *_oscombo_focus(OSCombo *combo);

void _oscombo_elem(GtkComboBox *combo, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image, ArrPt(String) *texts, ArrPt(Image) *images, uint32_t *imgwidth, uint32_t *imgheight);

__END_C

