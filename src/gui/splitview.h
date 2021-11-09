/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: splitview.h
 * https://nappgui.com/en/gui/splitview.html
 *
 */

/* Split view */

#include "gui.hxx"

__EXTERN_C

SplitView *splitview_horizontal(void);

SplitView *splitview_vertical(void);

void splitview_size(SplitView *split, const S2Df size);

void splitview_view(SplitView *split, View *view);

void splitview_text(SplitView *split, TextView *view);

void splitview_split(SplitView *split, SplitView *child);

void splitview_panel(SplitView *split, Panel *panel);

void splitview_pos(SplitView *split, const real32_t pos);

__END_C

