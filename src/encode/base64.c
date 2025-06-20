/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
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

static char_t i_B64_CHR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

uint32_t b64_encoded_size(const uint32_t data_size)
{
    uint32_t size = 4 * (data_size / 3);
    return size + 4 + 1;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_decoded_size(const uint32_t encoded_size)
{
    uint32_t size = 3 * (encoded_size / 4);
    return size + 4;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_encode(const byte_t *data, const uint32_t size, char_t *base64, const uint32_t esize)
{
    uint32_t i = 0, j = 0, k = 0, s[3];
    cassert_no_null(data);
    cassert_no_null(base64);
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

    cassert_unref(k < esize, esize);
    base64[k] = '\0';
    return k;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_decode(const char_t *base64, const uint32_t size, byte_t *data)
{
    uint32_t i = 0, j = 0, k = 0, s[4];

    cassert_no_null(base64);
    cassert_no_null(data);

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

static String *i_encode_from_data(const byte_t *data, const uint32_t size)
{
    uint32_t b64size = b64_encoded_size(size);
    String *str = str_reserve(b64size);
    char_t *b64data = tcc(str);
    uint32_t n = b64_encode(data, size, b64data, b64size);
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
    uint32_t binsize = b64_decode(cast_const(data, char_t), size, binary);
    Buffer *buffer = buffer_with_data(binary, binsize);
    heap_free(&binary, dsize, "b64_decode");
    return buffer;
}
