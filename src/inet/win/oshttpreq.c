/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oshttpreq.c
 *
 */

/* HTTP request (WinINet-based implementation) */

#include "../oshttpreq.inl"
#include "inet_win.inl"
#include <core/arrst.h>
#include <core/heap.h>
#include <core/stream.h>
#include <core/strings.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

#if !defined(__WINDOWS__)
#error This file is for Windows system
#endif

#include <sewer/nowarn.hxx>
#include <Windows.h>
#include <WinInet.h>
#include <sewer/warn.hxx>

#define INTERNET_SUPPRESS_COOKIE_PERSIST 0x00000003
#define INTERNET_SUPPRESS_COOKIE_PERSIST_RESET 0x00000004

struct _oshttp_t
{
    HINTERNET hInternet;
    HINTERNET hConnect;
    HINTERNET hRequest;
    ierror_t error;
    bool_t secure;
    cookies_t cookies;
    String *url;
    Stream *headers;
};

/*---------------------------------------------------------------------------*/

void oshttp_init(void)
{
}

/*---------------------------------------------------------------------------*/

void oshttp_finish(void)
{
}

/*---------------------------------------------------------------------------*/

OSHttp *oshttp_create(const char_t *host, const uint16_t port, const bool_t secure)
{
    OSHttp *http = heap_new0(OSHttp);

    http->secure = secure;
    http->error = ekIOK;
    http->cookies = ekCOOKIES_ALL;
    http->hInternet = InternetOpen(L"nappgui", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    http->url = str_printf("%s://%s", secure == TRUE ? "https" : "http", host);
    http->headers = stm_memory(1024);
    stm_set_write_utf(http->headers, ekUTF16);

    if (http->hInternet != NULL)
    {
        WCHAR whost[128];
        unicode_convers(host, cast(whost, char_t), ekUTF8, ekUTF16, sizeof(whost));
        http->hConnect = InternetConnect(http->hInternet, whost, (INTERNET_PORT)port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

        if (http->hConnect == NULL)
            http->error = ekINOHOST;
    }
    else
    {
        http->error = ekINONET;
    }

    return http;
}

/*---------------------------------------------------------------------------*/

void oshttp_destroy(OSHttp **http)
{
    cassert_no_null(http);
    cassert_no_null(*http);

    if ((*http)->hRequest != NULL)
        InternetCloseHandle((*http)->hRequest);

    if ((*http)->hConnect != NULL)
        InternetCloseHandle((*http)->hConnect);

    if ((*http)->hInternet != NULL)
        InternetCloseHandle((*http)->hInternet);

    str_destroy(&(*http)->url);
    stm_close(&(*http)->headers);
    heap_delete(http, OSHttp);
}

/*---------------------------------------------------------------------------*/

void oshttp_clear_headers(OSHttp *http)
{
    cassert_no_null(http);
    stm_close(&http->headers);
    http->headers = stm_memory(1024);
    stm_set_write_utf(http->headers, ekUTF16);
}

/*---------------------------------------------------------------------------*/

void oshttp_add_header(OSHttp *http, const char_t *name, const char_t *value)
{
    cassert_no_null(http);

    if (stm_bytes_written(http->headers) > 0)
        stm_writef(http->headers, "\n");

    stm_writef(http->headers, name);
    stm_writef(http->headers, ": ");
    stm_writef(http->headers, value);
}

/*---------------------------------------------------------------------------*/

void oshttp_cookies_policy(OSHttp *http, const cookies_t cookies)
{
    cassert_no_null(http);
    http->cookies = cookies;
}

/*---------------------------------------------------------------------------*/

static void i_request(OSHttp *http, const WCHAR *verb, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    WCHAR wpath[1024];
    uint64_t hsize = 0;
    BOOL status = FALSE;
    DWORD flags = 0;

    cassert_no_null(http);

    if (http->error != ekIOK)
    {
        ptr_assign(error, http->error);
        return;
    }

    unicode_convers(path, cast(wpath, char_t), ekUTF8, ekUTF16, sizeof(wpath));

    if (http->hRequest != NULL)
    {
        InternetCloseHandle(http->hRequest);
        http->hRequest = NULL;
    }

    flags |= http->secure ? INTERNET_FLAG_SECURE : 0;
    flags |= auto_redirect ? 0 : INTERNET_FLAG_NO_AUTO_REDIRECT;

    switch (http->cookies)
    {
    case ekCOOKIES_OFF:
        flags |= INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_RELOAD;
        break;

    case ekCOOKIES_ALL:
    {
        DWORD opt = INTERNET_SUPPRESS_COOKIE_PERSIST_RESET;
        BOOL ok = InternetSetOption(http->hInternet, INTERNET_OPTION_SUPPRESS_BEHAVIOR, &opt, sizeof(DWORD));
        cassert_unref(ok == TRUE, ok);
        break;
    }

    default:
        cassert_default(http->cookies);
    }

    http->hRequest = HttpOpenRequest(http->hConnect, verb, wpath, L"HTTP/1.1", NULL, NULL, flags, 0);
    if (http->hRequest == NULL)
    {
        ptr_assign(error, ekISERVER);
        return;
    }

    hsize = stm_bytes_written(http->headers);
    if (hsize > 0)
    {
        WCHAR *lpszHeaders = cast(stm_buffer(http->headers), WCHAR);
        status = HttpSendRequest(http->hRequest, lpszHeaders, (DWORD)hsize / sizeof(WCHAR), (LPVOID)data, (DWORD)size);
    }
    else
    {
        status = HttpSendRequest(http->hRequest, NULL, (DWORD)-1, (LPVOID)data, (DWORD)size);
    }

    if (status == TRUE)
    {
        ptr_assign(error, ekIOK);
    }
    else
    {
        ptr_assign(error, ekISERVER);
        InternetCloseHandle(http->hRequest);
        http->hRequest = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void oshttp_get(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, L"GET", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_post(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, L"POST", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_put(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, L"PUT", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_patch(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, L"PATCH", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_delete(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, L"DELETE", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

Stream *oshttp_response(OSHttp *http)
{
    cassert_no_null(http);
    if (http->hRequest != NULL)
    {
        uint32_t i = 0;
        DWORD asize = 512;
        DWORD size = 512;
        for (i = 0; i < 2; ++i)
        {
            WCHAR *data = cast(heap_malloc((uint32_t)asize, "http_headers"), WCHAR);

            if (HttpQueryInfo(http->hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)data, &size, NULL) == TRUE)
            {
                Stream *stm_src = stm_from_block(cast_const(data, byte_t), (uint32_t)size);
                Stream *stm_dest = stm_memory(size + 10);

                stm_set_read_utf(stm_src, ekUTF16);
                stm_set_write_utf(stm_dest, ekUTF8);

                stm_lines(line, stm_src)
                    stm_writef(stm_dest, line);
                    stm_write_char(stm_dest, 13); /* CR '\r' */
                    stm_write_char(stm_dest, 10); /* LF '\n' */
                stm_next(line, stm_src)

                heap_free(dcast(&data, byte_t), (uint32_t)asize, "http_headers");
                stm_close(&stm_src);
                return stm_dest;
            }
            else
            {
                heap_free(dcast(&data, byte_t), (uint32_t)asize, "http_headers");
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    asize = size;
                }
                else
                {
                    cassert(FALSE);
                }
            }
        }
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void oshttp_response_body(OSHttp *http, Stream *body, ierror_t *error)
{
    cassert_no_null(http);
    if (http->hRequest != NULL)
    {
        char szBuffer[1024];
        DWORD dwByteRead = 0;

        do
        {
            if (InternetReadFile(http->hRequest, szBuffer, sizeof(szBuffer), &dwByteRead) == TRUE)
            {
                stm_write(body, cast_const(szBuffer, byte_t), (uint32_t)dwByteRead);
            }
            else
            {
                ptr_assign(error, ekISTREAM);
                return;
            }

        } while (dwByteRead);
    }

    ptr_assign(error, ekIOK);
}

/*---------------------------------------------------------------------------*/

Stream *oshttp_cookies(OSHttp *http)
{
    WString str;
    Stream *stm = NULL;
    const WCHAR *url = NULL;
    TCHAR *buffer = NULL;
    DWORD size = 0;
    BOOL ok;
    cassert_no_null(http);
    url = wstring_init(tc(http->url), &str);
    ok = InternetGetCookieEx(url, NULL, NULL, &size, INTERNET_COOKIE_HTTPONLY, NULL);
    cassert_unref((ok == TRUE && size > 0) || (ok == FALSE && size == 0), ok);
    /* Size in bytes (not in TCHARs) */
    size /= sizeof(TCHAR);

    if (size > 0)
    {
        buffer = heap_new_n(size, TCHAR);
        /* Here size in TCHARs */
        ok = InternetGetCookieEx(url, NULL, buffer, &size, INTERNET_COOKIE_HTTPONLY, NULL);
        cassert_unref(ok == TRUE, ok);
    }

    if (buffer != NULL)
    {
        uint32_t csize = unicode_convers_nbytes(cast_const(buffer, char_t), ekUTF16, ekUTF8);
        char_t *cbuffer = heap_new_n(csize, char_t);
        unicode_convers(cast_const(buffer, char_t), cbuffer, ekUTF16, ekUTF8, csize);
        stm = stm_memory(1024);
        stm_writef(stm, cbuffer);
        heap_delete_n(&buffer, size, TCHAR);
        heap_delete_n(&cbuffer, csize, char_t);
    }

    wstring_remove(&str);
    return stm;
}

/*---------------------------------------------------------------------------*/

void oshttp_cookie_delete(OSHttp *http, const char_t *name)
{
    WString str1;
    WString str2;
    DWORD state;
    const WCHAR *url = NULL;
    const WCHAR *wname = NULL;
    cassert_no_null(http);
    url = wstring_init(tc(http->url), &str1);
    wname = wstring_init(name, &str2);
    state = InternetSetCookieEx(url, wname, L"deleted; expires=Thu, 01-Jan-1970 00:00:00 GMT", INTERNET_COOKIE_HTTPONLY, 0);
    cassert_unref(state != FALSE, state);
    wstring_remove(&str1);
    wstring_remove(&str2);
}
