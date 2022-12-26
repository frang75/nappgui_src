/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tfilter.c
 *
 */

/* Text filters */

#include "tfilter.inl"
#include "cassert.h"
#include "unicode.h"

typedef enum _state_t
{
    stSTART,
    stZERO,
    stINTEGER,
    stREAL
} gui_state_t;

/*---------------------------------------------------------------------------*/

void tfilter_number(const char_t *src, char_t *dest, const uint32_t size, const uint32_t num_decimals, const bool_t allow_negatives)
{
    uint32_t csize, i = 0;
    uint32_t codepoint = unicode_to_u32b(src, ekUTF8, &csize);
    uint32_t decimals = 0;
    gui_state_t state = stSTART;
    bool_t valid = FALSE;
    while (codepoint != 0)
    {
        valid = FALSE;
        switch (state) {
        case stSTART:
            if (codepoint == '0')
            {
                state = stZERO;
                valid = TRUE;
            }
            else if (codepoint >= '1' && codepoint <= '9')
            {
                state = stINTEGER;
                valid = TRUE;
            }
            else if (codepoint == '.' || codepoint == ',')
            {
                if (num_decimals > 0)
                {
                    state = stREAL;
                    valid = TRUE;
                }
            }
            else if (codepoint == '-')
            {
                if (allow_negatives == TRUE)
                {
                    state = stSTART;
                    valid = TRUE;
                }
            }
            break;

        case stZERO:
            if (num_decimals > 0)
            {
                if (codepoint == '.' || codepoint == ',')
                {
                    state = stREAL;
                    valid = TRUE;
                }
            }
            break;

        case stINTEGER:
            if (codepoint >= '0' && codepoint <= '9')
            {
                state = stINTEGER;
                valid = TRUE;
            }
            else if (codepoint == '.' || codepoint == ',')
            {
                if (num_decimals > 0)
                {
                    state = stREAL;
                    valid = TRUE;
                }
            }
            break;

        case stREAL:
            if (codepoint >= '0' && codepoint <= '9')
            {
                if (decimals < num_decimals)
                {
                    decimals += 1;
                    state = stREAL;
                    valid = TRUE;
                }
            }
            break;

            cassert_default();
        }

        if (valid == TRUE)
        {
            if (i + csize < size - 1)
            {
                dest += unicode_to_char(codepoint, dest, ekUTF8);
                i += csize;
            }
        }

        src = unicode_next(src, ekUTF8);
        codepoint = unicode_to_u32(src, ekUTF8);
    }

    unicode_to_char(0, dest, ekUTF8);
}
