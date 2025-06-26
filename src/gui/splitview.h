/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
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

_gui_api SplitView *splitview_horizontal(void);

_gui_api SplitView *splitview_vertical(void);

_gui_api void splitview_view(SplitView *split, View *view, const bool_t tabstop);

_gui_api void splitview_textview(SplitView *split, TextView *view, const bool_t tabstop);

_gui_api void splitview_webview(SplitView *split, WebView *view, const bool_t tabstop);

_gui_api void splitview_tableview(SplitView *split, TableView *view, const bool_t tabstop);

_gui_api void splitview_splitview(SplitView *split, SplitView *view);

_gui_api void splitview_panel(SplitView *split, Panel *panel);

_gui_api void splitview_pos(SplitView *split, const split_mode_t mode, const real32_t pos);

_gui_api real32_t splitview_get_pos(const SplitView *split, const split_mode_t mode);

_gui_api void splitview_visible0(SplitView *split, const bool_t visible);

_gui_api void splitview_visible1(SplitView *split, const bool_t visible);

_gui_api void splitview_minsize0(SplitView *split, const real32_t size);

_gui_api void splitview_minsize1(SplitView *split, const real32_t size);

__END_C
