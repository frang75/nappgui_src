/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: webview.h
 * https://nappgui.com/en/gui/webview.html
 *
 */

/* Web view */

#include "gui.hxx"

__EXTERN_C

_gui_api WebView *webview_create(void);

_gui_api void webview_OnFocus(WebView *view, Listener *listener);

_gui_api void webview_size(WebView *view, const S2Df size);

_gui_api void webview_navigate(WebView *view, const char_t *url);

_gui_api void webview_back(WebView *view);

_gui_api void webview_forward(WebView *view);

__END_C
