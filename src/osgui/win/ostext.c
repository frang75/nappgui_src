/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.c
 *
 */

/* Operating System native text view */

#include "ostext.h"
#include "ostext.inl"
#include "osgui.inl"
#include "osgui_win.inl"
#include "oscontrol.inl"
#include "ospanel.inl"
#include "oswindow.inl"
#include "cassert.h"
#include "color.h"
#include "event.h"
#include "font.h"
#include "heap.h"
#include "ptr.h"
#include "stream.h"
#include "unicode.h"

#include "nowarn.hxx"
#include <Richedit.h>
#include "warn.hxx"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _ostext_t 
{
    OSControl control;
    uint32_t units;
	DWORD dwEffects;
	LONG yHeight;
    COLORREF crTextColor;
    COLORREF crBackColor;
	WORD wAlignment;
    LONG dyLineSpacing;
    LONG dySpaceBefore;
    LONG dySpaceAfter;
	WCHAR szFaceName[LF_FACESIZE];
    char_t *text;
    uint32_t text_size;
    bool_t is_editable;
    RECT border;
    Listener *OnChange;
};

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSText *view = (OSText*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    cassert_no_null(view);  

    switch (uMsg) {
    case WM_NCCALCSIZE:
    {
        LRESULT res = CallWindowProc(view->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);

        //if (TRUE)
            res = _osgui_nccalcsize(hwnd, wParam, lParam, TRUE, &view->border);

        return res;
    }

    case WM_NCPAINT:
    {
        LRESULT res = CallWindowProc(view->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);

        //if (TRUE)
            res = _osgui_ncpaint(hwnd, &view->border);

        return res;
    }

    case WM_SETFOCUS:
        RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
        break;

    case WM_KILLFOCUS:
        RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
        break;

    case WM_PAINT:
        if (_oswindow_in_resizing(hwnd) == TRUE)
            return 0;
        break;
    }

    return CallWindowProc(view->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

static void i_set_editable(HWND hwnd, const BOOL is_editable)
{
    LRESULT res = 0;
    res = SendMessage(hwnd, EM_SETREADONLY, (WPARAM)!is_editable, (LPARAM)0);
    cassert(res != 0);
}

///*---------------------------------------------------------------------------*/
//
//static void i_set_font(HWND hwnd, const TCHAR *font_name, LONG size)
//{
//    CHARFORMAT2 format;
//    LRESULT res;
//    format.cbSize = sizeof(CHARFORMAT2);
//    format.dwMask = CFM_SIZE | CFM_BOLD;
//
//    if (font_name != NULL)
//    {
//        format.dwMask |= CFM_FACE;
//        wcscpy_s(format.szFaceName, LF_FACESIZE, font_name);
//    }
//
//    format.dwEffects = 0;
//    /* OJO!!!! Quitar el kTWIPS_PER_PIXEL */
//    format.yHeight = size * kTWIPS_PER_PIXEL;
//    res = SendMessage(hwnd, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&format);
//    cassert(res != 0);
//}

/*---------------------------------------------------------------------------*/

static void i_set_rich_text(HWND hwnd, const BOOL rich_text)
{
    DWORD flags;
    LRESULT res;
    flags = (rich_text == TRUE) ? TM_RICHTEXT : TM_PLAINTEXT;
    res = SendMessage(hwnd, EM_SETTEXTMODE, flags, 0); 
    cassert(res == 0);
}

/*---------------------------------------------------------------------------*/

OSText *ostext_create(const uint32_t flags)
{
    OSText *view = NULL;
    DWORD dwStyle = 0;
    unref(flags);
    view = heap_new0(OSText);
    view->control.type = ekGUI_TYPE_TEXTVIEW;
    dwStyle = WS_CHILD | WS_CLIPSIBLINGS | ES_MULTILINE | ES_WANTRETURN /*| ES_AUTOVSCROLL*/ | WS_VSCROLL /*| WS_HSCROLL*/;
    _oscontrol_init((OSControl*)view, PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY), dwStyle, kRICHEDIT_CLASS, 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    i_set_rich_text(view->control.hwnd, TRUE);
    i_set_editable(view->control.hwnd, FALSE);
    view->dyLineSpacing = 20;
	return view;
}

/*---------------------------------------------------------------------------*/

void ostext_destroy(OSText **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    listener_destroy(&(*view)->OnChange);

    if ((*view)->text != NULL)
        heap_free((byte_t**)&(*view)->text, (*view)->text_size, "OSTextText");

    _oscontrol_destroy((OSControl*)*view);
    heap_delete(view, OSText);
}

/*---------------------------------------------------------------------------*/

void ostext_OnTextChange(OSText *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnChange, listener);

    if (listener != NULL)
        SendMessage(view->control.hwnd, EM_SETEVENTMASK, (WPARAM)0, ENM_CHANGE);
    else
        SendMessage(view->control.hwnd, EM_SETEVENTMASK, (WPARAM)0, ENM_NONE);
}

/*---------------------------------------------------------------------------*/

static void i_add_text(OSText *view, CHARRANGE *cr, const char_t *text)
{
    uint32_t num_bytes = 0;
    WCHAR *wtext_alloc = NULL;
    WCHAR wtext_static[WCHAR_BUFFER_SIZE];
    WCHAR *wtext;
    CHARFORMAT2 format;
    PARAFORMAT2 pformat;
    LRESULT res = 0;

    num_bytes = unicode_convers_nbytes(text, ekUTF8, kWINDOWS_UNICODE);
    if (num_bytes < sizeof(wtext_static))
    {
        wtext = wtext_static; 
    }
    else
    {
        wtext_alloc = (WCHAR*)heap_malloc(num_bytes, "OSTextAddText");
        wtext = wtext_alloc;
    }

    {
        register uint32_t bytes = unicode_convers(text, (char_t*)wtext, ekUTF8, kWINDOWS_UNICODE, num_bytes);
        cassert_unref(bytes == num_bytes, bytes);
    }

    SendMessage(view->control.hwnd, EM_EXSETSEL, 0, (LPARAM)cr);

    format.cbSize = sizeof(CHARFORMAT2);
    format.dwMask = CFM_FACE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_SUBSCRIPT | CFM_SIZE | CFM_COLOR | CFM_BACKCOLOR;
    format.dwEffects = view->dwEffects;
    format.yHeight = view->yHeight;

    if (view->crTextColor == 0)
        format.dwEffects |= CFE_AUTOCOLOR;
    else
        format.crTextColor = view->crTextColor;

    if (view->crBackColor == 0)
        format.dwEffects |= CFE_AUTOBACKCOLOR;
    else
        format.crBackColor = view->crBackColor;

    wcscpy_s(format.szFaceName, sizeof(format.szFaceName) / sizeof(WCHAR), view->szFaceName);
    res = SendMessage(view->control.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&format);
    cassert_unref(res != 0, res);

    pformat.cbSize = sizeof(PARAFORMAT2);    
    pformat.dwMask = PFM_ALIGNMENT | PFM_LINESPACING | PFM_SPACEAFTER | PFM_SPACEBEFORE;
    pformat.wAlignment = view->wAlignment;
    pformat.bLineSpacingRule = 5;
    pformat.dyLineSpacing = view->dyLineSpacing;
    pformat.dySpaceBefore = view->dySpaceBefore;
    pformat.dySpaceAfter = view->dySpaceAfter;
    res = SendMessage(view->control.hwnd, EM_SETPARAFORMAT, 0, (LPARAM)&pformat);
    cassert_unref(res != 0, res);

    SendMessage(view->control.hwnd, EM_REPLACESEL, 0, (LPARAM)wtext);

    if (wtext_alloc != NULL)
        heap_free((byte_t**)&wtext_alloc, num_bytes, "OSTextAddText");
}

/*---------------------------------------------------------------------------*/

void ostext_insert_text(OSText *view, const char_t *text)
{
    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    i_add_text(view, &cr, text);
}

/*---------------------------------------------------------------------------*/

void ostext_set_text(OSText *view, const char_t *text)
{
    CHARRANGE cr;
    cr.cpMin = 0;
    cr.cpMax = -1;
    i_add_text(view, &cr, text);
}

/*---------------------------------------------------------------------------*/

static DWORD CALLBACK i_set_rtf(DWORD_PTR dwCookie, LPBYTE lpBuff, LONG cb, PLONG pcb)
{
    Stream *stm = (Stream*)dwCookie;
    *pcb = stm_read(stm, (byte_t*)lpBuff, (uint32_t)cb);
    return 0;
}

/*---------------------------------------------------------------------------*/

void ostext_set_rtf(OSText *view, Stream *rtf_in)
{
    EDITSTREAM es = { 0 };
	cassert_no_null(view);
    //i_set_rich_text(view->control.hwnd, TRUE);        
    es.pfnCallback = i_set_rtf;
    es.dwCookie = (DWORD_PTR)rtf_in;        
    SendMessage(view->control.hwnd, EM_STREAMIN, SF_RTF, (LPARAM)&es);
}

/*---------------------------------------------------------------------------*/

//void ostext_font(OSText *view, const Font *font);
//void ostext_font(OSText *view, const Font *font)
//{
//    const char_t *family = font_family(font);
//    real32_t size = font_size(font);
//    uint32_t style = font_style(font);
//
//    cassert_no_null(view);
//
//    if (style & ekFPOINTS)
//        size = size * (real32_t)kLOG_PIXY / 72.f;
//
//    view->yHeight = (LONG)(size * kTWIPS_PER_PIXEL);
//    
//    view->dwEffects = 0;
//    if (style & ekFBOLD)
//        view->dwEffects |= CFE_BOLD;
//
//    if (style & ekFITALIC)
//        view->dwEffects |= CFE_ITALIC;
//
//    if (style & ekFSTRIKEOUT)
//        view->dwEffects |= CFE_STRIKEOUT;
//
//    if (style & ekFUNDERLINE)
//        view->dwEffects |= CFE_UNDERLINE;
//
//    unicode_convers(family, (char_t*)view->szFaceName, ekUTF8, ekUTF16, sizeof(view->szFaceName));
//}

/*---------------------------------------------------------------------------*/

void ostext_property(OSText *view, const gui_prop_t prop, const void *value)
{
    cassert_no_null(view);
    cassert_no_null(value);
    switch (prop) {
    case ekGUI_PROP_FAMILY:
        unicode_convers((const char_t*)value, (char_t*)view->szFaceName, ekUTF8, ekUTF16, sizeof(view->szFaceName));
        break;

    case ekGUI_PROP_UNITS:
        view->units = *((const uint32_t*)value);
        break;

    case ekGUI_PROP_SIZE:
    {
        real32_t size = *((real32_t*)value);
        if (view->units & ekFPOINTS)
            size = size * (real32_t)kLOG_PIXY_GUI / 72.f;
        view->yHeight = (LONG)(size * kTWIPS_PER_PIXEL_GUI);
        break;
    }

    case ekGUI_PROP_STYLE:
    {
        uint32_t style = *((uint32_t*)value);
        view->dwEffects = 0;
        if (style & ekFBOLD)
            view->dwEffects |= CFE_BOLD;

        if (style & ekFITALIC)
            view->dwEffects |= CFE_ITALIC;

        if (style & ekFSTRIKEOUT)
            view->dwEffects |= CFE_STRIKEOUT;

        if (style & ekFUNDERLINE)
            view->dwEffects |= CFE_UNDERLINE;

        if (style & ekFSUBSCRIPT)
            view->dwEffects |= CFE_SUBSCRIPT;

        if (style & ekFSUPSCRIPT)
            view->dwEffects |= CFE_SUPERSCRIPT;

        break;
    }

    case ekGUI_PROP_COLOR:
        if (*((color_t*)value) == kCOLOR_TRANSPARENT)
            view->crTextColor = 0;
        else
            view->crTextColor = _oscontrol_colorref(*((color_t*)value));
        break;

    case ekGUI_PROP_BGCOLOR:
        if (*((color_t*)value) == kCOLOR_TRANSPARENT)
            view->crBackColor = 0;
        else
            view->crBackColor = _oscontrol_colorref(*((color_t*)value));
        break;

    case ekGUI_PROP_PGCOLOR:
        if (*((color_t*)value) == kCOLOR_TRANSPARENT)
            SendMessage(view->control.hwnd, EM_SETBKGNDCOLOR, 1, (LPARAM)0);
        else
            SendMessage(view->control.hwnd, EM_SETBKGNDCOLOR, 0, (LPARAM)_oscontrol_colorref(*((color_t*)value)));
        break;

    case ekGUI_PROP_PARALIGN:
        switch (*((align_t*)value)) {
        case ekLEFT:
        case ekJUSTIFY:
            view->wAlignment = PFA_LEFT;
            break;
        case ekCENTER:
            view->wAlignment = PFA_CENTER;
            break;
        case ekRIGHT:
            view->wAlignment = PFA_RIGHT;
            break;
        }
        break;

    case ekGUI_PROP_LSPACING:
        view->dyLineSpacing = (LONG)(20 * *((real32_t*)value));
        break;

    case ekGUI_PROP_AFPARSPACE:
        view->dySpaceAfter = (LONG)(20/*kTWIPS_PER_PIXEL*/ * *((real32_t*)value) /** (real32_t)kLOG_PIXY / 72.f*/);
        break;

    case ekGUI_PROP_BFPARSPACE:
        view->dySpaceBefore = (LONG)(20/*kTWIPS_PER_PIXEL*/ * *((real32_t*)value) /** (real32_t)kLOG_PIXY / 72.f*/);
        break;

    case ekGUI_PROP_VSCROLL:
    {
        real32_t pos = *(real32_t*)value;
        WPARAM wParam = SB_THUMBPOSITION;
        if (pos <= 0)
            wParam = SB_TOP;
        else if (pos >= 1e10f)
            wParam = SB_BOTTOM;
        SendMessage(view->control.hwnd, WM_VSCROLL, wParam, (LPARAM)NULL);
        break;
    }

    case ekGUI_PROP_RESIZE:
    case ekGUI_PROP_CHILDREN:
    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void ostext_editable(OSText *view, const bool_t is_editable)
{
	cassert_no_null(view);
    view->is_editable = is_editable;
    i_set_editable(view->control.hwnd, (BOOL)is_editable);
}

/*---------------------------------------------------------------------------*/

const char_t *ostext_get_text(const OSText *view)
{
    uint32_t num_chars = 0;
    WCHAR *wtext_alloc = NULL;
    WCHAR wtext_static[WCHAR_BUFFER_SIZE];
    WCHAR *wtext;
	cassert_no_null(view);
    if (view->text != NULL)
        heap_free((byte_t**)&view->text, view->text_size, "OSTextText");

    {
        GETTEXTLENGTHEX textl;
        textl.flags = GTL_DEFAULT;
        textl.codepage = 1200;
        num_chars = 1 + (uint32_t)SendMessage(view->control.hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&textl, (LPARAM)0);
    }

    if (num_chars < WCHAR_BUFFER_SIZE)
    {
        wtext = wtext_static;
    }
    else
    {
        wtext_alloc = (WCHAR*)heap_malloc(num_chars * sizeof(WCHAR), "OSTextGetText");
        wtext = wtext_alloc;
    }

    {
        GETTEXTEX gtext;
        uint32_t num_charsw;
        gtext.cb = num_chars * sizeof(WCHAR);
        gtext.flags = GT_DEFAULT;
        gtext.codepage = 1200;
        gtext.lpDefaultChar = NULL;
        gtext.lpUsedDefChar = NULL;
        /* EM_GETTEXTEX: The return value is the number of TCHARs copied into the output buffer, including the null terminator. */
        num_charsw = (uint32_t)SendMessage(view->control.hwnd, EM_GETTEXTEX, (WPARAM)&gtext, (LPARAM)wtext);
        cassert(num_chars == num_charsw + 1);
    }

    ((OSText*)view)->text_size = unicode_convers_nbytes((const char_t*)wtext, kWINDOWS_UNICODE, ekUTF8);
    ((OSText*)view)->text = (char_t*)heap_malloc(view->text_size, "OSTextText");

    {
        register uint32_t bytes = unicode_convers((const char_t*)wtext, view->text, kWINDOWS_UNICODE, ekUTF8, view->text_size);
        cassert_unref(bytes == view->text_size, bytes);
    }

    if (wtext_alloc != NULL)
        heap_free((byte_t**)&wtext_alloc, num_chars * sizeof(WCHAR), "OSTextGetText");

    return view->text;
}

/*---------------------------------------------------------------------------*/

void ostext_set_need_display(OSText *view)
{
	unref(view);
	cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void ostext_attach(OSText *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ostext_detach(OSText *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ostext_visible(OSText *view, const bool_t visible)
{
    _oscontrol_set_visible((OSControl*)view, visible);
}

/*---------------------------------------------------------------------------*/

void ostext_enabled(OSText *view, const bool_t enabled)
{
    _oscontrol_set_enabled((OSControl*)view, enabled);
}

/*---------------------------------------------------------------------------*/

void ostext_size(const OSText *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void ostext_origin(const OSText *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void ostext_frame(OSText *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl*)view, x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _ostext_detach_and_destroy(OSText **view, OSPanel *panel)
{
    cassert_no_null(view);
    ostext_detach(*view, panel);
    ostext_destroy(view);
}

/*---------------------------------------------------------------------------*/

void _ostext_command(OSText *view, WPARAM wParam)
{
    cassert_no_null(view);
    if (HIWORD(wParam) == EN_CHANGE)
    {
        if (/*edit->launch_change_event == TRUE 
            && */IsWindowEnabled(view->control.hwnd) 
            && view->OnChange != NULL)
        {
            //char_t *edit_text;
            //uint32_t tsize;
            EvText params;
            //edit_text = _oscontrol_get_text((const OSControl*)edit, &tsize);
            params.text = NULL;
            params.cpos = 0;//i_cursor_pos(edit->control.hwnd);
            listener_event(view->OnChange, ekGUI_EVENT_TXTCHANGE, view, &params, NULL, OSText, EvText, void);
            /*heap_deletes(&edit_text, tsize, char_t);
            SendMessage(edit->control.hwnd, EM_SETSEL, (WPARAM)params.cursor_pos, (LPARAM)params.cursor_pos);
            edit->is_editing = TRUE;*/
        }
    }
/*    else if (HIWORD(wParam) == EN_KILLFOCUS)
    {
        if (edit->launch_change_event == TRUE 
            && edit->is_editing == TRUE
            && IsWindowEnabled(edit->control.hwnd) 
            && edit->OnTextEndEditing_listener.object != NULL)
        {
            char_t *edit_text;
            uint32_t tsize;
            Event event;
            EvText params;
            edit_text = _oscontrol_get_text((const OSControl*)edit, &tsize);
            event.type = ekGUI_EVENT_TEXTEDIT_END;
            event.sender = edit;
            event.params = &params;
            event.result = NULL;
            params.text = (const char_t*)edit_text;
            listener_event(&edit->OnTextEndEditing_listener, &event);
            heap_deletes(&edit_text, tsize, char_t);
        }

        edit->is_editing = FALSE;
    }*/
}
