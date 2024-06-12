/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: webview.inl
 *
 */

/* Web view */

#include "gui.ixx"

__EXTERN_C

void _webview_destroy(WebView **view);

void _webview_dimension(WebView *view, const uint32_t i, real32_t *dim0, real32_t *dim1);

__END_C
