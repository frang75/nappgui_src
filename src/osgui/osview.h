/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.h
 *
 */

/* Operating System native custom view */

#include "osgui.hxx"

__EXTERN_C

OSView *osview_create(const uint32_t flags);

void osview_destroy(OSView **view);

void osview_OnDraw(OSView *view, Listener *listener);

void osview_OnEnter(OSView *view, Listener *listener);

void osview_OnExit(OSView *view, Listener *listener);

void osview_OnMoved(OSView *view, Listener *listener);

void osview_OnDown(OSView *view, Listener *listener);

void osview_OnUp(OSView *view, Listener *listener);

void osview_OnClick(OSView *view, Listener *listener);

void osview_OnDrag(OSView *view, Listener *listener);

void osview_OnWheel(OSView *view, Listener *listener);

void osview_OnKeyDown(OSView *view, Listener *listener);

void osview_OnKeyUp(OSView *view, Listener *listener);

void osview_OnFocus(OSView *view, Listener *listener);

void osview_OnNotify(OSView *view, Listener *listener);

void osview_scroll(OSView *view, const real32_t x, const real32_t y);

void osview_scroll_get(const OSView *view, real32_t *x, real32_t *y);

void osview_scroller_size(const OSView *view, real32_t *width, real32_t *height);

void osview_content_size(OSView *view, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height);

real32_t osview_scale_factor(const OSView *view);

void osview_set_need_display(OSView *view);

void *osview_get_native_view(const OSView *view);


void osview_attach(OSView *view, OSPanel *panel);

void osview_detach(OSView *view, OSPanel *panel);

void osview_visible(OSView *view, const bool_t visible);

void osview_enabled(OSView *view, const bool_t enabled);

void osview_size(const OSView *view, real32_t *width, real32_t *height);

void osview_origin(const OSView *view, real32_t *x, real32_t *y);

void osview_frame(OSView *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

__END_C

