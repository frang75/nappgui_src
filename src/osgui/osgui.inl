/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui.inl
 *
 */

/* Operating system native gui */

#include "osgui.ixx"

__EXTERN_C

void _osgui_start_imp(void);

void _osgui_finish_imp(void);

Font *_osgui_create_default_font(void);

void _osgui_word_size(StringSizeData *data, const char_t *word, real32_t *width, real32_t *height);

void _osgui_text_bounds(StringSizeData *data, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height);

const char_t *_osgui_component_type(const gui_type_t type);

bool_t _osgui_button_text_allowed(const uint32_t flags);

bool_t _osgui_button_image_allowed(const uint32_t flags);

gui_size_t _osgui_size_font(const real32_t font_size);

Image *_osgui_scale_image(const Image *image, const Font *font);

void _osgui_attach_menubar(OSWindow *window, OSMenu *menu);

void _osgui_detach_menubar(OSWindow *window, OSMenu *menu);

void _osgui_change_menubar(OSWindow *window, OSMenu *previous_menu, OSMenu *new_menu);

bool_t _osgui_process_message(void *msg);

__END_C
