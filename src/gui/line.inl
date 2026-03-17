/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: line.inl
 *
 */

/* Line */

#include "gui.ixx"

__EXTERN_C

void _line_destroy(Line **line);

void _line_natural(Line *line, const uint32_t i, real32_t *dim0, real32_t *dim1);

bool_t _line_is_horizontal(const Line *line);

__END_C
