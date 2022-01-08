/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: popup.h
 * https://nappgui.com/en/gui/popup.html
 *
 */

/* Pop-up button */

#include "gui.hxx"

__EXTERN_C

PopUp *popup_create(void);

void popup_OnSelect(PopUp *popup, Listener *listener);

void popup_tooltip(PopUp *popup, const char_t *text);

void popup_add_elem(PopUp *popup, const char_t *text, const Image *image);

void popup_set_elem(PopUp *popup, const uint32_t index, const char_t *text, const Image *image);

void popup_clear(PopUp *popup);

uint32_t popup_count(const PopUp *popup);

void popup_list_height(PopUp *popup, const uint32_t elems);

void popup_selected(PopUp *popup, const uint32_t index);

uint32_t popup_get_selected(const PopUp *popup);

__END_C

