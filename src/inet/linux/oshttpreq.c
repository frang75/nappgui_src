/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oshttpreq.c
 *
 */

/* HTTP request (LibCURL-based implementation) */

#include "../oshttpreq.inl"
#include <core/arrpt.h>
#include <core/heap.h>
#include <core/hfile.h>
#include <core/stream.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <curl/curl.h>

#if !defined(__LINUX__)
#error This file is only for Linux system
#endif

struct _oshttp_t
{
    CURL *curl;
    struct curl_slist *headers;
    String *host;
    bool_t secure;
    cookies_t cookies;
    Stream *resp_headers;
    Stream *resp_data;
    ierror_t error;
};

/*---------------------------------------------------------------------------*/

#define COOKIE_DOMAIN 0
#define COOKIE_SUBDOMAINS 1
#define COOKIE_PATH 2
#define COOKIE_SECURE 3
#define COOKIE_TIMESTAMP 4
#define COOKIE_NAME 5
#define COOKIE_VALUE 6

/*---------------------------------------------------------------------------*/

void oshttp_init(void)
{
    curl_global_init(CURL_GLOBAL_ALL);
}

/*---------------------------------------------------------------------------*/

void oshttp_finish(void)
{
    curl_global_cleanup();
}

/*---------------------------------------------------------------------------*/

OSHttp *oshttp_create(const char_t *host, const uint16_t port, const bool_t secure)
{
    OSHttp *http = heap_new(OSHttp);
    http->curl = curl_easy_init();
    http->error = ekIOK;
    http->cookies = ekCOOKIES_ALL;
    http->secure = secure;
    http->headers = NULL;
    http->resp_headers = NULL;
    http->resp_data = NULL;
    http->host = str_c(host);

    if (http->curl != NULL)
    {
        int res = curl_easy_setopt(http->curl, CURLOPT_PORT, (long)port);
        cassert_unref(res == CURLE_OK, res);
    }
    else
    {
        http->error = ekIUNDEF;
    }

    return http;
}

/*---------------------------------------------------------------------------*/

void oshttp_destroy(OSHttp **http)
{
    cassert_no_null(http);
    cassert_no_null(*http);

    if ((*http)->headers != NULL)
    {
        curl_slist_free_all((*http)->headers);
        (*http)->headers = NULL;
    }

    str_destroy(&(*http)->host);

    if ((*http)->curl != NULL)
        curl_easy_cleanup((*http)->curl);

    if ((*http)->resp_headers != NULL)
        stm_close(&(*http)->resp_headers);

    if ((*http)->resp_data != NULL)
        stm_close(&(*http)->resp_data);

    heap_delete(http, OSHttp);
}

/*---------------------------------------------------------------------------*/

void oshttp_clear_headers(OSHttp *http)
{
    cassert_no_null(http);
    if (http->headers != NULL)
    {
        curl_slist_free_all(http->headers);
        http->headers = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void oshttp_add_header(OSHttp *http, const char_t *name, const char_t *value)
{
    String *str = NULL;
    cassert(http != NULL);
    str = str_printf("%s: %s", name, value);
    http->headers = curl_slist_append(http->headers, tc(str));
    str_destroy(&str);
}

/*---------------------------------------------------------------------------*/

void oshttp_cookies_policy(OSHttp *http, const cookies_t cookies)
{
    cassert_no_null(http);
    http->cookies = cookies;
}

/*---------------------------------------------------------------------------*/

static size_t i_write_response(char *buffer, size_t size, size_t nitems, void *userdata)
{
    Stream *stm = (Stream *)userdata;
    stm_write(stm, (const byte_t *)buffer, (uint32_t)(size * nitems));
    return nitems * size;
}

/*---------------------------------------------------------------------------*/

static void i_request(OSHttp *http, const char *verb, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    int res = 0;
    cassert_no_null(http);

    if (http->error != ekIOK)
    {
        ptr_assign(error, http->error);
        return;
    }

    /* Seems that CURLOPT_FOLLOWLOCATION fails */
    res = curl_easy_setopt(http->curl, CURLOPT_FOLLOWLOCATION, auto_redirect ? 1L : 0L);
    cassert_unref(res == CURLE_OK, res);

    {
        String *url = str_printf("%s://%s%s", http->secure ? "https" : "http", tc(http->host), path);
        res = curl_easy_setopt(http->curl, CURLOPT_URL, tc(url));
        cassert_unref(res == CURLE_OK, res);
        str_destroy(&url);
    }

    if (http->headers != NULL)
    {
        res = curl_easy_setopt(http->curl, CURLOPT_HTTPHEADER, http->headers);
        cassert_unref(res == CURLE_OK, res);
    }

    if (data != NULL)
    {
        res = curl_easy_setopt(http->curl, CURLOPT_POSTFIELDSIZE, (long)size);
        cassert_unref(res == CURLE_OK, res);
        res = curl_easy_setopt(http->curl, CURLOPT_POSTFIELDS, cast(data, char));
        cassert_unref(res == CURLE_OK, res);
    }

    res = curl_easy_setopt(http->curl, CURLOPT_CUSTOMREQUEST, verb);
    cassert_unref(res == CURLE_OK, res);

    switch (http->cookies)
    {
    case ekCOOKIES_OFF:
        /* Means, "don't read cookies" */
        curl_easy_setopt(http->curl, CURLOPT_COOKIEFILE, NULL);
        /* Means, "don't write cookies" */
        curl_easy_setopt(http->curl, CURLOPT_COOKIEJAR, NULL);
        /* Means, "remove in-memory cookies" */
        curl_easy_setopt(http->curl, CURLOPT_COOKIELIST, "ALL");
        break;

    case ekCOOKIES_ALL:
    {
        String *cname = str_printf("%s_cookies.txt", tc(http->host));
        String *cfile = hfile_appdata(tc(cname));
        /* Means, "read cookies from this file" */
        curl_easy_setopt(http->curl, CURLOPT_COOKIEFILE, tc(cfile));
        /* Means, "write cookies to this file" */
        curl_easy_setopt(http->curl, CURLOPT_COOKIEJAR, tc(cfile));
        str_destroy(&cname);
        str_destroy(&cfile);
        break;
    }
    default:
        cassert_default(http->cookies);
    }

    if (http->resp_headers != NULL)
        stm_close(&http->resp_headers);

    if (http->resp_data != NULL)
        stm_close(&http->resp_data);

    http->resp_headers = stm_memory(100 * 1024);
    http->resp_data = stm_memory(1024 * 1024);

    res = curl_easy_setopt(http->curl, CURLOPT_HEADERFUNCTION, i_write_response);
    cassert(res == CURLE_OK);
    res = curl_easy_setopt(http->curl, CURLOPT_HEADERDATA, http->resp_headers);
    cassert(res == CURLE_OK);

    res = curl_easy_setopt(http->curl, CURLOPT_WRITEFUNCTION, i_write_response);
    cassert(res == CURLE_OK);
    res = curl_easy_setopt(http->curl, CURLOPT_WRITEDATA, http->resp_data);
    cassert(res == CURLE_OK);

    res = curl_easy_perform(http->curl);
    if (res == CURLE_OK)
    {
        ptr_assign(error, ekIOK);
    }
    else
    {
        /* TODO: Error codes */
        ptr_assign(error, ekISERVER);
    }
}

/*---------------------------------------------------------------------------*/

void oshttp_get(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, "GET", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_post(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, "POST", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_put(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, "PUT", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_patch(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, "PATCH", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

void oshttp_delete(OSHttp *http, const char_t *path, const byte_t *data, const uint32_t size, const bool_t auto_redirect, ierror_t *error)
{
    i_request(http, "DELETE", path, data, size, auto_redirect, error);
}

/*---------------------------------------------------------------------------*/

Stream *oshttp_response(OSHttp *http)
{
    Stream *stm = NULL;
    cassert_no_null(http);
    stm = http->resp_headers;
    http->resp_headers = NULL;
    return stm;
}

/*---------------------------------------------------------------------------*/

void oshttp_response_body(OSHttp *http, Stream *body, ierror_t *error)
{
    const byte_t *data = NULL;
    uint32_t size = 0;
    cassert_no_null(http);
    data = stm_buffer(http->resp_data);
    size = stm_buffer_size(http->resp_data);
    stm_write(body, data, size);
    stm_close(&http->resp_data);
    ptr_assign(error, ekIOK);
}

/*---------------------------------------------------------------------------*/
/*
* Cookie in Netscape format (\t as separator)
* domain (eg: example.com or #HttpOnly_example.com)
* include_subdomains TRUE or FALSE
* path (eg: /)
* secure TRUE if only HTTPS, FALSE if not
* expiration Timestamp UNIX (seconds) or 0
* name
* value
*/
static bool_t i_process_netscape_cookie(char_t *data, const char_t **fields, const uint32_t size)
{
    uint32_t i = 0;
    bool_t store = TRUE;
    cassert_no_null(data);
    cassert_no_null(fields);
    while (*data != 0)
    {
        if (store == TRUE)
        {
            if (i < size)
                fields[i] = data;
            i += 1;
            store = FALSE;
        }

        if (*data == '\t')
        {
            *data = 0;
            store = TRUE;
        }

        data += 1;
    }

    return (bool_t)(i == 7);
}

/*---------------------------------------------------------------------------*/

Stream *oshttp_cookies(OSHttp *http)
{
    Stream *stm = NULL;
    cassert_no_null(http);

    if (http->cookies == ekCOOKIES_ALL)
    {
        struct curl_slist *cookies = NULL;
        struct curl_slist *nc = NULL;
        bool_t comma = FALSE;
        int res = 0;

        res = curl_easy_getinfo(http->curl, CURLINFO_COOKIELIST, &cookies);
        cassert_unref(res == CURLE_OK, res);

        if (cookies != NULL)
            stm = stm_memory(1024);

        for (nc = cookies; nc != NULL; nc = nc->next)
        {
            const char_t *fields[7];
            if (i_process_netscape_cookie(cast(nc->data, char_t), fields, sizeof(fields)) == TRUE)
            {
                if (comma == TRUE)
                    stm_writef(stm, "; ");
                else
                    comma = TRUE;

                stm_writef(stm, fields[COOKIE_NAME]);
                stm_writef(stm, "=");
                stm_writef(stm, fields[COOKIE_VALUE]);
            }
        }

        curl_slist_free_all(cookies);
    }

    return stm;
}

/*---------------------------------------------------------------------------*/

void oshttp_cookie_delete(OSHttp *http, const char_t *name)
{
    struct curl_slist *cookies = NULL;
    struct curl_slist *nc = NULL;
    int res = 0;

    cassert_no_null(http);
    res = curl_easy_getinfo(http->curl, CURLINFO_COOKIELIST, &cookies);
    cassert_unref(res == CURLE_OK, res);

    for (nc = cookies; nc != NULL; nc = nc->next)
    {
        const char_t *fields[7];
        if (i_process_netscape_cookie(cast(nc->data, char_t), fields, sizeof(fields)) == TRUE)
        {
            if (str_equ_c(fields[COOKIE_NAME], name) == TRUE)
            {
                String *cdel = str_printf("%s\t%s\t%s\t%s\t0\t%s\t", fields[COOKIE_DOMAIN], fields[COOKIE_SUBDOMAINS], fields[COOKIE_PATH], fields[COOKIE_SECURE], fields[COOKIE_NAME]);
                res = curl_easy_setopt(http->curl, CURLOPT_COOKIELIST, tc(cdel));
                cassert_unref(res == CURLE_OK, res);
                str_destroy(&cdel);
                break;
            }
        }
    }

    curl_slist_free_all(cookies);
}
