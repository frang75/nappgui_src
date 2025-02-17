/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osglobals_gtk.inl
 *
 */

/* GTK System globals */

#include "osgui_gtk.ixx"

__EXTERN_C

void _osglobals_init(void);

void _osglobals_finish(void);

bool_t _osglobals_impostor_mapped(void);

GtkStyleContext *_osglobals_entry_context(void);

GtkStyleContext *_osglobals_button_context(void);

GtkStyleContext *_osglobals_table_context(void);

const char_t *_osglobals_css_entry(void);

const char_t *_osglobals_css_button(void);

const char_t *_osglobals_css_radio(void);

const char_t *_osglobals_css_check(void);

const char_t *_osglobals_css_combobox(void);

const char_t *_osglobals_css_textview(void);

const char_t *_osglobals_css_textview_text(void);

color_t _osglobals_text_color(void);

color_t _osglobals_seltext_color(void);

color_t _osglobals_hottext_color(void);

color_t _osglobals_textbackdrop_color(void);

color_t _osglobals_seltextbackdrop_color(void);

color_t _osglobals_hottextbackdrop_color(void);

color_t _osglobals_border_color(void);

uint32_t _osglobals_check_width(void);

uint32_t _osglobals_check_height(void);

uint32_t _osglobals_entry_height(void);

uint32_t _osglobals_progress_height(void);

uint32_t _osglobals_scrollbar_height(void);

GdkPixbuf *_osglobals_checks_bitmap(void);

String *_osglobals_frame_focus_css(void);

void _osglobals_restore_focus(GtkWidget *window, GtkWidget *widget);

extern const uint32_t kBUTTON_VPADDING;

extern const uint32_t kBUTTON_HPADDING;

extern const uint32_t kPOPUP_VPADDING;

extern const uint32_t kPOPUP_HPADDING;

extern const uint32_t kENTRY_VPADDING;

extern const uint32_t kENTRY_HPADDING;

extern const uint32_t kBUTTON_IMAGE_SEP;

extern const uint32_t kCHECKBOX_IMAGE_SEP;

__END_C
