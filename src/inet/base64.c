/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: base64.c
 *
 */

/* Base64 encoding */

#include "base64.h"
#include "cassert.h"

static char_t i_B64_CHR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*---------------------------------------------------------------------------*/

static uint32_t i_b64_int(const uint32_t ch) 
{
	// ASCII to base64_int
	// 65-90  Upper Case  >>  0-25
	// 97-122 Lower Case  >>  26-51
	// 48-57  Numbers     >>  52-61
	// 43     Plus (+)    >>  62
	// 47     Slash (/)   >>  63
	// 61     Equal (=)   >>  64~
	if (ch==43)
	    return 62;
	if (ch==47)
	    return 63;
	if (ch==61)
	    return 64;
	if ((ch>47) && (ch<58))
	    return ch + 4;
	if ((ch>64) && (ch<91))
	    return ch - 'A';
	if ((ch>96) && (ch<123))
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
	for (i=0; i < size; i++) 
    {
		s[j++] = *(data + i);
		if (j == 3) 
        {
			base64[k+0] = i_B64_CHR[s[0] >> 2];
			base64[k+1] = i_B64_CHR[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];
			base64[k+2] = i_B64_CHR[((s[1] & 0x0F) << 2) + ((s[2] & 0xC0) >> 6)];
			base64[k+3] = i_B64_CHR[s[2] & 0x3F];
			j = 0; k += 4;
		}
	}
	
	if (j > 0) 
    {
		if (j == 1)
			s[1] = 0;

		base64[k+0] = i_B64_CHR[s[0] >> 2];
		base64[k+1] = i_B64_CHR[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];

        if (j == 2)
        {
            base64[k + 2] = i_B64_CHR[((s[1] & 0x0F) << 2)];
        }
        else
        {
            //cassert(j == 3);
            base64[k + 2] = '=';
        }

		base64[k+3] = '=';
		k += 4;
	}

	cassert_unref(k < esize, esize);
	base64[k] = '\0';	
	return k;
}

/*---------------------------------------------------------------------------*/

uint32_t b64_decode(const char_t* base64, const uint32_t size, byte_t *data)
{
	uint32_t i = 0, j = 0, k = 0, s[4];

    cassert_no_null(base64);
    cassert_no_null(data);

	for (i = 0; i < size; i++) 
    {
		s[j++] = i_b64_int((uint32_t)*(base64 + i));
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

//#define WHITESPACE 64
//#define EQUALS     65
//#define INVALID    66
//
//static const unsigned char d[] = {
//    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
//    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
//    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
//    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
//    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
//    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
//    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
//    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
//    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
//    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
//    66,66,66,66,66,66
//};
//
//static int base64decode (char *in, size_t inLen, unsigned char *out, size_t *outLen) { 
//    char *end = in + inLen;
//    char iter = 0;
//    uint32_t buf = 0;
//    size_t len = 0;
//    
//    while (in < end) {
//        unsigned char c = d[*in++];
//        
//        switch (c) {
//        case WHITESPACE: continue;   /* skip whitespace */
//        case INVALID:    return 1;   /* invalid input, return error */
//        case EQUALS:                 /* pad character, end of data */
//            in = end;
//            continue;
//        default:
//            buf = buf << 6 | c;
//            iter++; // increment the number of iteration
//            /* If the buffer is full, split it into bytes */
//            if (iter == 4) {
//                if ((len += 3) > *outLen) return 1; /* buffer overflow */
//                *(out++) = (buf >> 16) & 255;
//                *(out++) = (buf >> 8) & 255;
//                *(out++) = buf & 255;
//                buf = 0; iter = 0;
//
//            }   
//        }
//    }
//   
//    if (iter == 3) {
//        if ((len += 2) > *outLen) return 1; /* buffer overflow */
//        *(out++) = (buf >> 10) & 255;
//        *(out++) = (buf >> 2) & 255;
//    }
//    else if (iter == 2) {
//        if (++len > *outLen) return 1; /* buffer overflow */
//        *(out++) = (buf >> 4) & 255;
//    }
//
//    *outLen = len; /* modify to reflect the actual output size */
//    return 0;
//}
//
//
//uint32_t b64_decode(const char_t* base64, const uint32_t size, byte_t *data)
//{
//    size_t outlen;
//    base64decode((char*)base64, size, (unsigned char*)data, &outlen);
//    return (uint32_t)outlen;
//}

