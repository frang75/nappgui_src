/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui_gtk.inl
 *
 */

/* Operating system native gui */

#include "osgui_gtk.ixx"

__EXTERN_C

extern PangoLayout *kPANGO_LAYOUT;

extern real32_t kPANGO_FROM_PIXELS;

const char_t *_osgui_register_icon(const Image *image);

void _osgui_register_entry(GtkBorder *padding);

void _osgui_ns_resize_cursor(GtkWidget *widget);

void _osgui_ew_resize_cursor(GtkWidget *widget);

void _osgui_default_cursor(GtkWidget *widget);

void _osgui_pre_initialize(void);

bool_t _osgui_is_pre_initialized(void);

extern const guint kVIRTUAL_KEY[];

extern uint32_t kNUM_VKEYS;

__END_C
