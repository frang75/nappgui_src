/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol_gtk.inl
 *
 */

/* Gtk Widgets common functions */

#include "osgui_gtk.ixx"

__EXTERN_C

void _oscontrol_init(OSControl *control, const gui_type_t type, GtkWidget *widget, GtkWidget *focus_widget, const bool_t show);

void _oscontrol_destroy(OSControl *control);

void _oscontrol_set_halign(OSControl *control, const align_t align);

void _oscontrol_set_visible(OSControl *control, const bool_t visible);

void _oscontrol_set_enabled(OSControl *control, const bool_t enabled);

void _oscontrol_get_origin(const OSControl *control, real32_t *x, real32_t *y);

void _oscontrol_get_size(const OSControl *control, real32_t *width, real32_t *height);

void _oscontrol_widget_get_size(const GtkWidget *widget, real32_t *width, real32_t *height);

void _oscontrol_set_frame(OSControl *control, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

void _oscontrol_attach_to_parent(OSControl *control, GtkWidget *parent_widget);

void _oscontrol_detach_from_parent(OSControl *control, GtkWidget *parent_widget);

void _oscontrol_widget_detach(GtkWidget *widget, GtkWidget *parent_widget);

uint32_t _oscontrol_num_children(GtkWidget *widget);

GtkWidget *_oscontrol_get_child(GtkWidget *widget, const uint32_t index);

uint32_t _oscontrol_find_child(GtkWidget *widget, GtkWidget *child);

GtkCssProvider *_oscontrol_css_provider(const char_t *css);

void _oscontrol_destroy_css_provider(GtkCssProvider **prov);

void _oscontrol_add_css_provider(GtkWidget *widget, GtkCssProvider *prov);

void _oscontrol_remove_css_provider(GtkWidget *widget, GtkCssProvider *prov);

void _oscontrol_fixed_css_provider(GtkWidget *widget, const char_t *css);

void _oscontrol_update_css_color(GtkWidget *widget, const char_t *cssobj, const color_t color, GtkCssProvider **prov);

void _oscontrol_update_css_bgcolor(GtkWidget *widget, const char_t *cssobj, const color_t color, GtkCssProvider **prov);

void _oscontrol_update_css_font(GtkWidget *widget, const char_t *cssobj, const Font *font, Font **cfont, GtkCssProvider **prov);

void _oscontrol_update_css_font_desc(GtkWidget *widget, const char_t *cssobj, const char_t *ffamily, const real32_t fsize, const uint32_t fstyle, GtkCssProvider **prov);

void _oscontrol_update_css_padding(GtkWidget *widget, const char_t *cssobj, const uint32_t vpadding, const uint32_t hpadding, GtkCssProvider **prov);

uint32_t _oscontrol_widget_font_size(GtkWidget *widget);

bool_t _oscontrol_widget_mouse_over(GtkWidget *widget, GdkEvent *event);

bool_t _oscontrol_widget_mouse_over_right(GtkWidget *widget, GdkEvent *event, gint right_px);

color_t _oscontrol_from_gdkrgba(const GdkRGBA *gdkcolor);

void _oscontrol_to_gdkrgba(const color_t color, GdkRGBA *gdkcolor);

GtkJustification _oscontrol_justification(const align_t align);

__END_C
