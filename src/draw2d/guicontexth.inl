/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: guicontexth.inl
 *
 */

/* Gui rendering context */

#include "draw2d.hxx"
#include "draw2dh.ixx"

__EXTERN_C

GuiContext *gui_context_retain(const GuiContext *context);

void gui_context_release(GuiContext **context);

void gui_context_destroy(GuiContext **context);

void gui_context_set_current(const GuiContext *context);

const GuiContext *gui_context_get_current(void);

__END_C
