/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
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
#include <osbs/dlib.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__LINUX__)
#error This file is only for Linux system
#endif

typedef void CURL;
typedef struct _curlslist_t CurlSList;
typedef struct _curlapi_t CurlApi;

typedef int (*FPtr_curl_global_init)(long flags);
typedef void (*FPtr_curl_global_cleanup)(void);
typedef CURL *(*FPtr_curl_easy_init)(void);
typedef void (*FPtr_curl_easy_cleanup)(CURL *curl);
typedef int (*FPtr_curl_easy_setopt)(CURL *curl, int option, ...);
typedef int (*FPtr_curl_easy_getinfo)(CURL *curl, int info, ...);
typedef int (*FPtr_curl_easy_perform)(CURL *curl);
typedef CurlSList *(*FPtr_curl_slist_append)(CurlSList *list, const char *data);
typedef void (*FPtr_curl_slist_free_all)(CurlSList *list);

struct _curlslist_t
{
    char *data;
    CurlSList *next;
};

struct _curlapi_t
{
    DLib *dlib;
    FPtr_curl_global_init global_init;
    FPtr_curl_global_cleanup global_cleanup;
    FPtr_curl_easy_init easy_init;
    FPtr_curl_easy_cleanup easy_cleanup;
    FPtr_curl_easy_setopt easy_setopt;
    FPtr_curl_easy_getinfo easy_getinfo;
    FPtr_curl_easy_perform easy_perform;
    FPtr_curl_slist_append slist_append;
    FPtr_curl_slist_free_all slist_free_all;
};

struct _oshttp_t
{
    CURL *curl;
    CurlSList *headers;
    String *host;
    bool_t secure;
    cookies_t cookies;
    Stream *resp_headers;
    Stream *resp_data;
    ierror_t error;
};

/*---------------------------------------------------------------------------*/

static const int i_CURLE_OK = 0;
static const long i_CURL_GLOBAL_ALL = 3;
static const int i_CURLOPT_PORT = 3;
static const int i_CURLOPT_FOLLOWLOCATION = 52;
static const int i_CURLOPT_POSTFIELDSIZE = 60;
static const int i_CURLOPT_WRITEDATA = 10001;
static const int i_CURLOPT_URL = 10002;
static const int i_CURLOPT_POSTFIELDS = 10015;
static const int i_CURLOPT_HTTPHEADER = 10023;
static const int i_CURLOPT_HEADERDATA = 10029;
static const int i_CURLOPT_COOKIEFILE = 10031;
static const int i_CURLOPT_CUSTOMREQUEST = 10036;
static const int i_CURLOPT_COOKIEJAR = 10082;
static const int i_CURLOPT_COOKIELIST = 10135;
static const int i_CURLOPT_WRITEFUNCTION = 20011;
static const int i_CURLOPT_HEADERFUNCTION = 20079;
static const int i_CURLINFO_COOKIELIST = 0x400000 + 28;
static CurlApi i_CURL;

/*---------------------------------------------------------------------------*/

#define COOKIE_DOMAIN 0
#define COOKIE_SUBDOMAINS 1
#define COOKIE_PATH 2
#define COOKIE_SECURE 3
#define COOKIE_TIMESTAMP 4
#define COOKIE_NAME 5
#define COOKIE_VALUE 6

/*---------------------------------------------------------------------------*/

static DLib *i_curl_dlib(void)
{
    DLib *dlib = dlib_open(NULL, "curl", "4");
    if (dlib == NULL)
        dlib = dlib_open(NULL, "curl-gnutls", "4");
    if (dlib == NULL)
        dlib = dlib_open(NULL, "curl", NULL);
    return dlib;
}

/*---------------------------------------------------------------------------*/

static bool_t i_curl_procs(CurlApi *api)
{
    cassert_no_null(api);
    api->global_init = dlib_proc(api->dlib, "curl_global_init", FPtr_curl_global_init);
    api->global_cleanup = dlib_proc(api->dlib, "curl_global_cleanup", FPtr_curl_global_cleanup);
    api->easy_init = dlib_proc(api->dlib, "curl_easy_init", FPtr_curl_easy_init);
    api->easy_cleanup = dlib_proc(api->dlib, "curl_easy_cleanup", FPtr_curl_easy_cleanup);
    api->easy_setopt = dlib_proc(api->dlib, "curl_easy_setopt", FPtr_curl_easy_setopt);
    api->easy_getinfo = dlib_proc(api->dlib, "curl_easy_getinfo", FPtr_curl_easy_getinfo);
    api->easy_perform = dlib_proc(api->dlib, "curl_easy_perform", FPtr_curl_easy_perform);
    api->slist_append = dlib_proc(api->dlib, "curl_slist_append", FPtr_curl_slist_append);
    api->slist_free_all = dlib_proc(api->dlib, "curl_slist_free_all", FPtr_curl_slist_free_all);
    if (api->global_init == NULL)
        return FALSE;
    if (api->global_cleanup == NULL)
        return FALSE;
    if (api->easy_init == NULL)
        return FALSE;
    if (api->easy_cleanup == NULL)
        return FALSE;
    if (api->easy_setopt == NULL)
        return FALSE;
    if (api->easy_getinfo == NULL)
        return FALSE;
    if (api->easy_perform == NULL)
        return FALSE;
    if (api->slist_append == NULL)
        return FALSE;
    if (api->slist_free_all == NULL)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

void oshttp_init(void)
{
    bmem_zero(&i_CURL, CurlApi);
    i_CURL.dlib = i_curl_dlib();
    if (i_CURL.dlib != NULL)
    {
        if (i_curl_procs(&i_CURL) == TRUE)
        {
            i_CURL.global_init(i_CURL_GLOBAL_ALL);
        }
        else
        {
            dlib_close(&i_CURL.dlib);
            bmem_zero(&i_CURL, CurlApi);
        }
    }
}

/*---------------------------------------------------------------------------*/

void oshttp_finish(void)
{
    if (i_CURL.dlib != NULL)
    {
        i_CURL.global_cleanup();
        dlib_close(&i_CURL.dlib);
        bmem_zero(&i_CURL, CurlApi);
    }
}

/*---------------------------------------------------------------------------*/

OSHttp *oshttp_create(const char_t *host, const uint16_t port, const bool_t secure)
{
    OSHttp *http = heap_new(OSHttp);
    if (i_CURL.dlib != NULL)
        http->curl = i_CURL.easy_init();
    else
        http->curl = NULL;
    http->error = ekIOK;
    http->cookies = ekCOOKIES_ALL;
    http->secure = secure;
    http->headers = NULL;
    http->resp_headers = NULL;
    http->resp_data = NULL;
    http->host = str_c(host);

    if (i_CURL.dlib == NULL)
    {
        http->error = ekINOIMPL;
    }
    else if (http->curl != NULL)
    {
        int res = i_CURL.easy_setopt(http->curl, i_CURLOPT_PORT, (long)port);
        cassert_unref(res == i_CURLE_OK, res);
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
        i_CURL.slist_free_all((*http)->headers);
        (*http)->headers = NULL;
    }

    str_destroy(&(*http)->host);

    if ((*http)->curl != NULL)
        i_CURL.easy_cleanup((*http)->curl);

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
        i_CURL.slist_free_all(http->headers);
        http->headers = NULL;
    }
}

/*---------------------------------------------------------------------------*/

bool_t oshttp_add_header(OSHttp *http, const char_t *name, const char_t *value)
{
    String *str = NULL;
    CurlSList *headers = NULL;
    cassert(http != NULL);

    if (i_CURL.dlib == NULL)
        return FALSE;

    str = str_printf("%s: %s", name, value);
    headers = i_CURL.slist_append(http->headers, tc(str));
    str_destroy(&str);

    if (headers == NULL)
        return FALSE;

    http->headers = headers;
    return TRUE;
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
    res = i_CURL.easy_setopt(http->curl, i_CURLOPT_FOLLOWLOCATION, auto_redirect ? 1L : 0L);
    cassert_unref(res == i_CURLE_OK, res);

    {
        String *url = str_printf("%s://%s%s", http->secure ? "https" : "http", tc(http->host), path);
        res = i_CURL.easy_setopt(http->curl, i_CURLOPT_URL, tc(url));
        cassert_unref(res == i_CURLE_OK, res);
        str_destroy(&url);
    }

    if (http->headers != NULL)
    {
        res = i_CURL.easy_setopt(http->curl, i_CURLOPT_HTTPHEADER, http->headers);
        cassert_unref(res == i_CURLE_OK, res);
    }

    if (data != NULL)
    {
        res = i_CURL.easy_setopt(http->curl, i_CURLOPT_POSTFIELDSIZE, (long)size);
        cassert_unref(res == i_CURLE_OK, res);
        res = i_CURL.easy_setopt(http->curl, i_CURLOPT_POSTFIELDS, cast(data, char));
        cassert_unref(res == i_CURLE_OK, res);
    }

    res = i_CURL.easy_setopt(http->curl, i_CURLOPT_CUSTOMREQUEST, verb);
    cassert_unref(res == i_CURLE_OK, res);

    switch (http->cookies)
    {
    case ekCOOKIES_OFF:
        /* Means, "don't read cookies" */
        i_CURL.easy_setopt(http->curl, i_CURLOPT_COOKIEFILE, NULL);
        /* Means, "don't write cookies" */
        i_CURL.easy_setopt(http->curl, i_CURLOPT_COOKIEJAR, NULL);
        /* Means, "remove in-memory cookies" */
        i_CURL.easy_setopt(http->curl, i_CURLOPT_COOKIELIST, "ALL");
        break;

    case ekCOOKIES_ALL:
    {
        String *cname = str_printf("%s_cookies.txt", tc(http->host));
        String *cfile = hfile_appdata(tc(cname));
        /* Means, "read cookies from this file" */
        i_CURL.easy_setopt(http->curl, i_CURLOPT_COOKIEFILE, tc(cfile));
        /* Means, "write cookies to this file" */
        i_CURL.easy_setopt(http->curl, i_CURLOPT_COOKIEJAR, tc(cfile));
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

    res = i_CURL.easy_setopt(http->curl, i_CURLOPT_HEADERFUNCTION, i_write_response);
    cassert(res == i_CURLE_OK);
    res = i_CURL.easy_setopt(http->curl, i_CURLOPT_HEADERDATA, http->resp_headers);
    cassert(res == i_CURLE_OK);

    res = i_CURL.easy_setopt(http->curl, i_CURLOPT_WRITEFUNCTION, i_write_response);
    cassert(res == i_CURLE_OK);
    res = i_CURL.easy_setopt(http->curl, i_CURLOPT_WRITEDATA, http->resp_data);
    cassert(res == i_CURLE_OK);

    res = i_CURL.easy_perform(http->curl);
    if (res == i_CURLE_OK)
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

    if (http->curl != NULL && http->cookies == ekCOOKIES_ALL)
    {
        CurlSList *cookies = NULL;
        CurlSList *nc = NULL;
        bool_t comma = FALSE;
        int res = 0;

        res = i_CURL.easy_getinfo(http->curl, i_CURLINFO_COOKIELIST, &cookies);
        cassert_unref(res == i_CURLE_OK, res);

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

        i_CURL.slist_free_all(cookies);
    }

    return stm;
}

/*---------------------------------------------------------------------------*/

void oshttp_cookie_delete(OSHttp *http, const char_t *name)
{
    CurlSList *cookies = NULL;
    CurlSList *nc = NULL;
    int res = 0;

    cassert_no_null(http);
    if (http->curl == NULL)
        return;

    res = i_CURL.easy_getinfo(http->curl, i_CURLINFO_COOKIELIST, &cookies);
    cassert_unref(res == i_CURLE_OK, res);

    for (nc = cookies; nc != NULL; nc = nc->next)
    {
        const char_t *fields[7];
        if (i_process_netscape_cookie(cast(nc->data, char_t), fields, sizeof(fields)) == TRUE)
        {
            if (str_equ_c(fields[COOKIE_NAME], name) == TRUE)
            {
                String *cdel = str_printf("%s\t%s\t%s\t%s\t0\t%s\t", fields[COOKIE_DOMAIN], fields[COOKIE_SUBDOMAINS], fields[COOKIE_PATH], fields[COOKIE_SECURE], fields[COOKIE_NAME]);
                res = i_CURL.easy_setopt(http->curl, i_CURLOPT_COOKIELIST, tc(cdel));
                cassert_unref(res == i_CURLE_OK, res);
                str_destroy(&cdel);
                break;
            }
        }
    }

    i_CURL.slist_free_all(cookies);
}
