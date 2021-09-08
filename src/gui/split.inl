/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: split.inl
 *
 */

/* Split view */

#include "gui.ixx"

__EXTERN_C

void _splitview_destroy(SplitView **split);

void _splitview_dimension(SplitView *split, const uint32_t i, real32_t *dim0, real32_t *dim1);

void _splitview_OnResize(SplitView *split, const S2Df *size);

__END_C
