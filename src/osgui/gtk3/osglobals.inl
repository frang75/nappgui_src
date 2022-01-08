/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osglobals.inl
 *
 */

/* GTK System globals */

#include "osgui_gtk.ixx"

__EXTERN_C

void osglobals_init(void);

void osglobals_finish(void);

void osglobals_register_entry(GtkBorder *padding);

bool_t osglobals_impostor_mapped(void);

GtkWidget *osglobals_entry(void);

GtkStyleContext *osglobals_entry_context(void);

GtkStyleContext *osglobals_button_context(void);

GtkStyleContext *osglobals_table_context(void);

GtkStyleContext *osglobals_header_context(void);

color_t osglobals_text_color(void);

color_t osglobals_seltext_color(void);

color_t osglobals_hottext_color(void);

color_t osglobals_textbackdrop_color(void);

color_t osglobals_seltextbackdrop_color(void);

color_t osglobals_hottextbackdrop_color(void);

color_t osglobals_border_color(void);

uint32_t osglobals_check_width(void);

uint32_t osglobals_check_height(void);

GdkPixbuf* osglobals_checks_bitmap(void);

__END_C

