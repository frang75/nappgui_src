/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: line.h
 *
 */

/* Line */

#include "gui.hxx"

__EXTERN_C

_gui_api Line *line_horizontal(void);

_gui_api Line *line_vertical(void);

_gui_api void line_length(Line *line, const real32_t length);

__END_C
