/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscomwin.c
 *
 */

/* Operating System native common windows */

#include "oscontrol_win.inl"
#include "oswindow_win.inl"
#include "../oscomwin.h"
#include <draw2d/color.h>
#include <core/event.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

#include <sewer/nowarn.hxx>
#include <Windows.h>
#include <ShlObj.h>
#include <sewer/warn.hxx>

/*---------------------------------------------------------------------------*/

typedef struct _cdata_t CData;

struct _cdata_t
{
    LONG x;
    LONG y;
    align_t halign;
    align_t valign;
    WCHAR title[256];
};

/*---------------------------------------------------------------------------*/

#define CUSTOM_COLOR_SIZE 16
static char_t i_FILENAME[4 * MAX_PATH];

/*---------------------------------------------------------------------------*/

static int CALLBACK i_folder_browse(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    unref(lParam);
    unref(lpData);
    if (uMsg == BFFM_INITIALIZED)
    {
        SendMessage(hwnd, BFFM_SETSELECTION, 1, (LPARAM)i_FILENAME);
        return 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

const char_t *oscomwin_dir(OSWindow *parent, const char_t *caption, const char_t *start_dir)
{
    BROWSEINFO bi;
    TCHAR dir[MAX_PATH];
    TCHAR lcaption[STATIC_TEXT_SIZE];
    LPITEMIDLIST item;
    ZeroMemory(&bi, sizeof(bi));

    if (str_empty_c(caption) == FALSE)
    {
        unicode_convers(caption, cast(lcaption, char_t), ekUTF8, ekUTF16, sizeof(lcaption));
        bi.lpszTitle = lcaption;
    }
    else
    {
        bi.lpszTitle = NULL;
    }

    bi.hwndOwner = _oswindow_hwnd(parent);
    bi.pidlRoot = NULL;
    bi.pszDisplayName = dir;
    bi.ulFlags = 0;

    if (str_empty_c(start_dir) == FALSE)
    {
        uint32_t bytes = unicode_convers(start_dir, i_FILENAME, ekUTF8, ekUTF16, sizeof(i_FILENAME));
        cassert_unref(bytes < sizeof(i_FILENAME), bytes);
        bi.lpfn = i_folder_browse;
    }
    else
    {
        bi.lpfn = NULL;
    }

    bi.lParam = 0;
    bi.iImage = -1;

    item = SHBrowseForFolder(&bi);
    if (item != NULL)
    {
        uint32_t bytes;
        SHGetPathFromIDList(item, dir);
        bytes = unicode_convers(cast_const(dir, char_t), i_FILENAME, ekUTF16, ekUTF8, sizeof(i_FILENAME));
        cassert_unref(bytes < MAX_PATH, bytes);
        return i_FILENAME;
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

static void i_allowed_file_types(const char_t **ftypes, const uint32_t size, TCHAR *file_types, const uint32_t bufsize)
{
    uint32_t lbufsize = bufsize;
    cassert_no_null(file_types);
    file_types[0] = '\0';

    if (ftypes != NULL)
    {
        uint32_t i;
        for (i = 0; i < size; ++i)
        {
            TCHAR type[32];
            uint32_t tsize;
            tsize = unicode_convers(ftypes[i], cast(type, char_t), ekUTF8, ekUTF16, sizeof(type));
            cassert(tsize < sizeof(type));
            tsize += 4; /* "*." */
            if (lbufsize > tsize * 2)
            {
                int bytes;
                bytes = wsprintf(file_types, L"*.%s", type);
                file_types += bytes + 1;
                bytes = wsprintf(file_types, L"*.%s", type);
                file_types += bytes + 1;
                lbufsize -= tsize * 2;
            }
        }

        *file_types = '\0';
    }
    else
    {
        int bytes;
        bytes = wsprintf(file_types, L"*.*");
        file_types += bytes + 1;
        bytes = wsprintf(file_types, L"*.*");
        file_types += bytes + 1;
        *file_types = '\0';
    }
}

/*---------------------------------------------------------------------------*/

static void i_force_extension(WCHAR *file, INT buffer_size, const char_t *extension)
{
    WCHAR ext[32];
    WCHAR *file_ext = NULL;
    uint32_t ext_size;
    ext_size = unicode_convers(extension, cast(ext, char_t), ekUTF8, ekUTF16, sizeof(ext));
    cassert_unref(ext_size < sizeof(ext), ext_size);
    _wcslwr_s(ext, 32);

    {
        int i = 0;
        int last_dot = -1;
        while (file[i] != '\0')
        {
            if (file[i] == '.')
                last_dot = i;
            i++;
        }

        if (last_dot != -1)
            file_ext = file + last_dot + 1;
    }

    if (file_ext != NULL)
        file_ext[0] = '\0';
    else
        wcscat_s(file, (rsize_t)buffer_size, L".");

    wcscat_s(file, (rsize_t)buffer_size, ext);
}

/*---------------------------------------------------------------------------*/

const char_t *oscomwin_file(OSWindow *parent, const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir, const char_t *filename, const bool_t open)
{
    TCHAR file_types[256];
    OPENFILENAME ofn;
    TCHAR file[MAX_PATH];
    TCHAR lcaption[STATIC_TEXT_SIZE];
    BOOL ok;
    i_allowed_file_types(ftypes, size, file_types, sizeof(file_types));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = _oswindow_hwnd(parent);
    ofn.hInstance = NULL;
    ofn.lpstrFilter = file_types;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (str_empty_c(start_dir) == FALSE)
    {
        uint32_t bytes = unicode_convers(start_dir, i_FILENAME, ekUTF8, ekUTF16, sizeof(i_FILENAME));
        cassert_unref(bytes < sizeof(i_FILENAME), bytes);
        ofn.lpstrInitialDir = (LPCWSTR)i_FILENAME;
    }
    else
    {
        ofn.lpstrInitialDir = NULL;
    }

    if (str_empty_c(filename) == FALSE)
    {
        uint32_t bytes = unicode_convers(filename, cast(file, char_t), ekUTF8, ekUTF16, sizeof(file));
        cassert_unref(bytes < sizeof(file), bytes);
    }
    else
    {
        file[0] = '\0';
    }

    if (str_empty_c(caption) == FALSE)
    {
        unicode_convers(caption, cast(lcaption, char_t), ekUTF8, ekUTF16, sizeof(lcaption));
        ofn.lpstrTitle = lcaption;
    }
    else
    {
        ofn.lpstrTitle = NULL;
    }

    if (open == TRUE)
        ok = GetOpenFileName(&ofn);
    else
        ok = GetSaveFileName(&ofn);

    if (ok == TRUE)
    {
        uint32_t bytes;
        if (open == FALSE && size == 1)
            i_force_extension(file, MAX_PATH, ftypes[0]);

        bytes = unicode_convers(cast_const(file, char_t), i_FILENAME, ekUTF16, ekUTF8, sizeof(i_FILENAME));
        cassert_unref(bytes < MAX_PATH, bytes);
        return i_FILENAME;
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

static UINT_PTR CALLBACK i_color_msg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_INITDIALOG)
    {
        CHOOSECOLOR *col = cast(lParam, CHOOSECOLOR);
        CData *cdata = cast(col->lCustData, CData);
        RECT rect;
        BOOL ret = GetWindowRect(hwnd, &rect);
        int screen_width = GetSystemMetrics(SM_CXSCREEN);
        int screen_height = GetSystemMetrics(SM_CYSCREEN);
        cassert_unref(ret != 0, ret);

        if (cdata->halign != ekLEFT || cdata->valign != ekTOP)
        {
            switch (cdata->halign)
            {
            case ekLEFT:
            case ekJUSTIFY:
                break;
            case ekCENTER:
                cdata->x -= (rect.right - rect.left) / 2;
                break;
            case ekRIGHT:
                cdata->x -= rect.right - rect.left;
                break;
            default:
                cassert_default(cdata->halign);
            }

            switch (cdata->valign)
            {
            case ekTOP:
            case ekJUSTIFY:
                break;
            case ekCENTER:
                cdata->y -= (rect.bottom - rect.top) / 2;
                break;
            case ekRIGHT:
                cdata->y -= rect.bottom - rect.top;
                break;
            default:
                cassert_default(cdata->valign);
            }
        }

        /* Avoid to show the dialog outside the screen */
        if (cdata->x + (rect.right - rect.left) > screen_width)
            cdata->x = screen_width - (rect.right - rect.left);

        if (cdata->y + (rect.bottom - rect.top) > screen_height)
            cdata->y = screen_height - (rect.bottom - rect.top);

        SetWindowPos(hwnd, NULL, cdata->x, cdata->y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if (cdata->title[0] != 0)
            SetWindowText(hwnd, cdata->title);
    }

    unref(wParam);
    return 0;
}

/*---------------------------------------------------------------------------*/

void oscomwin_color(OSWindow *parent, const char_t *caption, const real32_t x, const real32_t y, const align_t halign, const align_t valign, const color_t current, color_t *colors, const uint32_t n, Listener *OnChange)
{
    CHOOSECOLOR col;
    COLORREF cols[CUSTOM_COLOR_SIZE];
    CData cdata;
    uint32_t i;

    col.lStructSize = sizeof(CHOOSECOLOR);
    col.hwndOwner = _oswindow_hwnd(parent);
    col.hInstance = NULL;
    col.rgbResult = _oscontrol_colorref(current);

    /* Custom colors expects 16 color-array */
    for (i = 0; i < CUSTOM_COLOR_SIZE; ++i)
    {
        if (i < n)
            cols[i] = _oscontrol_colorref(colors[i]);
        else
            cols[i] = _oscontrol_colorref(kCOLOR_WHITE);
    }

    cdata.x = (LONG)x;
    cdata.y = (LONG)y;
    cdata.halign = halign;
    cdata.valign = valign;

    if (str_empty_c(caption) == FALSE)
        unicode_convers(caption, cast(cdata.title, char_t), ekUTF8, ekUTF16, sizeof(cdata.title));
    else
        cdata.title[0] = 0;

    col.lpCustColors = cols;
    col.Flags = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
    col.lCustData = (LPARAM)&cdata;
    col.lpfnHook = i_color_msg;
    col.lpTemplateName = NULL;

    if (ChooseColor(&col) == TRUE)
    {
        color_t c = _oscontrol_from_colorref(col.rgbResult);
        listener_event(OnChange, ekGUI_EVENT_COLOR, NULL, &c, NULL, void, color_t, void);
    }

    /* Update color table */
    for (i = 0; i < n; ++i)
    {
        if (i < CUSTOM_COLOR_SIZE)
            colors[i] = _oscontrol_from_colorref(col.lpCustColors[i]);
        else
            colors[i] = kCOLOR_WHITE;
    }

    listener_destroy(&OnChange);
}
