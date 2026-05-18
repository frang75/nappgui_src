/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tabs.inl
 *
 */

/* Tab control */

#include "gui.ixx"

__EXTERN_C

void _tabs_destroy(Tabs **tabs);

void _tabs_natural(Tabs *tabs, const uint32_t i, real32_t *dim0, real32_t *dim1);

void _tabs_expand(Tabs *tabs, const uint32_t i, const real32_t current_size, const real32_t required_size, real32_t *final_size);

void _tabs_locale(Tabs *tabs);

uint32_t _tabs_flags(const Tabs *tabs);

__END_C
