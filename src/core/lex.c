/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: lex.c
 *
 */

/* Lexical scanner */

#include "lex.inl"
#include "stream.inl"
#include "heap.h"
#include "stream.h"
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

#define DISCARD_CHAR 0
#define VALID_CHAR 1
#define STORE_CHAR 2

typedef struct _lexopts_t LexOpts;

typedef enum _state_t
{
    stSTART,
    stSPACES,
    stSLASH,
    stSL_COMMENT,
    stML_COMMENT,
    stML_COMMENT2,
    stPERIOD,
    stIDENTIFIER,
    stSTRING,
    stSTRING_SCAPE,
    stSTRING_OCTAL,
    stSTRING_HEX,
    stSTRING_UNICODE4,
    stSTRING_UNICODE8,
    stSTRING_SURR1,
    stSTRING_SURR2,
    stZERO,
    stINT,
    stOCTAL,
    stOCTAL89,
    stHEX0,
    stHEX,
    stREAL,
    stEXP,
    stEXPSIGN,
    stEXP1,
    stNUMERR,
    stEND
} gui_state_t;

struct _lexopts_t
{
    bool_t spaces;
    bool_t newlines;
    bool_t escapes;
    bool_t comments;
    bool_t octal;
};

struct _lexscn_t
{
    LexOpts opts;
    uint32_t pcol;
    uint32_t prow;
    uint32_t tcol;
    uint32_t trow;
    uint32_t lexsize;
    uint32_t lexi;
    char_t *lexeme;
    uint32_t esci;
    uint32_t escval;
    uint32_t escsurr;
    uint32_t escnbytes;
    uint32_t escneed;
    byte_t escbytes[4];
};

/*---------------------------------------------------------------------------*/

LexScn *_lexscn_create(void)
{
    LexScn *lex = heap_new0(LexScn);
    lex->lexsize = 256;
    lex->lexeme = cast(heap_malloc(lex->lexsize, "LexLexeme"), char_t);
    return lex;
}

/*---------------------------------------------------------------------------*/

void _lexscn_destroy(LexScn **lex)
{
    cassert_no_null(lex);
    cassert_no_null(*lex);
    heap_free(dcast(&(*lex)->lexeme, byte_t), (*lex)->lexsize, "LexLexeme");
    heap_delete(lex, LexScn);
}

/*---------------------------------------------------------------------------*/

void _lexscn_spaces(LexScn *lex, const bool_t activate)
{
    cassert_no_null(lex);
    lex->opts.spaces = activate;
}

/*---------------------------------------------------------------------------*/

void _lexscn_newlines(LexScn *lex, const bool_t activate)
{
    cassert_no_null(lex);
    lex->opts.newlines = activate;
}

/*---------------------------------------------------------------------------*/

void _lexscn_escapes(LexScn *lex, const bool_t activate)
{
    cassert_no_null(lex);
    lex->opts.escapes = activate;
}

/*---------------------------------------------------------------------------*/

void _lexscn_comments(LexScn *lex, const bool_t activate)
{
    cassert_no_null(lex);
    lex->opts.comments = activate;
}

/*---------------------------------------------------------------------------*/

void _lexscn_octal(LexScn *lex, const bool_t activate)
{
    cassert_no_null(lex);
    lex->opts.octal = activate;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_get_char(LexScn *lex, Stream *stm)
{
    uint32_t col = stm_col(stm);
    uint32_t row = stm_row(stm);
    uint32_t code = stm_read_char(stm);
    cassert_no_null(lex);

    if (__TRUE_EXPECTED(code != UINT32_MAX))
    {
        lex->pcol = col;
        lex->prow = row;
    }

    return code;
}

/*---------------------------------------------------------------------------*/

static void i_store_char(LexScn *lex, Stream *stm, const uint32_t code)
{
    char_t data[5];
    uint32_t size;
    cassert_no_null(lex);
    size = unicode_to_char(code, data, ekUTF8);
    cassert(size >= 1 && size <= 4);
    _stm_restore(stm, cast_const(data, byte_t), size);
    _stm_restore_col(stm, lex->pcol);
    _stm_restore_row(stm, lex->prow);
}

/*---------------------------------------------------------------------------*/

static void i_put_code(LexScn *lex, const uint32_t code)
{
    uint32_t csize;
    cassert_no_null(lex);

    if (code == UINT32_MAX)
        return;

    if (lex->lexi + 5 > lex->lexsize)
    {
        lex->lexeme = cast(heap_realloc(cast(lex->lexeme, byte_t), lex->lexsize, lex->lexsize * 2, "LexLexeme"), char_t);
        lex->lexsize *= 2;
    }

    csize = unicode_to_char(code, lex->lexeme + lex->lexi, ekUTF8);
    lex->lexi += csize;
}

/*---------------------------------------------------------------------------*/

static void i_flush_bytes(LexScn *lex)
{
    cassert_no_null(lex);
    if (lex->escnbytes > 0)
    {
        lex->escnbytes = 0;
        lex->escneed = 0;
        i_put_code(lex, 0xFFFD);
    }
}

/*---------------------------------------------------------------------------*/

static void i_add_lexeme(LexScn *lex, const uint32_t code)
{
    i_flush_bytes(lex);
    i_put_code(lex, code);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_START(const uint32_t code, ltoken_t *token)
{
    cassert_no_null(token);
    switch (code)
    {
    case '<':
        *token = ekTLESS;
        return stEND;
    case '>':
        *token = ekTGREAT;
        return stEND;
    case ',':
        *token = ekTCOMMA;
        return stEND;
    case ';':
        *token = ekTSCOLON;
        return stEND;
    case ':':
        *token = ekTCOLON;
        return stEND;
    case '(':
        *token = ekTOPENPAR;
        return stEND;
    case ')':
        *token = ekTCLOSPAR;
        return stEND;
    case '[':
        *token = ekTOPENBRAC;
        return stEND;
    case ']':
        *token = ekTCLOSBRAC;
        return stEND;
    case '{':
        *token = ekTOPENCURL;
        return stEND;
    case '}':
        *token = ekTCLOSCURL;
        return stEND;
    case '+':
        *token = ekTPLUS;
        return stEND;
    case '-':
        *token = ekTMINUS;
        return stEND;
    case '*':
        *token = ekTASTERK;
        return stEND;
    case '=':
        *token = ekTEQUALS;
        return stEND;
    case '$':
        *token = ekTDOLLAR;
        return stEND;
    case '%':
        *token = ekTPERCENT;
        return stEND;
    case '#':
        *token = ekTPOUND;
        return stEND;
    case '&':
        *token = ekTAMPER;
        return stEND;
    case '\'':
        *token = ekTAPOST;
        return stEND;
    case '^':
        *token = ekTCIRCUM;
        return stEND;
    case '~':
        *token = ekTTILDE;
        return stEND;
    case '!':
        *token = ekTEXCLA;
        return stEND;
    case '?':
        *token = ekTQUEST;
        return stEND;
    case '|':
        *token = ekTVLINE;
        return stEND;
    case '\\':
        *token = ekTBSLASH;
        return stEND;
    case '@':
        *token = ekTAT;
        return stEND;
    case '/':
        return stSLASH;
    case '.':
        return stPERIOD;
    case '_':
        return stIDENTIFIER;
    case '"':
        return stSTRING;
    default:
        break;
    }

    if (code == ' ' || code == '\t' || code == '\v' || code == '\f' || code == '\r')
        return stSPACES;

    if (code == '\n')
    {
        *token = ekTEOL;
        return stEND;
    }

    if ((code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z'))
        return stIDENTIFIER;

    if (code == '0')
        return stZERO;

    if (code >= '1' && code <= '9')
        return stINT;

    *token = ekTUNDEF;
    return stEND;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_SPACES(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code == ' ' || code == '\t' || code == '\v' || code == '\f' || code == '\r')
        return stSPACES;

    *token = ekTSPACE;
    *charst = STORE_CHAR;
    return stEND;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_SLASH(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code == '/')
        return stSL_COMMENT;
    if (code == '*')
        return stML_COMMENT;

    *token = ekTSLASH;
    *charst = STORE_CHAR;
    return stEND;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_SL_COMMENT(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code == '\n' || code == 0)
    {
        *token = ekTSLCOM;
        *charst = STORE_CHAR;
        return stEND;
    }
    return stSL_COMMENT;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_ML_COMMENT(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code == '*')
        return stML_COMMENT2;

    if (code == 0)
    {
        *token = ekTUNDEF;
        *charst = STORE_CHAR;
        return stEND;
    }

    return stML_COMMENT;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_ML_COMMENT2(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code == '/')
    {
        *token = ekTMLCOM;
        return stEND;
    }

    if (code == '*')
        return stML_COMMENT2;

    if (code == 0)
    {
        *token = ekTUNDEF;
        *charst = STORE_CHAR;
        return stEND;
    }

    return stML_COMMENT;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_PERIOD(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '9')
        return stREAL;

    *charst = STORE_CHAR;
    *token = ekTPERIOD;
    return stEND;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_IDENTIFIER(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= 'a' && code <= 'z')
        return stIDENTIFIER;
    if (code >= 'A' && code <= 'Z')
        return stIDENTIFIER;
    if (code >= '0' && code <= '9')
        return stIDENTIFIER;
    if (code == '_')
        return stIDENTIFIER;

    *token = ekTIDENT;
    *charst = STORE_CHAR;
    return stEND;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_STRING(const uint32_t code, const bool_t escapes, ltoken_t *token, uint8_t *charst)
{
    if (code == '\\')
    {
        if (escapes == TRUE)
            *charst = DISCARD_CHAR;
        else
            *charst = VALID_CHAR;
        return stSTRING_SCAPE;
    }

    if (code == '\"')
    {
        *token = ekTSTRING;
        return stEND;
    }

    if (code == '\n' || code == 0)
    {
        *token = ekTUNDEF;
        *charst = STORE_CHAR;
        return stEND;
    }

    return stSTRING;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_hex_value(const uint32_t code)
{
    if (code >= '0' && code <= '9')
        return code - '0';
    if (code >= 'a' && code <= 'f')
        return code - 'a' + 10;
    cassert(code >= 'A' && code <= 'F');
    return code - 'A' + 10;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_utf8_need(const uint32_t value)
{
    if (value >= 0xC2 && value <= 0xDF)
        return 2;
    if (value >= 0xE0 && value <= 0xEF)
        return 3;
    if (value >= 0xF0 && value <= 0xF4)
        return 4;
    return 0;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_utf8_decode(const byte_t *bytes, const uint32_t nbytes)
{
    uint32_t code = 0;
    uint32_t minc = 0;
    uint32_t i;
    cassert_no_null(bytes);
    cassert(nbytes >= 2 && nbytes <= 4);
    code = (uint32_t)(bytes[0] & (0x7F >> nbytes));
    for (i = 1; i < nbytes; ++i)
        code = (code << 6) | (uint32_t)(bytes[i] & 0x3F);

    if (nbytes == 2)
        minc = 0x80;
    else if (nbytes == 3)
        minc = 0x800;
    else
        minc = 0x10000;

    if (code < minc || unicode_valid(code) == FALSE)
        return 0xFFFD;
    return code;
}

/*---------------------------------------------------------------------------*/

static void i_add_byte(LexScn *lex, const uint32_t value)
{
    cassert_no_null(lex);
    if (value > 0xFF)
    {
        i_add_lexeme(lex, 0xFFFD);
        return;
    }

    if (value == 0)
    {
        i_flush_bytes(lex);
        return;
    }

    if (lex->escnbytes > 0)
    {
        if ((value & 0xC0) == 0x80)
        {
            lex->escbytes[lex->escnbytes] = (byte_t)value;
            lex->escnbytes += 1;
            if (lex->escnbytes == lex->escneed)
            {
                uint32_t code = i_utf8_decode(lex->escbytes, lex->escnbytes);
                lex->escnbytes = 0;
                lex->escneed = 0;
                i_put_code(lex, code);
            }
            return;
        }

        i_flush_bytes(lex);
    }

    if (value < 0x80)
    {
        i_put_code(lex, value);
        return;
    }

    lex->escneed = i_utf8_need(value);
    if (lex->escneed == 0)
    {
        i_put_code(lex, 0xFFFD);
        return;
    }

    lex->escbytes[0] = (byte_t)value;
    lex->escnbytes = 1;
}

/*---------------------------------------------------------------------------*/

static void i_add_escape(LexScn *lex, const uint32_t code)
{
    if (code == 0)
    {
        i_flush_bytes(lex);
        return;
    }

    if (unicode_valid(code) == TRUE)
        i_add_lexeme(lex, code);
    else
        i_add_lexeme(lex, 0xFFFD);
}

/*---------------------------------------------------------------------------*/

static void i_pending_surrogate(LexScn *lex)
{
    cassert_no_null(lex);
    if (lex->escsurr != 0)
    {
        i_add_lexeme(lex, 0xFFFD);
        lex->escsurr = 0;
    }
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_unicode4_value(LexScn *lex, const uint32_t value)
{
    cassert_no_null(lex);
    if (value >= 0xDC00 && value <= 0xDFFF && lex->escsurr != 0)
    {
        uint32_t code = 0x10000 + ((lex->escsurr - 0xD800) << 10) + (value - 0xDC00);
        lex->escsurr = 0;
        i_add_lexeme(lex, code);
        return stSTRING;
    }

    i_pending_surrogate(lex);

    if (value >= 0xD800 && value <= 0xDBFF)
    {
        lex->escsurr = value;
        return stSTRING_SURR1;
    }

    i_add_escape(lex, value);
    return stSTRING;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_STRING_ESCAPE(LexScn *lex, uint32_t *code, uint8_t *charst)
{
    cassert_no_null(lex);
    cassert(lex->esci == 0);
    cassert(lex->escval == 0);
    if (*code == 0)
    {
        *charst = STORE_CHAR;
        return stSTRING;
    }

    if (lex->opts.escapes == FALSE)
    {
        *charst = VALID_CHAR;
        return stSTRING;
    }

    switch (*code)
    {
    case 'a':
        *code = 0x07;
        return stSTRING;

    case 'b':
        *code = 0x08;
        return stSTRING;

    case 'f':
        *code = 0x0C;
        return stSTRING;

    case 'n':
        *code = 0x0A;
        return stSTRING;

    case 'r':
        *code = 0x0D;
        return stSTRING;

    case 't':
        *code = 0x09;
        return stSTRING;

    case 'v':
        *code = 0x0B;
        return stSTRING;

    case '\\':
        return stSTRING;

    case '/':
        return stSTRING;

    case '\'':
        return stSTRING;

    case '\"':
        return stSTRING;

    case '?':
        return stSTRING;

    default:
        break;
    }

    /* Octal char */
    if (*code >= '0' && *code <= '7')
    {
        *charst = DISCARD_CHAR;
        lex->escval = *code - '0';
        lex->esci = 1;
        return stSTRING_OCTAL;
    }

    /* Hex char */
    if (*code == 'x')
    {
        *charst = DISCARD_CHAR;
        return stSTRING_HEX;
    }

    /* Unicode BMP */
    if (*code == 'u')
    {
        *charst = DISCARD_CHAR;
        return stSTRING_UNICODE4;
    }

    /* Unicode 32bits */
    if (*code == 'U')
    {
        *charst = DISCARD_CHAR;
        return stSTRING_UNICODE8;
    }

    /* Unknown escape sequence */
    return stSTRING;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_STRING_OCTAL(LexScn *lex, const uint32_t code, uint8_t *charst)
{
    cassert_no_null(lex);
    cassert(lex->esci > 0 && lex->esci < 3);
    if (code >= '0' && code <= '7')
    {
        lex->escval = lex->escval * 8 + (code - '0');
        lex->esci += 1;
        *charst = DISCARD_CHAR;
        if (lex->esci < 3)
            return stSTRING_OCTAL;
    }
    else
    {
        *charst = STORE_CHAR;
    }

    i_add_byte(lex, lex->escval);
    lex->escval = 0;
    lex->esci = 0;
    return stSTRING;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_STRING_HEX(LexScn *lex, const uint32_t code, uint8_t *charst)
{
    cassert_no_null(lex);
    if (unicode_isxdigit(code) == TRUE)
    {
        if (lex->escval <= 0xFF)
            lex->escval = lex->escval * 16 + i_hex_value(code);
        lex->esci += 1;
        *charst = DISCARD_CHAR;
        return stSTRING_HEX;
    }

    if (lex->esci > 0)
        i_add_byte(lex, lex->escval);
    else
        i_add_lexeme(lex, 0xFFFD);

    lex->escval = 0;
    lex->esci = 0;
    *charst = STORE_CHAR;
    return stSTRING;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_STRING_UNICODE(LexScn *lex, const uint32_t code, uint8_t *charst, const uint32_t ndigits, const gui_state_t state)
{
    cassert_no_null(lex);
    cassert(ndigits == 4 || ndigits == 8);
    cassert(lex->esci < ndigits);
    if (unicode_isxdigit(code) == TRUE)
    {
        lex->escval = (lex->escval << 4) | i_hex_value(code);
        lex->esci += 1;
        *charst = DISCARD_CHAR;
        if (lex->esci < ndigits)
        {
            return state;
        }
        else
        {
            uint32_t value = lex->escval;
            lex->escval = 0;
            lex->esci = 0;
            if (state == stSTRING_UNICODE4)
                return i_unicode4_value(lex, value);
            i_add_escape(lex, value);
            return stSTRING;
        }
    }

    lex->escval = 0;
    lex->esci = 0;
    i_pending_surrogate(lex);
    i_add_lexeme(lex, 0xFFFD);
    *charst = STORE_CHAR;
    return stSTRING;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_STRING_SURR1(LexScn *lex, const uint32_t code, uint8_t *charst)
{
    if (code == '\\')
    {
        *charst = DISCARD_CHAR;
        return stSTRING_SURR2;
    }

    i_pending_surrogate(lex);
    *charst = STORE_CHAR;
    return stSTRING;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_STRING_SURR2(LexScn *lex, const uint32_t code, uint8_t *charst)
{
    if (code == 'u')
    {
        *charst = DISCARD_CHAR;
        return stSTRING_UNICODE4;
    }

    i_pending_surrogate(lex);
    *charst = STORE_CHAR;
    return stSTRING_SCAPE;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_number_end(const uint32_t code, const ltoken_t ntoken, ltoken_t *token, uint8_t *charst)
{
    if ((code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z') || code == '_')
        return stNUMERR;

    *token = ntoken;
    *charst = STORE_CHAR;
    return stEND;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_NUMERR(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if ((code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z') || (code >= '0' && code <= '9') || code == '_')
        return stNUMERR;

    *token = ekTUNDEF;
    *charst = STORE_CHAR;
    return stEND;
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_ZERO(const uint32_t code, const bool_t octal, ltoken_t *token, uint8_t *charst)
{
    if (code == 'x' || code == 'X')
        return stHEX0;
    if (code == '.')
        return stREAL;
    if (code == 'e' || code == 'E')
        return stEXP;
    if (code >= '0' && code <= '9')
    {
        if (octal == FALSE)
            return stINT;
        if (code <= '7')
            return stOCTAL;
        return stOCTAL89;
    }
    return i_number_end(code, ekTINTEGER, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_OCTAL(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '7')
        return stOCTAL;
    if (code == '8' || code == '9')
        return stOCTAL89;
    if (code == '.')
        return stREAL;
    if (code == 'e' || code == 'E')
        return stEXP;
    return i_number_end(code, ekTOCTAL, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_OCTAL89(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '9')
        return stOCTAL89;
    if (code == '.')
        return stREAL;
    if (code == 'e' || code == 'E')
        return stEXP;
    return i_number_end(code, ekTUNDEF, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_INT(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '9')
        return stINT;
    if (code == '.')
        return stREAL;
    if (code == 'e' || code == 'E')
        return stEXP;
    return i_number_end(code, ekTINTEGER, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_HEX0(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (unicode_isxdigit(code) == TRUE)
        return stHEX;
    return i_number_end(code, ekTUNDEF, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_HEX(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (unicode_isxdigit(code) == TRUE)
        return stHEX;
    return i_number_end(code, ekTHEX, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_REAL(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '9')
        return stREAL;
    if (code == 'e' || code == 'E')
        return stEXP;
    return i_number_end(code, ekTREAL, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_EXP(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '9')
        return stEXP1;
    if (code == '-' || code == '+')
        return stEXPSIGN;
    return i_number_end(code, ekTUNDEF, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_EXPSIGN(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '9')
        return stEXP1;
    return i_number_end(code, ekTUNDEF, token, charst);
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_EXP1(const uint32_t code, ltoken_t *token, uint8_t *charst)
{
    if (code >= '0' && code <= '9')
        return stEXP1;
    return i_number_end(code, ekTREAL, token, charst);
}

/*---------------------------------------------------------------------------*/

ltoken_t _lexscn_token(LexScn *lex, Stream *stm)
{
    ltoken_t token = ENUM_MAX(ltoken_t);

    for (;;)
    {
        uint32_t code = 0;
        gui_state_t state = stSTART;
        cassert_no_null(lex);
        lex->tcol = stm_col(stm);
        lex->trow = stm_row(stm);
        lex->lexi = 0;
        lex->esci = 0;
        lex->escval = 0;
        lex->escsurr = 0;
        lex->escnbytes = 0;
        lex->escneed = 0;
        code = i_get_char(lex, stm);

        if (code == 0)
        {
            token = ekTEOF;
            break;
        }

        for (;;)
        {
            uint8_t charst = VALID_CHAR;

            if (code == UINT32_MAX)
            {
                token = ekTCORRUP;
                break;
            }

            token = ENUM_MAX(ltoken_t);
            switch (state)
            {
            case stSTART:
                state = i_START(code, &token);
                break;
            case stSPACES:
                state = i_SPACES(code, &token, &charst);
                break;
            case stSLASH:
                state = i_SLASH(code, &token, &charst);
                break;
            case stSL_COMMENT:
                state = i_SL_COMMENT(code, &token, &charst);
                break;
            case stML_COMMENT:
                state = i_ML_COMMENT(code, &token, &charst);
                break;
            case stML_COMMENT2:
                state = i_ML_COMMENT2(code, &token, &charst);
                break;
            case stPERIOD:
                state = i_PERIOD(code, &token, &charst);
                break;
            case stIDENTIFIER:
                state = i_IDENTIFIER(code, &token, &charst);
                break;
            case stSTRING:
                state = i_STRING(code, lex->opts.escapes, &token, &charst);
                break;
            case stSTRING_SCAPE:
                state = i_STRING_ESCAPE(lex, &code, &charst);
                break;
            case stSTRING_OCTAL:
                state = i_STRING_OCTAL(lex, code, &charst);
                break;
            case stSTRING_HEX:
                state = i_STRING_HEX(lex, code, &charst);
                break;
            case stSTRING_UNICODE4:
                state = i_STRING_UNICODE(lex, code, &charst, 4, stSTRING_UNICODE4);
                break;
            case stSTRING_UNICODE8:
                state = i_STRING_UNICODE(lex, code, &charst, 8, stSTRING_UNICODE8);
                break;
            case stSTRING_SURR1:
                state = i_STRING_SURR1(lex, code, &charst);
                break;
            case stSTRING_SURR2:
                state = i_STRING_SURR2(lex, code, &charst);
                break;
            case stZERO:
                state = i_ZERO(code, lex->opts.octal, &token, &charst);
                break;
            case stINT:
                state = i_INT(code, &token, &charst);
                break;
            case stOCTAL:
                state = i_OCTAL(code, &token, &charst);
                break;
            case stOCTAL89:
                state = i_OCTAL89(code, &token, &charst);
                break;
            case stHEX0:
                state = i_HEX0(code, &token, &charst);
                break;
            case stHEX:
                state = i_HEX(code, &token, &charst);
                break;
            case stREAL:
                state = i_REAL(code, &token, &charst);
                break;
            case stEXP:
                state = i_EXP(code, &token, &charst);
                break;
            case stEXPSIGN:
                state = i_EXPSIGN(code, &token, &charst);
                break;
            case stEXP1:
                state = i_EXP1(code, &token, &charst);
                break;
            case stNUMERR:
                state = i_NUMERR(code, &token, &charst);
                break;
            case stEND:
            default:
                cassert_default(state);
            }

            if (charst == VALID_CHAR)
                i_add_lexeme(lex, code);
            else if (charst == STORE_CHAR)
                i_store_char(lex, stm, code);
            else
            {
                cassert(charst == DISCARD_CHAR);
            }

            if (state == stEND)
                break;

            if (code == 0 && charst != STORE_CHAR)
                break;

            code = i_get_char(lex, stm);
        }

        i_flush_bytes(lex);

        if (token == ekTMLCOM || token == ekTSLCOM)
        {
            if (lex->opts.comments)
                break;
        }
        else if (token == ekTSPACE)
        {
            if (lex->opts.spaces)
                break;
        }
        else if (token == ekTEOL)
        {
            if (lex->opts.spaces | lex->opts.newlines)
                break;
        }
        else
        {
            break;
        }
    }

    lex->lexeme[lex->lexi] = '\0';
    return token;
}

/*---------------------------------------------------------------------------*/

uint32_t _lexscn_row(const LexScn *lex)
{
    cassert_no_null(lex);
    return lex->trow;
}

/*---------------------------------------------------------------------------*/

uint32_t _lexscn_col(const LexScn *lex)
{
    cassert_no_null(lex);
    return lex->tcol;
}

/*---------------------------------------------------------------------------*/

const char_t *_lexscn_lexeme(const LexScn *lex, uint32_t *size)
{
    cassert_no_null(lex);
    ptr_assign(size, lex->lexi);
    return lex->lexeme;
}

/*---------------------------------------------------------------------------*/

const char_t *_lexscn_string(const ltoken_t token)
{
    switch (token)
    {
    case ekTSLCOM:
    case ekTMLCOM:
        return "comment";
    case ekTSPACE:
        return "spaces";
    case ekTEOL:
        return "newline";
    case ekTLESS:
        return "<";
    case ekTGREAT:
        return ">";
    case ekTCOMMA:
        return ",";
    case ekTPERIOD:
        return ".";
    case ekTSCOLON:
        return ";";
    case ekTCOLON:
        return ":";
    case ekTOPENPAR:
        return "(";
    case ekTCLOSPAR:
        return ")";
    case ekTOPENBRAC:
        return "[";
    case ekTCLOSBRAC:
        return "]";
    case ekTOPENCURL:
        return "{";
    case ekTCLOSCURL:
        return "}";
    case ekTPLUS:
        return "+";
    case ekTMINUS:
        return "-";
    case ekTASTERK:
        return "*";
    case ekTEQUALS:
        return "=";
    case ekTDOLLAR:
        return "$";
    case ekTPERCENT:
        return "%";
    case ekTPOUND:
        return "#";
    case ekTAMPER:
        return "&";
    case ekTAPOST:
        return "'";
    case ekTQUOTE:
        return "\"";
    case ekTCIRCUM:
        return "^";
    case ekTTILDE:
        return "~";
    case ekTEXCLA:
        return "!";
    case ekTQUEST:
        return "?";
    case ekTVLINE:
        return "|";
    case ekTSLASH:
        return "/";
    case ekTBSLASH:
        return "\\";
    case ekTAT:
        return "@";
    case ekTINTEGER:
        return "integer";
    case ekTOCTAL:
        return "octal";
    case ekTHEX:
        return "hex";
    case ekTREAL:
        return "real";
    case ekTSTRING:
        return "string";
    case ekTIDENT:
        return "identifier";
    case ekTUNDEF:
        return "undef";
    case ekTCORRUP:
        return "corrupt";
    case ekTEOF:
        return "eof";
    case ekTRESERVED:
        return "reserved";
    default:
        cassert_default(token);
    }

    return "--";
}
