/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: split.h
 * https://nappgui.com/en/gui/split.html
 *
 */

/* Split view */

#include "gui.hxx"

__EXTERN_C

SplitView *splitview_horizontal(void);

SplitView *splitview_vertical(void);

void splitview_view(SplitView *split, View *view);

void splitview_text(SplitView *split, TextView *view);

void splitview_split(SplitView *split, SplitView *child);

void splitview_panel(SplitView *split, Panel *panel);

void splitview_divider(SplitView *split, const real32_t pos);

__END_C

