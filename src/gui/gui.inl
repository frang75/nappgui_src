/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gui.inl
 *
 */

/* Graphics User Interface */

#include "gui.ixx"

__EXTERN_C

Window *_gui_main_window(void);

Font *_gui_create_default_font(void);

bool_t _gui_effective_alt_font(const Font *font, const Font *alt_font);

void _gui_update_font(Font **font, Font **alt_font, const Font *new_font);

void _gui_add_transition_imp(void *owner, Listener *listener);

void _gui_delete_transition_imp(void *owner);

void _gui_add_window(Window *window);

void _gui_delete_window(Window *window);

void _gui_add_menu(Menu *menu);

void _gui_delete_menu(Menu *menu);

const char_t *_gui_respack_text(const ResId id, ResId *store_id);

const Image *_gui_respack_image(const ResId id, ResId *store_id);

const Cursor *_gui_cursor(const cursor_t cursor, const Image *image, const real32_t hot_x, const real32_t hot_y);

void _gui_OnThemeChanged(void);

void _gui_OnTranslate(Listener *listener);

void _gui_OnMenubar(Listener *listener);

void _gui_OnFinish(Listener *listener);

void gui_update_transitions(const real64_t prtime, const real64_t crtime);

#define GUI_COMPONENT_MAX_PANELS    2

__END_C

#define _gui_add_transition(owner, listener, type)\
    (\
        (void)((type*)owner == owner),\
        _gui_add_transition_imp((void*)owner, listener)\
    )

#define _gui_delete_transition(owner, type)\
    (\
        (void)((type*)owner == owner),\
        _gui_delete_transition_imp((void*)owner)\
    )
