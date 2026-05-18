/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tabs.h
 *
 */

/* Tab control */

#include "gui.hxx"

__EXTERN_C

_gui_api Tabs *tabs_create(const gui_pos_t pos);

_gui_api void tabs_OnSelect(Tabs *tabs, Listener *listener);

_gui_api void tabs_length(Tabs *tabs, const real32_t length);

_gui_api void tabs_tooltip(Tabs *tabs, const char_t *text);

_gui_api void tabs_add_elem(Tabs *tabs, const char_t *text, const Image *image);

_gui_api void tabs_set_elem(Tabs *tabs, const uint32_t index, const char_t *text, const Image *image);

_gui_api void tabs_ins_elem(Tabs *tabs, const uint32_t index, const char_t *text, const Image *image);

_gui_api void tabs_del_elem(Tabs *tabs, const uint32_t index);

_gui_api void tabs_clear(Tabs *tabs);

_gui_api uint32_t tabs_count(const Tabs *tabs);

_gui_api void tabs_selected(Tabs *tabs, const uint32_t index);

_gui_api uint32_t tabs_get_selected(const Tabs *tabs);

_gui_api const char_t *tabs_get_text(const Tabs *tabs, const uint32_t index);

_gui_api const Image *tabs_get_image(const Tabs *tabs, const uint32_t index);

__END_C
