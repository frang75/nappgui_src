/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osweb.cpp
 *
 */

/* Operating System native web view */
/*
 * WebView2 known issues
 * VS2017 Debug-mode crash: https://github.com/MicrosoftEdge/WebView2Feedback/issues/2614
 * WebView2 Windows7 end support: https://blogs.windows.com/msedgedev/2022/12/09/microsoft-edge-and-webview2-ending-support-for-windows-7-and-windows-8-8-1/
 *
 */
#include "osweb.h"
#include "osweb_win.inl"
#include "osgui.inl"
#include "osgui_win.inl"
#include "oscontrol_win.inl"
#include "ospanel_win.inl"
#include "oswindow_win.inl"
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

typedef struct _webimpt_t WebImp;

#if defined(NAPPGUI_WEB_SUPPORT)
#include <sewer/nowarn.hxx>
#include <WebView2.h>
#include <sewer/warn.hxx>

class WebCtrlCallback;
class WebEnvCallback;

/*---------------------------------------------------------------------------*/

struct _webimpt_t
{
    HWND hwnd;
    ICoreWebView2Controller *webViewCtrl;
    ICoreWebView2 *webView;
    WebCtrlCallback *webCtrlCallback;
    WebEnvCallback *webEnvCallback;
    String *navigate;
};

/*---------------------------------------------------------------------------*/

static void i_web_navigate(WebImp *web, const char_t *url)
{
    uint32_t num_bytes = 0;
    WCHAR *wtext_alloc = NULL;
    WCHAR wtext_static[WCHAR_BUFFER_SIZE];
    WCHAR *wtext = NULL;
    cassert_no_null(web);
    cassert_no_null(web->webView);

    num_bytes = unicode_convers_nbytes(url, ekUTF8, kWINDOWS_UNICODE);
    if (num_bytes < sizeof(wtext_static))
    {
        wtext = wtext_static;
    }
    else
    {
        wtext_alloc = cast(heap_malloc(num_bytes, "OSWebViewNavigate"), WCHAR);
        wtext = wtext_alloc;
    }

    {
        uint32_t bytes = unicode_convers(url, cast(wtext, char_t), ekUTF8, kWINDOWS_UNICODE, num_bytes);
        cassert_unref(bytes == num_bytes, bytes);
    }

    web->webView->Navigate(wtext);

    if (wtext_alloc != NULL)
        heap_free(dcast(&wtext_alloc, byte_t), num_bytes, "OSWebViewNavigate");
}

/*---------------------------------------------------------------------------*/

class WebCtrlCallback : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
{
  public:
    WebCtrlCallback(WebImp *webImp)
        : mWebImp(webImp), mRefs(0)
    {
    }

    /*---------------------------------------------------------------------------*/

    virtual HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Controller *webViewCtrl)
    {
        ICoreWebView2Settings *settings = NULL;
        cassert_no_null(this->mWebImp);
        unref(errorCode);

        if (webViewCtrl != NULL)
        {
            cassert(this->mWebImp->webViewCtrl == NULL);
            cassert(this->mWebImp->webView == NULL);
            webViewCtrl->AddRef();
            this->mWebImp->webViewCtrl = webViewCtrl;
            this->mWebImp->webViewCtrl->get_CoreWebView2(&this->mWebImp->webView);
        }

        /* Add a few settings for the webview */
        this->mWebImp->webView->get_Settings(&settings);
        settings->put_IsScriptEnabled(TRUE);
        settings->put_AreDefaultScriptDialogsEnabled(TRUE);
        settings->put_IsWebMessageEnabled(TRUE);

        RECT bounds;
        GetClientRect(this->mWebImp->hwnd, &bounds);
        this->mWebImp->webViewCtrl->put_Bounds(bounds);

        /* Pending navigation */
        if (this->mWebImp->navigate != NULL)
        {
            i_web_navigate(this->mWebImp, tc(this->mWebImp->navigate));
            str_destroy(&this->mWebImp->navigate);
        }

        return S_OK;
    }

    /*---------------------------------------------------------------------------*/

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        unref(riid);
        unref(ppvObject);
        return S_OK;
    }

    /*---------------------------------------------------------------------------*/

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        mRefs += 1;
        return mRefs;
    }

    /*---------------------------------------------------------------------------*/

    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        cassert(mRefs > 0);
        mRefs -= 1;
        return mRefs;
    }

    WebImp *mWebImp;
    ULONG mRefs;
};

/*---------------------------------------------------------------------------*/

class WebEnvCallback : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler
{
  public:
    WebEnvCallback(WebImp *webImp)
        : mWebImp(webImp), mRefs(0)
    {
    }

    /*---------------------------------------------------------------------------*/

    virtual HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Environment *webEnv)
    {
        cassert(mWebImp->webCtrlCallback == NULL);
        mWebImp->webCtrlCallback = new WebCtrlCallback(mWebImp);
        webEnv->CreateCoreWebView2Controller(mWebImp->hwnd, mWebImp->webCtrlCallback);
        unref(errorCode);
        return S_OK;
    }

    /*---------------------------------------------------------------------------*/

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        unref(riid);
        unref(ppvObject);
        return S_OK;
    }

    /*---------------------------------------------------------------------------*/

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        mRefs += 1;
        return mRefs;
    }

    /*---------------------------------------------------------------------------*/

    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        cassert(mRefs > 0);
        mRefs -= 1;
        return mRefs;
    }

    WebImp *mWebImp;
    ULONG mRefs;
};

/*---------------------------------------------------------------------------*/

static void i_remove_webimp(WebImp *web)
{
    cassert_no_null(web);
    str_destopt(&web->navigate);
    delete web->webCtrlCallback;
    delete web->webEnvCallback;
}

#else

struct _webimpt_t
{
    uint32_t dummy;
};

#endif

struct _osweb_t
{
    OSControl control;
    uint32_t flags;
    bool_t launch_event;
    RECT border;
    Listener *OnFocus;
    WebImp web;
};

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSWeb *view = (OSWeb *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    cassert_no_null(view);

    switch (uMsg)
    {
    case WM_NCCALCSIZE:
        if (view->flags & ekVIEW_BORDER)
        {
            LRESULT res = CallWindowProc(view->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
            res = _osgui_nccalcsize(hwnd, wParam, lParam, TRUE, 0, &view->border);
            return res;
        }
        break;

    case WM_NCPAINT:
        if (view->flags & ekVIEW_BORDER)
        {
            bool_t focused = (bool_t)(GetFocus() == hwnd);
            CallWindowProc(view->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
            return _osgui_ncpaint(hwnd, focused, &view->border, NULL);
        }
        break;

    case WM_SETFOCUS:
        if (view->flags & ekVIEW_BORDER)
            RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

        if (view->launch_event == TRUE)
        {
            if (view->OnFocus != NULL)
            {
                bool_t params = TRUE;
                listener_event(view->OnFocus, ekGUI_EVENT_FOCUS, view, &params, NULL, OSWeb, bool_t, void);
            }
        }
        break;

    case WM_KILLFOCUS:
        if (view->flags & ekVIEW_BORDER)
            RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

        if (view->launch_event == TRUE)
        {
            if (view->OnFocus != NULL)
            {
                bool_t params = FALSE;
                listener_event(view->OnFocus, ekGUI_EVENT_FOCUS, view, &params, NULL, OSWeb, bool_t, void);
            }
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (_oswindow_mouse_down(cast(view, OSControl)) == TRUE)
            break;
        return 0;
    }

    return CallWindowProc(view->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

OSWeb *osweb_create(const uint32_t flags)
{
    OSWeb *view = NULL;
    DWORD dwStyle = 0;
    unref(flags);
    view = heap_new0(OSWeb);
    view->control.type = ekGUI_TYPE_WEBVIEW;
    view->flags = flags;
    dwStyle = WS_CHILD | WS_CLIPSIBLINGS | CS_HREDRAW | CS_VREDRAW;

    if (flags & ekVIEW_BORDER)
        dwStyle |= WS_BORDER;

    _oscontrol_init((OSControl *)view, PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY), dwStyle, kWEBVIEW_CLASS, 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    view->launch_event = TRUE;

#if defined(NAPPGUI_WEB_SUPPORT)
    {
        cassert(view->web.webEnvCallback == NULL);
        view->web.hwnd = view->control.hwnd;
        view->web.webEnvCallback = new WebEnvCallback(&view->web);
        CreateCoreWebView2EnvironmentWithOptions(NULL, NULL, NULL, view->web.webEnvCallback);
    }
#endif

    return view;
}

/*---------------------------------------------------------------------------*/

void osweb_destroy(OSWeb **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    listener_destroy(&(*view)->OnFocus);

#if defined(NAPPGUI_WEB_SUPPORT)
    i_remove_webimp(&(*view)->web);
#endif

    _oscontrol_destroy((OSControl *)*view);
    heap_delete(view, OSWeb);
}

/*---------------------------------------------------------------------------*/

void osweb_OnFocus(OSWeb *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osweb_command(OSWeb *view, const gui_web_t cmd, const void *param, void *result)
{
#if defined(NAPPGUI_WEB_SUPPORT)
    cassert_no_null(view);
    unref(result);
    switch (cmd)
    {
    case ekGUI_WEB_NAVIGATE:
        if (view->web.webView != NULL)
            i_web_navigate(&view->web, cast_const(param, char_t));
        else
            str_upd(&view->web.navigate, cast_const(param, char_t));
        break;

    case ekGUI_WEB_BACK:
        if (view->web.webView != NULL)
            view->web.webView->GoBack();
        break;

    case ekGUI_WEB_FORWARD:
        if (view->web.webView != NULL)
            view->web.webView->GoForward();
        break;

        cassert_default();
    }
#else
    unref(view);
    unref(cmd);
    unref(param);
    unref(result);
#endif
}

/*---------------------------------------------------------------------------*/

void osweb_scroller_visible(OSWeb *view, const bool_t horizontal, const bool_t vertical)
{
    unref(view);
    unref(horizontal);
    unref(vertical);
}

/*---------------------------------------------------------------------------*/

void osweb_set_need_display(OSWeb *view)
{
    unref(view);
}

/*---------------------------------------------------------------------------*/

void osweb_clipboard(OSWeb *view, const clipboard_t clipboard)
{
    unref(view);
    unref(clipboard);
}

/*---------------------------------------------------------------------------*/

void osweb_attach(OSWeb *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void osweb_detach(OSWeb *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void osweb_visible(OSWeb *view, const bool_t visible)
{
    _oscontrol_set_visible(cast(view, OSControl), visible);

#if defined(NAPPGUI_WEB_SUPPORT)
    cassert_no_null(view);
    if (view->web.webViewCtrl != NULL)
        view->web.webViewCtrl->put_IsVisible((BOOL)visible);
#endif
}

/*---------------------------------------------------------------------------*/

void osweb_enabled(OSWeb *view, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(view, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osweb_size(const OSWeb *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(view, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osweb_origin(const OSWeb *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(view, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osweb_frame(OSWeb *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl *)view, x, y, width, height);

#if defined(NAPPGUI_WEB_SUPPORT)
    cassert_no_null(view);
    /* The webview callback may not have been executed yet */
    if (view->web.webViewCtrl != NULL)
    {
        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = (LONG)width;
        bounds.bottom = (LONG)height;
        view->web.webViewCtrl->put_Bounds(bounds);
    }
#endif
}

/*---------------------------------------------------------------------------*/

HWND _osweb_focus_widget(OSWeb *view)
{
    cassert_no_null(view);
    return view->control.hwnd;
}
