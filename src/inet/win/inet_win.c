/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: inet_win.c
 *
 */

/* Inet Windows */

#include "inet_win.inl"
#include <core/heap.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

/*---------------------------------------------------------------------------*/

const WCHAR *wstring_init(const char_t *text, WString *str)
{
    WCHAR *wtext = NULL;
    cassert_no_null(str);
    str->nchars = 1 + unicode_nchars(text, ekUTF8);

    if (str->nchars < STATIC_TEXT_SIZE)
    {
        str->alloctext = NULL;
        wtext = str->statictext;
    }
    else
    {
        str->alloctext = cast(heap_malloc(str->nchars * sizeof(WCHAR), "WString_osdraw"), WCHAR);
        wtext = str->alloctext;
    }

    unicode_convers(text, cast(wtext, char_t), ekUTF8, ekUTF16, str->nchars * sizeof(WCHAR));
    return wtext;
}

/*---------------------------------------------------------------------------*/

void wstring_remove(WString *str)
{
    cassert_no_null(str);
    if (str->alloctext != NULL)
        heap_free(dcast(&str->alloctext, byte_t), str->nchars * sizeof(WCHAR), "WString_osdraw");
}
