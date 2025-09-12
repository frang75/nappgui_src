/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.c
 *
 */

/* Operating System native window */

#include "oswindow_win.inl"
#include "osgui_win.inl"
#include "osbutton_win.inl"
#include "oscontrol_win.inl"
#include "osmenuitem_win.inl"
#include "ospanel_win.inl"
#include "ossplit_win.inl"
#include "../oswindow.h"
#include "../oswindow.inl"
#include "../osgui.inl"
#include "../ostabstop.inl"
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/event.h>
#include <core/heap.h>
#include <osbs/osbs.h>
#include <osbs/bthread.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

typedef enum _wstate_t
{
    ekNORMAL,
    i_ekSTATE_MANAGED
} wstate_t;

struct _oswindow_t
{
    OSControl control;
    DWORD dwStyle;
    DWORD dwExStyle;
    HMENU current_popup_menu;
    HCURSOR cursor;
    bool_t launch_resize_event;
    bool_t destroy_main_view;
    bool_t wm_sizing;
    uint32_t flags;
    wstate_t state;
    gui_role_t role;
    OSPanel *main_panel;
    Listener *OnMoved;
    Listener *OnResize;
    Listener *OnClose;
    OSTabStop tabstop;
    ArrSt(OSHotKey) *hotkeys;
};

DeclPt(Listener);

/*---------------------------------------------------------------------------*/

#define i_WM_MODAL_STOP 0x444
static HWND i_CURRENT_ACTIVE_WINDOW = NULL;
static ArrPt(Listener) *i_IDLES = NULL;

/*---------------------------------------------------------------------------*/

/* Same as AdjustWindowRectEx, but compatible with multi-row menubars */
static void i_adjust_window_size(HWND hwnd, const LONG width, const LONG height, const DWORD dwStyle, const DWORD dwExStyle, LONG *nwidth, LONG *nheight)
{
    RECT rect;
    BOOL ok = FALSE;
    HMENU menubar = GetMenu(hwnd);
    cassert_no_null(nwidth);
    cassert_no_null(nheight);
    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    ok = AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
    cassert_unref(ok != 0, ok);
    *nwidth = rect.right - rect.left;
    *nheight = rect.bottom - rect.top;

    if (menubar != NULL)
    {
        RECT mrect, irect;
        int i, n = GetMenuItemCount(menubar);
        SetRectEmpty(&mrect);
        for (i = 0; i < n; ++i)
        {
            GetMenuItemRect(hwnd, menubar, (UINT)i, &irect);
            UnionRect(&mrect, &mrect, &irect);
        }

        *nheight += mrect.bottom - mrect.top;
    }
}

/*---------------------------------------------------------------------------*/

static void i_resizing(OSWindow *window, WPARAM edge, RECT *wrect)
{
    cassert_no_null(window);
    cassert_no_null(wrect);
    if (window->launch_resize_event == TRUE)
    {
        if (window->OnResize != NULL)
        {
            /* ekGUI_EVENT_WND_SIZING needs the client area size (no full window size) */
            EvSize params;
            EvSize result;

            {
                LONG nwidth, nheight;
                i_adjust_window_size(window->control.hwnd, 0, 0, window->dwStyle, window->dwExStyle, &nwidth, &nheight);
                params.width = (real32_t)((wrect->right - wrect->left) - nwidth);
                params.height = (real32_t)((wrect->bottom - wrect->top) - nheight);
            }

            listener_event(window->OnResize, ekGUI_EVENT_WND_SIZING, window, &params, &result, OSWindow, EvSize, EvSize);

            {
                LONG nwidth, nheight;
                i_adjust_window_size(window->control.hwnd, (LONG)result.width, (LONG)result.height, window->dwStyle, window->dwExStyle, &nwidth, &nheight);

                switch (edge)
                {
                case WMSZ_RIGHT:
                    wrect->right = wrect->left + nwidth;
                    break;
                case WMSZ_BOTTOM:
                    wrect->bottom = wrect->top + nheight;
                    break;
                case WMSZ_LEFT:
                    wrect->left = wrect->right - nwidth;
                    break;
                case WMSZ_TOP:
                    wrect->top = wrect->bottom - nheight;
                    break;
                case WMSZ_BOTTOMLEFT:
                    wrect->left = wrect->right - nwidth;
                    wrect->bottom = wrect->top + nheight;
                    break;
                case WMSZ_BOTTOMRIGHT:
                    wrect->right = wrect->left + nwidth;
                    wrect->bottom = wrect->top + nheight;
                    break;
                case WMSZ_TOPLEFT:
                    wrect->left = wrect->right - nwidth;
                    wrect->top = wrect->bottom - nheight;
                    break;
                case WMSZ_TOPRIGHT:
                    wrect->right = wrect->left + nwidth;
                    wrect->top = wrect->bottom - nheight;
                    break;
                default:
                    break;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_resize(OSWindow *window, LONG client_width, LONG client_height)
{
    cassert_no_null(window);
    if (window->launch_resize_event == TRUE && window->OnResize != NULL)
    {
        EvSize params;
        params.width = (real32_t)client_width;
        params.height = (real32_t)client_height;
        listener_event(window->OnResize, ekGUI_EVENT_WND_SIZE, window, &params, NULL, OSWindow, EvSize, void);
    }
}

/*---------------------------------------------------------------------------*/

static void i_moved(OSWindow *window)
{
    cassert_no_null(window);
    if (window->OnMoved != NULL)
    {
        RECT rect;
        EvPos params;
        _osgui_frame_without_shadows(window->control.hwnd, &rect);
        params.x = (real32_t)(rect.left);
        params.y = (real32_t)(rect.top);
        listener_event(window->OnMoved, ekGUI_EVENT_WND_MOVED, window, &params, NULL, OSWindow, EvPos, void);
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_close(OSWindow *window, const gui_close_t close_origin)
{
    bool_t closed = TRUE;
    cassert_no_null(window);

    /* Checks if the current control allows the window to be closed */
    if (close_origin == ekGUI_CLOSE_INTRO)
        closed = _ostabstop_can_close_window(&window->tabstop);

    /* Notify the user and check if allows the window to be closed */
    if (closed == TRUE && window->OnClose != NULL)
    {
        EvWinClose params;
        params.origin = close_origin;
        listener_event(window->OnClose, ekGUI_EVENT_WND_CLOSE, window, &params, &closed, OSWindow, EvWinClose, bool_t);
    }

    return closed;
}

/*---------------------------------------------------------------------------*/

static void i_menu_command(HWND hwnd, HMENU popup_hmenu, WORD command_id)
{
    MENUITEMINFO info = {0};
    HMENU hmenu = NULL;
    BOOL ok = FALSE;
    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_DATA | MIIM_ID | MIIM_FTYPE | MIIM_STATE;

    hmenu = GetMenu(hwnd);
    if (hmenu != NULL)
        ok = GetMenuItemInfo(hmenu, command_id, MF_BYCOMMAND, &info);

    if (ok == FALSE && popup_hmenu != NULL)
        ok = GetMenuItemInfo(popup_hmenu, command_id, MF_BYCOMMAND, &info);

    /* Command from accelerator without active menu doesn't send event */
    if (ok == TRUE)
    {
        cassert(info.wID == command_id);
        _osmenuitem_click(cast(info.dwItemData, OSMenuItem), info.wID, info.fType, info.fState);
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_press_defbutton(OSWindow *window)
{
    cassert_no_null(window);
    if (window->tabstop.defbutton != NULL)
    {
        HWND button_hwnd = cast(window->tabstop.defbutton, OSControl)->hwnd;
        /* Simulates the click hightlight */
        SendMessage(button_hwnd, BM_SETSTATE, 1, 0);
        SendMessage(button_hwnd, WM_PAINT, 0, 0);
        _osbutton_command(window->tabstop.defbutton, (WPARAM)MAKELONG(0, BN_CLICKED), FALSE);
        bthread_sleep(100);
        SendMessage(button_hwnd, BM_SETSTATE, 0, 0);
        SendMessage(button_hwnd, WM_PAINT, 0, 0);
        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_activate(OSWindow *window)
{
    cassert_no_null(window);
    /* Force the tabstop because 'WM_ACTIVATE' is not send if hwnd is the current active */
    SetActiveWindow(window->control.hwnd);
    _ostabstop_restore(&window->tabstop);
}

/*---------------------------------------------------------------------------*/

static ___INLINE bool_t i_sizing_by_dragging(WPARAM wParam)
{
    switch (wParam)
    {
    case WMSZ_LEFT:
    case WMSZ_RIGHT:
    case WMSZ_TOP:
    case WMSZ_TOPLEFT:
    case WMSZ_TOPRIGHT:
    case WMSZ_BOTTOM:
    case WMSZ_BOTTOMLEFT:
    case WMSZ_BOTTOMRIGHT:
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    OSWindow *window = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSWindow);
    cassert_no_null(window);

    switch (msg)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
        {
            i_CURRENT_ACTIVE_WINDOW = hwnd;
            _ostabstop_restore(&window->tabstop);
        }
        else
        {
            cassert(LOWORD(wParam) == WA_INACTIVE);
            if (window->role == ekGUI_ROLE_OVERLAY)
            {
                if (i_close(window, ekGUI_CLOSE_DEACT) == TRUE)
                    window->role = ENUM_MAX(gui_role_t);
            }

            i_CURRENT_ACTIVE_WINDOW = NULL;
        }

        return 0;

    case WM_COMMAND:
        /* COMMAND by Control */
        if (lParam != 0)
        {
            /* Press enter when a button has de focus */
            OSControl *control = cast(GetWindowLongPtr((HWND)lParam, GWLP_USERDATA), OSControl);
            cassert_no_null(control);
            cassert(control->type == ekGUI_TYPE_BUTTON);
            _osbutton_command(cast(control, OSButton), wParam, TRUE);
            return 0;
        }

        /* COMMAND by Menu */
        if (HIWORD(wParam) == 0)
        {
            /* DialogBox "Menu" Commands? */
            switch (LOWORD(wParam))
            {
            case IDCANCEL:
            {
                i_close(window, ekGUI_CLOSE_ESC);
                return TRUE;
            }

            case IDOK:
                i_press_defbutton(window);
                i_close(window, ekGUI_CLOSE_INTRO);
                return 0;

            default:
                break;
            }

            /* Menu Click */
            i_menu_command(hwnd, window->current_popup_menu, LOWORD(wParam));
            return 0;
        }
        /* COMMAND by Accelerator */
        else
        {
            WORD cmd = LOWORD(wParam);
            HWND ahwnd = _osgui_hwnd_accelerator(cmd);
            cassert(HIWORD(wParam) == 1);
            if (ahwnd != NULL)
            {
                /* Accelerator from button */
                OSControl *control = cast(GetWindowLongPtr(ahwnd, GWLP_USERDATA), OSControl);
                DWORD nwParam = MAKELONG(cmd, BN_CLICKED);
                /* Simulates the click hightlight */
                SendMessage(control->hwnd, BM_SETSTATE, 1, 0);
                SendMessage(control->hwnd, WM_PAINT, 0, 0);
                bthread_sleep(100);
                _osbutton_toggle(cast(control, OSButton));
                SendMessage(control->hwnd, BM_SETSTATE, 0, 0);
                SendMessage(control->hwnd, WM_PAINT, 0, 0);
                _osbutton_command(cast(control, OSButton), nwParam, TRUE);
            }
            else
            {
                /* Accelerator from menu */
                i_menu_command(hwnd, window->current_popup_menu, LOWORD(wParam));
            }

            return 0;
        }

    case WM_SIZING:
    {
        RECT *rect = cast(lParam, RECT);
        RECT current = *rect;
        i_resizing(window, wParam, rect);
        /* The resize edges rect is accepted --> The WM_SIZE event will be triggered */
        if (((current.right - current.left) == (rect->right - rect->left)) && ((current.bottom - current.top) == (rect->bottom - rect->top)))
        {
            window->wm_sizing = i_sizing_by_dragging(wParam);
        }
        /* Some edge has been blocked by i_resizing(), we force the WM_SIZE event */
        else
        {
            LONG owidth, oheight;
            LONG clwidth, clheight;
            i_adjust_window_size(window->control.hwnd, 0, 0, window->dwStyle, window->dwExStyle, &owidth, &oheight);
            clwidth = (rect->right - rect->left) - owidth;
            clheight = (rect->bottom - rect->top) - oheight;
            PostMessage(window->control.hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(clwidth, clheight));
            window->wm_sizing = TRUE;
        }
        return TRUE;
    }

    case WM_SIZE:
        /*
         * NAppGUI resizing in done in two steps:
         * WM_SIZING: The user is dragging the edges. i_resizing() can block the edges
         *            when the dragging exceed the window constraint limits.
         * WM_SIZE: Will be called immediately after WM_SIZING and will apply the controls
         *          dimensions calculated by i_resizing(). In these cases wParam == SIZE_RESTORED (0).
         *
         * But...
         * We can recibe single WM_SIZE with SIZE_RESTORED events when using the new snap feature
         * https://support.microsoft.com/en-us/windows/snap-your-windows-885a9b1e-a983-a3b1-16cd-c531795e6241
         * It these cases, i_resizing() step must be done, to correcly resize the window.
         */
        if (IsWindowVisible(window->control.hwnd) == TRUE && wParam != SIZE_MINIMIZED)
        {
            if (window->wm_sizing == FALSE)
            {
                LONG client_width = LOWORD(lParam);
                LONG client_height = HIWORD(lParam);
                RECT rect;
                rect.left = 0;
                rect.top = 0;
                i_adjust_window_size(window->control.hwnd, client_width, client_height, window->dwStyle, window->dwExStyle, &rect.right, &rect.bottom);
                /* i_resizing() uses full window size and not client area size */
                i_resizing(window, 1, &rect);
            }
            i_resize(window, LOWORD(lParam), HIWORD(lParam));
        }
        window->wm_sizing = FALSE;
        return 0;

    case WM_MOVE:
        i_moved(window);
        return 0;

    case WM_CLOSE:
        i_close(window, ekGUI_CLOSE_BUTTON);
        return 0;

    case WM_SETCURSOR:
        if (window->cursor != NULL)
        {
            SetCursor(window->cursor);
            return TRUE;
        }
        break;

    case i_WM_MODAL_STOP:
        PostQuitMessage((int)wParam);
        return 0;

    default:
        break;
    }

    return CallWindowProc(window->control.def_wnd_proc, hwnd, msg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

static void i_window_style(const window_flag_t flags, DWORD *dwStyle, DWORD *dwExStyle)
{
    cassert_no_null(dwStyle);
    cassert_no_null(dwExStyle);

    *dwStyle = 0 | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    *dwExStyle = 0;

    if (flags & ekWINDOW_EDGE)
        *dwStyle |= WS_BORDER;

    if (flags & ekWINDOW_TITLE)
    {
        *dwStyle |= WS_CAPTION | WS_DLGFRAME | WS_OVERLAPPED;
        *dwExStyle |= WS_EX_DLGMODALFRAME;
    }

    if (flags & ekWINDOW_CLOSE)
        *dwStyle |= WS_SYSMENU;

    if (flags & ekWINDOW_MAX)
        *dwStyle |= WS_MAXIMIZEBOX | WS_SYSMENU;

    if (flags & ekWINDOW_MIN)
        *dwStyle |= WS_MINIMIZEBOX | WS_SYSMENU;

    if (flags & ekWINDOW_RESIZE)
        *dwStyle |= WS_THICKFRAME;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_create(const uint32_t flags)
{
    OSWindow *window = heap_new0(OSWindow);
    window->control.type = ekGUI_TYPE_WINDOW;
    i_window_style(flags, &window->dwStyle, &window->dwExStyle);
    _oscontrol_init_hidden(cast(window, OSControl), window->dwExStyle, window->dwStyle | WS_POPUP, kWINDOW_CLASS, 0, 0, i_WndProc, GetDesktopWindow());
    window->launch_resize_event = TRUE;
    window->destroy_main_view = TRUE;
    window->wm_sizing = FALSE;
    window->flags = flags;
    window->state = ekNORMAL;
    window->role = ENUM_MAX(gui_role_t);
    _ostabstop_init(&window->tabstop, window);

    {
        HICON icon = LoadIcon(_osgui_instance(), L"APPLICATION_ICON");
        if (icon != NULL)
        {
            SendMessage(window->control.hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
            SendMessage(window->control.hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
        }
    }

    return window;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_managed(void *native_ptr)
{
    OSWindow *window = heap_new(OSWindow);
    cassert_no_null(native_ptr);
    window->control.type = ekGUI_TYPE_WINDOW;
    window->control.hwnd = (HWND)native_ptr;
    window->control.def_wnd_proc = NULL;
    window->launch_resize_event = TRUE;
    window->state = i_ekSTATE_MANAGED;
    window->role = ENUM_MAX(gui_role_t);
    window->destroy_main_view = TRUE;
    return window;
}

/*---------------------------------------------------------------------------*/

void oswindow_destroy(OSWindow **window)
{
    cassert_no_null(window);
    cassert_no_null(*window);
    if ((*window)->destroy_main_view == TRUE && (*window)->main_panel != NULL)
    {
        OSPanel *panel = (*window)->main_panel;
        oswindow_detach_panel(*window, panel);
        _ospanel_destroy(&panel);
    }

    cassert((*window)->main_panel == NULL);
    cassert(_oscontrol_num_children(cast(*window, OSControl)) == 0);
    listener_destroy(&(*window)->OnMoved);
    listener_destroy(&(*window)->OnResize);
    listener_destroy(&(*window)->OnClose);
    _oswindow_hotkey_destroy(&(*window)->hotkeys);
    _ostabstop_remove(&(*window)->tabstop);

    if ((*window)->state != i_ekSTATE_MANAGED)
        _oscontrol_destroy(&(*window)->control);

    heap_delete(window, OSWindow);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnMoved(OSWindow *window, Listener *listener)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    listener_update(&window->OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnResize(OSWindow *window, Listener *listener)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    listener_update(&window->OnResize, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnClose(OSWindow *window, Listener *listener)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    listener_update(&window->OnClose, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_title(OSWindow *window, const char_t *text)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    _oscontrol_set_text(cast(window, OSControl), text);
}

/*---------------------------------------------------------------------------*/

void oswindow_edited(OSWindow *window, const bool_t is_edited)
{
    unref(window);
    unref(is_edited);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_movable(OSWindow *window, const bool_t is_movable)
{
    unref(window);
    unref(is_movable);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_z_order(OSWindow *window, OSWindow *below_window)
{
    HWND parent = NULL;
    cassert_no_null(window);
    cassert(FALSE);
    cassert(window->state != i_ekSTATE_MANAGED);

    if (below_window != NULL)
        parent = below_window->control.hwnd;

    SetWindowLongPtr(window->control.hwnd, GWLP_HWNDPARENT, (LONG_PTR)parent);
}

/*---------------------------------------------------------------------------*/

void oswindow_alpha(OSWindow *window, const real32_t alpha)
{
    unref(window);
    unref(alpha);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_enable_mouse_events(OSWindow *window, const bool_t enabled)
{
    unref(window);
    unref(enabled);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_hotkey(OSWindow *window, const vkey_t key, const uint32_t modifiers, Listener *listener)
{
    cassert_no_null(window);
    _oswindow_hotkey_set(&window->hotkeys, key, modifiers, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_taborder(OSWindow *window, OSControl *control)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    _ostabstop_list_add(&window->tabstop, control);
    if (control == NULL)
    {
        /* The window main panel has changed. We ensure that default button is still valid */
        window->tabstop.defbutton = _oswindow_apply_default_button(window, window->tabstop.defbutton);

        /* Force to show the focus rectangle in all controls */
        /* https://stackoverflow.com/questions/46489537/focus-rectangle-not-showing-even-if-control-has-focus */
        SendMessage(window->control.hwnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS | UISF_HIDEACCEL), (LPARAM)NULL);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_tabcycle(OSWindow *window, const bool_t cycle)
{
    cassert_no_null(window);
    window->tabstop.cycle = cycle;
}

/*---------------------------------------------------------------------------*/

gui_focus_t oswindow_tabstop(OSWindow *window, const bool_t next)
{
    cassert_no_null(window);
    if (next == TRUE)
        return _ostabstop_next(&window->tabstop, FALSE);
    else
        return _ostabstop_prev(&window->tabstop, FALSE);
}

/*---------------------------------------------------------------------------*/

gui_focus_t oswindow_focus(OSWindow *window, OSControl *control)
{
    cassert_no_null(window);
    cassert_no_null(control);
    cassert(window->state != i_ekSTATE_MANAGED);
    return _ostabstop_move(&window->tabstop, control);
}

/*---------------------------------------------------------------------------*/

OSControl *oswindow_get_focus(const OSWindow *window)
{
    cassert_no_null(window);
    return window->tabstop.current;
}

/*---------------------------------------------------------------------------*/

gui_tab_t oswindow_info_focus(const OSWindow *window, void **next_ctrl)
{
    cassert_no_null(window);
    return _ostabstop_info_focus(&window->tabstop, next_ctrl);
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_panel(OSWindow *window, OSPanel *panel)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    cassert(window->main_panel == NULL);
    _oscontrol_attach_to_parent(cast(panel, OSControl), cast(window, OSControl));
    window->main_panel = panel;
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_panel(OSWindow *window, OSPanel *panel)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    cassert(window->main_panel == panel);
    _oscontrol_detach_from_parent(cast(panel, OSControl), cast(window, OSControl));
    window->main_panel = NULL;
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_window(OSWindow *parent_window, OSWindow *child_window)
{
    unref(parent_window);
    unref(child_window);
    /*
    HWND prevParent = 0;
    prevParent = SetParent(child_window->control.hwnd, parent_window->control.hwnd);
    unref(prevParent);
    prevParent = GetParent(child_window->control.hwnd);
    // SetWindowLong(child_window->control.hwnd, GWL_STYLE, child_window->dwStyle | WS_CHILD);
    // SetWindowLong(child_window->control.hwnd, GWL_EXSTYLE, child_window->dwExStyle);
    // oswindow_set_z_order(child_window, parent_window);
    */
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_window(OSWindow *parent_window, OSWindow *child_window)
{
    unref(parent_window);
    unref(child_window);
    /*
    HWND prevParent = 0;
    cassert_no_null(parent_window);
    cassert_no_null(child_window);
    prevParent = SetParent(child_window->control.hwnd, GetDesktopWindow());
    cassert(prevParent == parent_window->control.hwnd);
    SetWindowLong(child_window->control.hwnd, GWL_STYLE, child_window->dwStyle);
    SetWindowLong(child_window->control.hwnd, GWL_EXSTYLE, child_window->dwExStyle);
    */
}

/*---------------------------------------------------------------------------*/

void oswindow_launch(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    if (!(window->flags & ekWINDOW_OFFSCREEN))
    {
        if (parent_window != NULL)
        {
            SetWindowPos(window->control.hwnd, parent_window->control.hwnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            window->role = ekGUI_ROLE_OVERLAY;
        }
        else
        {
            window->role = ekGUI_ROLE_MAIN;
        }

        window->launch_resize_event = FALSE;
        _oscontrol_set_visible(cast(window, OSControl), TRUE);
        window->launch_resize_event = TRUE;

        i_activate(window);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_hide(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    unref(parent_window);
    window->role = ENUM_MAX(gui_role_t);
    window->launch_resize_event = FALSE;
    _oscontrol_set_visible(cast(window, OSControl), FALSE);
    window->launch_resize_event = TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t oswindow_launch_modal(OSWindow *window, OSWindow *parent_window)
{
    uint32_t ret = UINT32_MAX;

    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);

    if (parent_window != NULL)
    {
        SetWindowLongPtr(window->control.hwnd, GWLP_HWNDPARENT, (LONG_PTR)parent_window->control.hwnd);
        _oscontrol_set_enabled(cast(parent_window, OSControl), FALSE);
    }

    window->role = ekGUI_ROLE_MODAL;
    window->launch_resize_event = FALSE;
    _oscontrol_set_visible(cast(window, OSControl), TRUE);
    window->launch_resize_event = TRUE;
    i_activate(window);

    /* Wait until the window is closed */
    ret = _oswindow_message_loop(window);

    if (parent_window != NULL)
    {
        _oscontrol_set_enabled(cast(parent_window, OSControl), TRUE);

        if (!(window->flags & ekWINDOW_MODAL_NOHIDE))
        {
            SetWindowLongPtr(window->control.hwnd, GWLP_HWNDPARENT, (LONG_PTR)GetDesktopWindow());
            SetWindowPos(parent_window->control.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        i_activate(parent_window);
    }

    return ret;
}

/*---------------------------------------------------------------------------*/

void oswindow_stop_modal(OSWindow *window, const uint32_t return_value)
{
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    SendMessage(window->control.hwnd, i_WM_MODAL_STOP, (WPARAM)return_value, (LPARAM)0);
    window->role = ENUM_MAX(gui_role_t);
}

/*---------------------------------------------------------------------------*/

void oswindow_get_origin(const OSWindow *window, real32_t *x, real32_t *y)
{
    cassert_no_null(window);
    cassert_no_null(x);
    cassert_no_null(y);
    /* The window top-left corner */
    if (*x == REAL32_MAX && *y == REAL32_MAX)
    {
        RECT rect;
        _osgui_frame_without_shadows(window->control.hwnd, &rect);
        *x = (real32_t)rect.left;
        *y = (real32_t)rect.top;
    }
    /* A window inner point (in client area coordinates) */
    else
    {
        POINT pt;
        pt.x = (LONG)*x;
        pt.y = (LONG)*y;
        ClientToScreen(window->control.hwnd, &pt);
        *x = (real32_t)pt.x;
        *y = (real32_t)pt.y;
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_origin(OSWindow *window, const real32_t x, const real32_t y)
{
    RECT rect1;
    RECT rect2;
    cassert_no_null(window);

    {
        BOOL ret = GetWindowRect(window->control.hwnd, &rect1);
        cassert_unref(ret != 0, ret);
    }

    _osgui_frame_without_shadows(window->control.hwnd, &rect2);

    {
        BOOL ret = SetWindowPos(window->control.hwnd, NULL, (int)x + (rect1.left - rect2.left), (int)y + (rect1.top - rect2.top), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        cassert_unref(ret != 0, ret);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_get_size(const OSWindow *window, real32_t *width, real32_t *height)
{
    RECT rect;
    cassert_no_null(window);
    cassert_no_null(width);
    cassert_no_null(height);
    _osgui_frame_without_shadows(window->control.hwnd, &rect);
    *width = (real32_t)(rect.right - rect.left);
    *height = (real32_t)(rect.bottom - rect.top);
}

/*---------------------------------------------------------------------------*/

void oswindow_size(OSWindow *window, const real32_t content_width, const real32_t content_height)
{
    BOOL ok = FALSE;
    LONG nwidth, nheight;
    cassert_no_null(window);
    cassert(window->state != i_ekSTATE_MANAGED);
    i_adjust_window_size(window->control.hwnd, (LONG)content_width, (LONG)content_height, window->dwStyle, window->dwExStyle, &nwidth, &nheight);
    window->launch_resize_event = FALSE;
    ok = SetWindowPos(window->control.hwnd, NULL, 0, 0, (int)nwidth, (int)nheight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    cassert_unref(ok != 0, ok);
    window->launch_resize_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void oswindow_set_default_pushbutton(OSWindow *window, OSButton *button)
{
    cassert_no_null(window);
    window->tabstop.defbutton = _oswindow_apply_default_button(window, button);
}

/*---------------------------------------------------------------------------*/

void oswindow_set_cursor(OSWindow *window, Cursor *cursor)
{
    cassert_no_null(window);
    window->cursor = (HCURSOR)cursor;
}

/*---------------------------------------------------------------------------*/

void oswindow_property(OSWindow *window, const gui_prop_t property, const void *value)
{
    cassert_no_null(window);
    unref(value);
    if (property == ekGUI_PROP_CHILDREN)
    {
        window->destroy_main_view = FALSE;
    }
}

/*---------------------------------------------------------------------------*/

void _oswindow_widget_set_focus(OSWindow *window, OSWidget *widget)
{
    unref(window);
    cassert_no_null(widget);
    SetFocus((HWND)widget);
}

/*---------------------------------------------------------------------------*/

static void i_get_controls(OSControl *control, ArrPt(OSControl) *controls)
{
    cassert_no_null(control);
    if (control->type == ekGUI_TYPE_PANEL)
    {
        ArrPt(OSControl) *children = _ospanel_children(cast(control, OSPanel));
        arrpt_foreach(child, children, OSControl)
            i_get_controls(child, controls);
        arrpt_end()
    }
    else if (control->type == ekGUI_TYPE_SPLITVIEW)
    {
        OSControl *child1 = NULL, *child2 = NULL;
        _ossplit_children(cast(control, OSSplit), &child1, &child2);
        if (child1 != NULL)
            i_get_controls(child1, controls);
        if (child2 != NULL)
            i_get_controls(child2, controls);
    }
    else
    {
        cassert(arrpt_find(controls, control, OSControl) == UINT32_MAX);
        arrpt_append(controls, control, OSControl);
    }
}

/*---------------------------------------------------------------------------*/

void _oswindow_find_all_controls(OSWindow *window, ArrPt(OSControl) *controls)
{
    cassert_no_null(window);
    cassert(arrpt_size(controls, OSControl) == 0);
    i_get_controls(cast(window->main_panel, OSControl), controls);
}

/*---------------------------------------------------------------------------*/

const ArrPt(OSControl) *_oswindow_get_all_controls(const OSWindow *window)
{
    cassert_no_null(window);
    return window->tabstop.controls;
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_app(void *app, void *icon)
{
    cassert(FALSE);
    unref(app);
    unref(icon);
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_app_terminate(void)
{
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void _oswindow_destroy_idles(void)
{
    arrpt_destopt(&i_IDLES, listener_destroy, Listener);
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_menubar(OSWindow *window, HMENU hmenu)
{
    BOOL ok = FALSE;
    cassert_no_null(window);
    cassert_no_null(hmenu);
    cassert(GetMenu(window->control.hwnd) == NULL);
    window->launch_resize_event = FALSE;
    ok = SetMenu(window->control.hwnd, hmenu);
    cassert_unref(ok == TRUE, ok);
    cassert(GetMenu(window->control.hwnd) == hmenu);
    window->launch_resize_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void _oswindow_unset_menubar(OSWindow *window, HMENU hmenu)
{
    BOOL ok = FALSE;
    cassert_no_null(window);
    cassert_unref(GetMenu(window->control.hwnd) == hmenu, hmenu);
    window->launch_resize_event = FALSE;
    ok = SetMenu(window->control.hwnd, NULL);
    cassert_unref(ok == TRUE, ok);
    cassert(GetMenu(window->control.hwnd) == NULL);
    window->launch_resize_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void _oswindow_change_menubar(OSWindow *window, HMENU prev_hmenu, HMENU new_hmenu)
{
    BOOL ok = FALSE;
    cassert_no_null(window);
    cassert_no_null(prev_hmenu);
    cassert_no_null(new_hmenu);
    window->launch_resize_event = FALSE;
    cassert(GetMenu(window->control.hwnd) == prev_hmenu);
    ok = SetMenu(window->control.hwnd, new_hmenu);
    cassert_unref(ok == TRUE, ok);
    cassert(GetMenu(window->control.hwnd) == new_hmenu);
    window->launch_resize_event = TRUE;
}

/*---------------------------------------------------------------------------*/

HWND _oswindow_set_current_popup_menu(OSWindow *window, HMENU hmenu)
{
    cassert_no_null(window);
    window->current_popup_menu = hmenu;
    return window->control.hwnd;
}

/*---------------------------------------------------------------------------*/

HWND _oswindow_hwnd(OSWindow *window)
{
    if (window != NULL)
        return window->control.hwnd;
    else
        return NULL;
}

/*---------------------------------------------------------------------------*/

void _oswindow_OnIdle(Listener *listener)
{
    if (i_IDLES == NULL)
        i_IDLES = arrpt_create(Listener);
    arrpt_append(i_IDLES, listener, Listener);
}

/*---------------------------------------------------------------------------*/

static OSWindow *i_get_window(HWND hwnd)
{
    OSWindow *window = NULL;
    while (window == NULL && hwnd != NULL)
    {
        window = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSWindow);
        if (window != NULL)
        {
            if (window->control.type != ekGUI_TYPE_WINDOW)
                window = NULL;
        }

        if (window == NULL)
            hwnd = GetParent(hwnd);
    }

    return window;
}

/*---------------------------------------------------------------------------*/

static BOOL i_IsDialogMessage(HWND hDlg, LPMSG lpMsg)
{
    OSWindow *window = NULL;
    cassert_no_null(lpMsg);
    window = i_get_window(lpMsg->hwnd);

    if (window != NULL)
    {
        if (lpMsg->message == WM_KEYDOWN)
        {
            if (lpMsg->wParam == VK_TAB)
            {
                if (window->control.hwnd == hDlg)
                {
                    if (_ostabstop_capture_tab(&window->tabstop) == FALSE)
                    {
                        SHORT lshif_state = GetAsyncKeyState(VK_LSHIFT);
                        SHORT rshif_state = GetAsyncKeyState(VK_RSHIFT);
                        BOOL previous = ((0x8000 & lshif_state) != 0) || ((0x8000 & rshif_state) != 0);
                        if (previous == TRUE)
                            _ostabstop_prev(&window->tabstop, TRUE);
                        else
                            _ostabstop_next(&window->tabstop, TRUE);
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
            }
            else if (lpMsg->wParam == VK_RETURN)
            {
                if (_ostabstop_capture_return(&window->tabstop) == FALSE)
                {
                    bool_t def = i_press_defbutton(window);

                    if (window->flags & ekWINDOW_RETURN)
                    {
                        i_close(window, ekGUI_CLOSE_INTRO);
                        return TRUE;
                    }

                    if (def == TRUE)
                        return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else if (lpMsg->wParam == VK_ESCAPE)
            {
                if (window->flags & ekWINDOW_ESC)
                {
                    i_close(window, ekGUI_CLOSE_ESC);
                    return TRUE;
                }
            }

            /* Check hotkeys */
            if (window->hotkeys != NULL)
            {
                vkey_t key = _osgui_vkey((WORD)lpMsg->wParam);
                uint32_t modifiers = _osgui_modifiers();
                if (_oswindow_hotkey_process(window, window->hotkeys, key, modifiers) == TRUE)
                    return TRUE;
            }
        }
        else if (lpMsg->message == WM_SETFOCUS)
        {
            /*
            arrpt_foreach(tabstop, window->tabstops, OSControl)
            {
                if (tabstop->hwnd == lpMsg->hwnd)
                {
                    window->ctabstop = tabstop;
                    break;
                }
            }
            arrpt_end()
            */
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_message(MSG *msg, HACCEL accelerator_table)
{
    cassert_no_null(msg);

    if (msg->message == WM_TIMER)
        return FALSE;

    if (i_CURRENT_ACTIVE_WINDOW != NULL)
    {
        BOOL accelerator_message = FALSE;

        if (accelerator_table != NULL)
            accelerator_message = TranslateAccelerator(i_CURRENT_ACTIVE_WINDOW, accelerator_table, msg);

        if (accelerator_message == TRUE)
        {
            return TRUE;
        }
        else
        {
            if (i_IsDialogMessage(i_CURRENT_ACTIVE_WINDOW, msg) != 0)
                return TRUE;
            else
                return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

uint32_t _oswindow_message_loop(OSWindow *window)
{
    MSG msg;
    for (;;)
    {
        BOOL doidle = FALSE;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            HACCEL accel = _osgui_accel_table();

            if (msg.message == WM_QUIT)
                return (uint32_t)msg.wParam;

            /* This is the message loop for a modal window */
            if (window != NULL)
            {
                /*
                   The active window in the app can change in Alt+Tab navigation.
                   Will this we ensure the correct message processing
                */
                if (GetActiveWindow() != window->control.hwnd)
                    i_activate(window);
            }

            if (i_message(cast(&msg, void), accel) == FALSE)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            doidle = TRUE;
        }

        if (doidle == TRUE)
        {
            if (i_IDLES != NULL)
            {
                if (arrpt_size(i_IDLES, Listener) > 0)
                {
                    Listener *idle = arrpt_first(i_IDLES, Listener);
                    arrpt_delete(i_IDLES, 0, NULL, Listener);
                    listener_event(idle, ekGUI_EVENT_IDLE, NULL, NULL, NULL, void, void, void);
                    listener_destroy(&idle);
                }
            }
        }
        else
        {
            /* Doesn't return until a message posted */
            WaitMessage();
        }
    }
}

/*---------------------------------------------------------------------------*/

static ___INLINE OSWindow *i_root(HWND hwnd)
{
    HWND root_hwnd = NULL;
    cassert_no_null(hwnd);
    root_hwnd = GetAncestor(hwnd, GA_ROOT);
    cassert_no_null(root_hwnd);
    return cast(GetWindowLongPtr(root_hwnd, GWLP_USERDATA), OSWindow);
}

/*---------------------------------------------------------------------------*/

bool_t _oswindow_mouse_down(OSControl *control)
{
    OSWindow *window = NULL;
    cassert_no_null(control);
    window = i_root(control->hwnd);
    cassert_no_null(window);
    return _ostabstop_mouse_down(&window->tabstop, control);
}

/*---------------------------------------------------------------------------*/

void _oswindow_release_transient_focus(OSControl *control)
{
    OSWindow *window = NULL;
    cassert_no_null(control);
    window = i_root(control->hwnd);
    cassert_no_null(window);
    _ostabstop_release_transient(&window->tabstop, control);
}
