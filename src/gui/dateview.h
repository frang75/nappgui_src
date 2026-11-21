/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: dateview.h
 *
 */

/* Date view */

#include "gui.hxx"

__EXTERN_C

_gui_api DateView *dateview_create(void);

_gui_api void dateview_OnSelect(DateView *view, Listener *listener);

_gui_api void dateview_font(DateView *view, const Font *font);

_gui_api void dateview_first_wday(DateView *view, const week_day_t wday);

_gui_api void dateview_month_names(DateView *view, const char_t **names);

_gui_api void dateview_wday_names(DateView *view, const char_t **names);

_gui_api void dateview_now(DateView *view, const Date *date);

_gui_api void dateview_select(DateView *view, const Date *date);

_gui_api Date dateview_selected(const DateView *view);

__END_C
