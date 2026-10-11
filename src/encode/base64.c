/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: base64.c
 *
 */

/* Base64 encoding */

#include "base64.h"
#include <core/buffer.h>
#include <core/heap.h>
#include <core/hfile.h>
#include <core/stream.h>
#include <core/strings.h>
#include <sewer/cassert.h>

#define i_B64_MAX_ENCODED_SIZE ((uint64_t)UINT32_MAX - 1024ULL)

static char_t i_B64_CHR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*---------------------------------------------------------------------------*/

static bool_t i_b64_encoded_size(const uint32_t data_size, uint32_t *encoded_size)
{
    uint64_t size = 4ULL * ((uint64_t)data_size / 3ULL) + 5ULL;

    cassert_no_null(encoded_size);

    if (size > i_B64_MAX_ENCODED_SIZE)
    {
        *encoded_size = 0;
        return FALSE;
    }

    *encoded_size = (uint32_t)size;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_b64_required_size(const uint32_t data_size, uint32_t *required_size)
{
    uint64_t size = ((((uint64_t)data_size + 2ULL) / 3ULL) * 4ULL) + 1ULL;

    cassert_no_null(required_size);

    if (size > (uint64_t)UINT32_MAX)
    {
        *required_size = 0;
        return FALSE;
    }

    *required_size = (uint32_t)size;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_b64_int(const uint32_t ch)
{
    /* ASCII to base64_int
	65-90  Upper Case  >>  0-25
	97-122 Lower Case  >>  26-51
	48-57  Numbers     >>  52-61
	43     Plus (+)    >>  62
	47     Slash (/)   >>  63
	61     Equal (=)   >>  64~
    */
    if (ch == 43)
        return 62;
    if (ch == 47)
        return 63;
    if (ch == 61)
        return 64;
    if ((ch > 47) && (ch < 58))
        return ch + 4;
    if ((ch > 64) && (ch < 91))
        return ch - 'A';
    if ((ch > 96) && (ch < 123))
        return (ch - 'a') + 26;
    return 0;
}

/*---------------------------------------------------------------------------*/

static bool_t i_b64_decoded_bytes(const char_t *base64, const uint32_t size, uint32_t *decoded_size)
{
    uint32_t i = 0, j = 0, k = 0, s[4];

    cassert_no_null(decoded_size);
    *decoded_size = 0;

    if (base64 == NULL)
        return FALSE;

    for (i = 0; i < size; i++)
    {
        s[j++] = i_b64_int((uint32_t) * (base64 + i));
        if (j == 4)
        {
            if (s[2] == 64)
                k += 1;
            else if (s[3] == 64)
                k += 2;
            else
                k += 3;

            j = 0;
        }
    }

    *decoded_size = k;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_encoded_size(const uint32_t data_size)
{
    uint32_t size = 0;
    i_b64_encoded_size(data_size, &size);
    return size;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_decoded_size(const uint32_t encoded_size)
{
    uint32_t size = 3 * (encoded_size / 4);
    return size + 4;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_b64_encode(const byte_t *data, const uint32_t size, char_t *base64)
{
    uint32_t i = 0, j = 0, k = 0, s[3];
    for (i = 0; i < size; i++)
    {
        s[j++] = *(data + i);
        if (j == 3)
        {
            base64[k + 0] = i_B64_CHR[s[0] >> 2];
            base64[k + 1] = i_B64_CHR[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];
            base64[k + 2] = i_B64_CHR[((s[1] & 0x0F) << 2) + ((s[2] & 0xC0) >> 6)];
            base64[k + 3] = i_B64_CHR[s[2] & 0x3F];
            j = 0;
            k += 4;
        }
    }

    if (j > 0)
    {
        if (j == 1)
            s[1] = 0;

        base64[k + 0] = i_B64_CHR[s[0] >> 2];
        base64[k + 1] = i_B64_CHR[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];

        if (j == 2)
        {
            base64[k + 2] = i_B64_CHR[((s[1] & 0x0F) << 2)];
        }
        else
        {
            base64[k + 2] = '=';
        }

        base64[k + 3] = '=';
        k += 4;
    }

    base64[k] = '\0';
    return k;
}

/*---------------------------------------------------------------------------*/

bool_t b64_encode_ex(const byte_t *data, const uint32_t size, char_t *base64, const uint32_t esize, uint32_t *written)
{
    uint32_t required_size = 0;
    uint32_t n = 0;

    if (written != NULL)
        *written = 0;

    if (base64 == NULL || (data == NULL && size > 0))
        return FALSE;

    if (i_b64_required_size(size, &required_size) == FALSE || esize < required_size)
    {
        if (base64 != NULL && esize > 0)
            base64[0] = '\0';
        return FALSE;
    }

    n = i_b64_encode(data, size, base64);
    if (written != NULL)
        *written = n;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_encode(const byte_t *data, const uint32_t size, char_t *base64, const uint32_t esize)
{
    uint32_t required_size = 0;
    bool_t required_ok = FALSE;

    cassert_no_null(data);
    cassert_no_null(base64);
    required_ok = i_b64_required_size(size, &required_size);
    cassert(required_ok == TRUE);
    cassert_unref(required_size <= esize, esize);
    unref(required_ok);
    unref(required_size);

    return i_b64_encode(data, size, base64);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_b64_decode(const char_t *base64, const uint32_t size, byte_t *data)
{
    uint32_t i = 0, j = 0, k = 0, s[4];

    for (i = 0; i < size; i++)
    {
        s[j++] = i_b64_int((uint32_t) * (base64 + i));
        if (j == 4)
        {
            data[k + 0] = (byte_t)((s[0] << 2) + ((s[1] & 0x30) >> 4));

            if (s[2] != 64)
            {
                data[k + 1] = (byte_t)(((s[1] & 0x0F) << 4) + ((s[2] & 0x3C) >> 2));

                if ((s[3] != 64))
                {
                    data[k + 2] = (byte_t)(((s[2] & 0x03) << 6) + (s[3]));
                    k += 3;
                }
                else
                {
                    k += 2;
                }
            }
            else
            {
                k += 1;
            }

            j = 0;
        }
    }

    return k;
}

/*---------------------------------------------------------------------------*/

bool_t b64_decode_ex(const char_t *base64, const uint32_t size, byte_t *data, const uint32_t dsize, uint32_t *written)
{
    uint32_t k = 0;
    uint32_t required_size = 0;

    if (written != NULL)
        *written = 0;

    if (base64 == NULL || data == NULL)
        return FALSE;

    if (i_b64_decoded_bytes(base64, size, &required_size) == FALSE || required_size > dsize)
        return FALSE;

    k = i_b64_decode(base64, size, data);
    if (written != NULL)
        *written = k;

    return TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_decode(const char_t *base64, const uint32_t size, byte_t *data)
{
    cassert_no_null(base64);
    cassert_no_null(data);

    return i_b64_decode(base64, size, data);
}

/*---------------------------------------------------------------------------*/

static String *i_encode_from_data(const byte_t *data, const uint32_t size)
{
    uint32_t b64size = b64_encoded_size(size);
    String *str = NULL;
    char_t *b64data = NULL;
    uint32_t n = 0;

    if (b64size == 0)
        return str_c("");

    str = str_reserve(b64size);
    b64data = tcc(str);
    if (b64_encode_ex(data, size, b64data, b64size, &n) == FALSE)
        n = 0;
    b64data[n] = '\0';
    return str;
}

/*---------------------------------------------------------------------------*/

String *b64_encode_from_stm(Stream *stm)
{
    const byte_t *data = stm_buffer(stm);
    uint32_t size = stm_buffer_size(stm);
    return i_encode_from_data(data, size);
}

/*---------------------------------------------------------------------------*/

String *b64_encode_from_file(const char_t *pathname, ferror_t *error)
{
    Stream *stm = hfile_stream(pathname, error);
    String *str = NULL;
    if (stm != NULL)
    {
        str = b64_encode_from_stm(stm);
        stm_close(&stm);
    }

    if (str == NULL)
        str = str_c("");

    return str;
}

/*---------------------------------------------------------------------------*/

String *b64_encode_from_data(const byte_t *data, const uint32_t size)
{
    return i_encode_from_data(data, size);
}

/*---------------------------------------------------------------------------*/

String *b64_encode_from_str(const String *str)
{
    const byte_t *data = cast_const(tc(str), byte_t);
    uint32_t size = str_len(str);
    return i_encode_from_data(data, size);
}

/*---------------------------------------------------------------------------*/

String *b64_encode_from_cstr(const char_t *str)
{
    const byte_t *data = cast_const(str, byte_t);
    uint32_t size = str_len_c(str);
    return i_encode_from_data(data, size);
}

/*---------------------------------------------------------------------------*/

Buffer *b64_decode_from_str(const String *base64)
{
    const byte_t *data = cast_const(tc(base64), byte_t);
    uint32_t len = str_len_c(tc(base64));
    return b64_decode_from_data(data, len);
}

/*---------------------------------------------------------------------------*/

Buffer *b64_decode_from_data(const byte_t *data, const uint32_t size)
{
    uint32_t dsize = b64_decoded_size(size);
    byte_t *binary = heap_malloc(dsize, "b64_decode");
    uint32_t binsize = 0;
    Buffer *buffer = NULL;

    b64_decode_ex(cast_const(data, char_t), size, binary, dsize, &binsize);
    buffer = buffer_with_data(binary, binsize);
    heap_free(&binary, dsize, "b64_decode");
    return buffer;
}
