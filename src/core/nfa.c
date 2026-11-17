/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: nfa.c
 *
 */

/* Non-deterministic finite automata */

#include "nfa.inl"
#include "arrpt.h"
#include "arrst.h"
#include "heap.h"
#include "stream.h"
#include "strings.h"
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

typedef enum _symbol_t
{
    ekCHAR,
    ekOR,
    ekCONCAT,
    ekCLOSURE,
    ekLEFT_PAR,
    ekRIGH_PAR,
    ekEMPTY,
    ekBEGIN,
    ekEND
} symbol_t;

typedef enum _escape_t
{
    ekESC_CHAR,
    ekESC_CLASS,
    ekESC_BEGIN,
    ekESC_END,
    ekESC_ERROR
} escape_t;

typedef enum _repeat_t
{
    ekREP_LITERAL,
    ekREP_OK,
    ekREP_BAD
} repeat_t;

typedef struct _ntoken_t NToken;
typedef struct _trans_t Trans;
typedef struct _range_t Range;
typedef struct _posix_t Posix;
typedef struct _parser_t Parser;
typedef struct _nthread_t NThread;
typedef struct _simul_t Simul;

struct _ntoken_t
{
    symbol_t symbol;
    uint32_t from;
    uint32_t to;
    uint32_t pos;
};

struct _trans_t
{
    uint32_t state;
    uint32_t symbol;
    uint32_t extra;
};

struct _range_t
{
    uint32_t from;
    uint32_t to;
};

struct _posix_t
{
    const char_t *name;
    const char_t *ranges;
};

struct _parser_t
{
    const char_t *pattern;
    const char_t *ptr;
    const char_t *errptr;
    rerror_t error;
    symbol_t prev;
    ArrSt(NToken) *tokens;
    ArrSt(uint32_t) *opens;
    ArrSt(Range) *ranges;
};

struct _nthread_t
{
    uint32_t state;
    uint32_t start;
};

struct _simul_t
{
    const Trans *ttable;
    uint32_t accept;
    NThread *clist;
    NThread *nlist;
    uint32_t ccount;
    uint32_t ncount;
    uint32_t *mark;
    uint32_t gen;
    bool_t found;
    uint32_t mstart;
    uint32_t mend;
};

struct _nfa_t
{
    ArrSt(Trans) *ttable;
};

DeclSt(NToken);
DeclSt(Trans);
DeclSt(Range);

/*---------------------------------------------------------------------------*/

static const uint32_t i_MAX_UNICODE = 0x10FFFF;
static const uint32_t i_NO_CHAR = 0x110000;
static const uint32_t i_EPSILON = 0xFFFFFFFF;
static const uint32_t i_ASSERT_BEGIN = 0xFFFFFFFE;
static const uint32_t i_ASSERT_END = 0xFFFFFFFD;
static const uint32_t i_INFINITE = 0xFFFFFFFF;
static const uint32_t i_MAX_REPEAT = 1000;
static const uint32_t i_MAX_STATES = 100000;
static const symbol_t i_NO_SYMBOL = ENUM_MAX(symbol_t);

static const Posix i_POSIX[] = {
    {"alnum", "09AZaz"},
    {"alpha", "AZaz"},
    {"ascii", "\x01\x7f"},
    {"blank", "\t\t  "},
    {"cntrl", "\x01\x1f\x7f\x7f"},
    {"digit", "09"},
    {"graph", "!~"},
    {"lower", "az"},
    {"print", " ~"},
    {"punct", "!/:@[`{~"},
    {"space", "\t\r  "},
    {"upper", "AZ"},
    {"word", "09AZ__az"},
    {"xdigit", "09AFaf"}};

/*---------------------------------------------------------------------------*/

static void i_write_tokens(Stream *stm, const ArrSt(NToken) *tokens)
{
    arrst_foreach_const(token, tokens, NToken)
        switch (token->symbol)
        {
        case ekCHAR:
            if (token->from == token->to)
                stm_printf(stm, "[%d]", token->from);
            else
                stm_printf(stm, "[%d-%d]", token->from, token->to);
            break;

        case ekOR:
            stm_write_char(stm, '|');
            break;

        case ekCONCAT:
            stm_write_char(stm, '&');
            break;

        case ekCLOSURE:
            stm_printf(stm, "{%d,%d}", token->from, token->to);
            break;

        case ekLEFT_PAR:
        case ekRIGH_PAR:
            stm_write_char(stm, token->from);
            break;

        case ekEMPTY:
            stm_writef(stm, "[e]");
            break;

        case ekBEGIN:
            stm_write_char(stm, '^');
            break;

        case ekEND:
            stm_write_char(stm, '$');
            break;

        default:
            cassert_default(token->symbol);
        }
    arrst_end()

    stm_write_char(stm, '\n');
    stm_flush(stm);
}

/*---------------------------------------------------------------------------*/

static void i_write_nfa(Stream *stm, const NFA *nfa)
{
    cassert_no_null(nfa);
    arrst_foreach(trans, nfa->ttable, Trans)
        if (trans->symbol == i_EPSILON)
        {
            if (trans->extra != UINT32_MAX)
                stm_printf(stm, "%d [e1] --> %d [e2] --> %d\n", trans_i, trans->state, trans->extra);
            else
                stm_printf(stm, "%d [e] --> %d\n", trans_i, trans->state);
        }
        else if (trans->symbol == i_ASSERT_BEGIN)
        {
            stm_printf(stm, "%d [^] --> %d\n", trans_i, trans->state);
        }
        else if (trans->symbol == i_ASSERT_END)
        {
            stm_printf(stm, "%d [$] --> %d\n", trans_i, trans->state);
        }
        else if (trans->state == UINT32_MAX)
        {
            cassert(trans_i == trans_total - 1);
            stm_printf(stm, "%d [ACCEPT]\n", trans_i);
        }
        else
        {
            stm_printf(stm, "%d [%d-%d] --> %d\n", trans_i, trans->symbol, trans->extra, trans->state);
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

#if defined(__ASSERTS__)

static bool_t i_is_last(const Trans *trans)
{
    cassert_no_null(trans);
    return (bool_t)(trans->state == UINT32_MAX && trans->symbol == 0 && trans->extra == 0);
}

/*---------------------------------------------------------------------------*/

static bool_t i_check_nfa(const NFA *nfa)
{
    Trans *ltrans = NULL;
    arrst_foreach(trans, nfa->ttable, Trans)
        if (trans->state != UINT32_MAX && trans->state >= trans_total)
            return FALSE;
    arrst_end()

    ltrans = arrst_last(nfa->ttable, Trans);
    return i_is_last(ltrans);
}

#endif

/*---------------------------------------------------------------------------*/

void _nfa_destroy(NFA **nfa)
{
    cassert_no_null(nfa);
    cassert_no_null(*nfa);
    arrst_destroy(&(*nfa)->ttable, NULL, Trans);
    heap_delete(nfa, NFA);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_cp(const char_t *ptr)
{
    return unicode_to_u32(ptr, ekUTF8);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_nx(const char_t *ptr)
{
    return unicode_next(ptr, ekUTF8);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_char_pos(const char_t *pattern, const char_t *ptr)
{
    uint32_t pos = 0;
    while (pattern < ptr)
    {
        pattern = unicode_next(pattern, ekUTF8);
        pos += 1;
    }

    return pos;
}

/*---------------------------------------------------------------------------*/

static void i_error(Parser *parser, const rerror_t error, const char_t *ptr)
{
    cassert_no_null(parser);
    if (parser->error == ekROK)
    {
        parser->error = error;
        parser->errptr = ptr;
    }
}

/*---------------------------------------------------------------------------*/

static void i_token(Parser *parser, const symbol_t symbol, const uint32_t from, const uint32_t to, const char_t *ptr)
{
    NToken *token = NULL;
    cassert_no_null(parser);
    token = arrst_new(parser->tokens, NToken);
    token->symbol = symbol;
    token->from = from;
    token->to = to;
    token->pos = (uint32_t)(ptr - parser->pattern);
    parser->prev = symbol;
}

/*---------------------------------------------------------------------------*/

static bool_t i_operand_end(const symbol_t prev)
{
    return (bool_t)(prev == ekCHAR || prev == ekRIGH_PAR);
}

/*---------------------------------------------------------------------------*/

static void i_empty_operand(Parser *parser)
{
    cassert_no_null(parser);
    if (parser->prev == i_NO_SYMBOL || parser->prev == ekOR || parser->prev == ekLEFT_PAR)
        i_token(parser, ekEMPTY, 0, 0, parser->ptr);
}

/*---------------------------------------------------------------------------*/

static void i_add_range(ArrSt(Range) *ranges, const uint32_t from, const uint32_t to)
{
    Range *range = arrst_new(ranges, Range);
    range->from = from;
    range->to = to;
}

/*---------------------------------------------------------------------------*/

static int i_range_cmp(const Range *range1, const Range *range2)
{
    if (range1->from < range2->from)
        return -1;
    if (range1->from > range2->from)
        return 1;
    return 0;
}

/*---------------------------------------------------------------------------*/

static void i_normalize(ArrSt(Range) *ranges, const bool_t negate)
{
    ArrSt(Range) *merged = arrst_create(Range);
    arrst_sort(ranges, i_range_cmp, Range);
    arrst_foreach_const(range, ranges, Range)
        Range *last = NULL;
        if (arrst_size(merged, Range) > 0)
            last = arrst_last(merged, Range);

        if (last != NULL && range->from <= last->to + 1)
        {
            if (range->to > last->to)
                last->to = range->to;
        }
        else
        {
            i_add_range(merged, range->from, range->to);
        }
    arrst_end()

    arrst_clear(ranges, NULL, Range);
    if (negate == TRUE)
    {
        uint32_t from = 1;
        arrst_foreach_const(range, merged, Range)
            if (range->from > from)
                i_add_range(ranges, from, range->from - 1);
            from = range->to + 1;
        arrst_end()

        if (from <= i_MAX_UNICODE)
            i_add_range(ranges, from, i_MAX_UNICODE);
    }
    else
    {
        arrst_foreach_const(range, merged, Range)
            i_add_range(ranges, range->from, range->to);
        arrst_end()
    }

    arrst_destroy(&merged, NULL, Range);
}

/*---------------------------------------------------------------------------*/

static void i_emit_class(Parser *parser, const char_t *ptr)
{
    uint32_t n = 0;
    cassert_no_null(parser);
    n = arrst_size(parser->ranges, Range);
    if (n == 0)
    {
        i_token(parser, ekCHAR, i_NO_CHAR, i_NO_CHAR, ptr);
    }
    else if (n == 1)
    {
        const Range *range = arrst_get_const(parser->ranges, 0, Range);
        i_token(parser, ekCHAR, range->from, range->to, ptr);
    }
    else
    {
        i_token(parser, ekLEFT_PAR, '(', '(', ptr);
        arrst_foreach_const(range, parser->ranges, Range)
            if (range_i > 0)
                i_token(parser, ekOR, 0, 0, ptr);
            i_token(parser, ekCHAR, range->from, range->to, ptr);
        arrst_end()
        i_token(parser, ekRIGH_PAR, ')', ')', ptr);
    }
}

/*---------------------------------------------------------------------------*/

static void i_add_ranges(ArrSt(Range) *ranges, const char_t *pairs)
{
    while (*pairs != '\0')
    {
        i_add_range(ranges, (uint32_t)(unsigned char)pairs[0], (uint32_t)(unsigned char)pairs[1]);
        pairs += 2;
    }
}

/*---------------------------------------------------------------------------*/

static void i_class_escape(ArrSt(Range) *ranges, const uint32_t codepoint)
{
    ArrSt(Range) *temp = arrst_create(Range);
    bool_t negate = FALSE;
    switch (codepoint)
    {
    case 'D':
        negate = TRUE;
        i_add_ranges(temp, "09");
        break;
    case 'd':
        i_add_ranges(temp, "09");
        break;
    case 'S':
        negate = TRUE;
        i_add_ranges(temp, "\t\n\f\r  ");
        break;
    case 's':
        i_add_ranges(temp, "\t\n\f\r  ");
        break;
    case 'W':
        negate = TRUE;
        i_add_ranges(temp, "09AZ__az");
        break;
    case 'w':
        i_add_ranges(temp, "09AZ__az");
        break;
    default:
        cassert_default(codepoint);
    }

    i_normalize(temp, negate);
    arrst_foreach_const(range, temp, Range)
        i_add_range(ranges, range->from, range->to);
    arrst_end()
    arrst_destroy(&temp, NULL, Range);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_hex_value(const uint32_t codepoint)
{
    if (codepoint >= '0' && codepoint <= '9')
        return codepoint - '0';
    if (codepoint >= 'a' && codepoint <= 'f')
        return codepoint - 'a' + 10;
    if (codepoint >= 'A' && codepoint <= 'F')
        return codepoint - 'A' + 10;
    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

static bool_t i_hex_escape(const char_t **ptr, uint32_t *value)
{
    const char_t *cur = *ptr;
    uint32_t val = 0;
    if (i_cp(cur) == '{')
    {
        uint32_t ndigits = 0;
        cur = i_nx(cur);
        while (i_hex_value(i_cp(cur)) != UINT32_MAX)
        {
            val = val * 16 + i_hex_value(i_cp(cur));
            ndigits += 1;
            if (val > i_MAX_UNICODE)
                return FALSE;
            cur = i_nx(cur);
        }

        if (ndigits == 0 || i_cp(cur) != '}')
            return FALSE;
        cur = i_nx(cur);
    }
    else
    {
        uint32_t i;
        for (i = 0; i < 2; ++i)
        {
            uint32_t digit = i_hex_value(i_cp(cur));
            if (digit == UINT32_MAX)
                return FALSE;
            val = val * 16 + digit;
            cur = i_nx(cur);
        }
    }

    *ptr = cur;
    *value = val;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_is_alnum(const uint32_t codepoint)
{
    if (codepoint >= '0' && codepoint <= '9')
        return TRUE;
    if (codepoint >= 'a' && codepoint <= 'z')
        return TRUE;
    if (codepoint >= 'A' && codepoint <= 'Z')
        return TRUE;
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static escape_t i_escape(const char_t **ptr, const bool_t inclass, ArrSt(Range) *ranges, uint32_t *value)
{
    const char_t *cur = i_nx(*ptr);
    uint32_t codepoint = i_cp(cur);
    escape_t escape = ekESC_CHAR;
    if (codepoint == 0)
        return ekESC_ERROR;
    cur = i_nx(cur);
    switch (codepoint)
    {
    case 'a':
        *value = 7;
        break;
    case 'f':
        *value = 12;
        break;
    case 't':
        *value = 9;
        break;
    case 'n':
        *value = 10;
        break;
    case 'r':
        *value = 13;
        break;
    case 'v':
        *value = 11;
        break;
    case 'x':
        if (i_hex_escape(&cur, value) == FALSE)
            escape = ekESC_ERROR;
        break;
    case 'd':
    case 'D':
    case 's':
    case 'S':
    case 'w':
    case 'W':
        i_class_escape(ranges, codepoint);
        escape = ekESC_CLASS;
        break;
    case 'A':
        escape = ekESC_BEGIN;
        break;
    case 'z':
        escape = ekESC_END;
        break;
    default:
        if (codepoint >= 128 || i_is_alnum(codepoint) == TRUE)
            escape = ekESC_ERROR;
        else
            *value = codepoint;
        break;
    }

    if (inclass == TRUE && (escape == ekESC_BEGIN || escape == ekESC_END))
        escape = ekESC_ERROR;

    if (escape != ekESC_ERROR)
        *ptr = cur;
    return escape;
}

/*---------------------------------------------------------------------------*/

static bool_t i_posix_class(const char_t **ptr, ArrSt(Range) *ranges, bool_t *valid)
{
    const char_t *cur = i_nx(i_nx(*ptr));
    const char_t *name = NULL;
    uint32_t len = 0;
    bool_t negate = FALSE;
    uint32_t i, n = sizeof(i_POSIX) / sizeof(Posix);

    if (i_cp(cur) == '^')
    {
        negate = TRUE;
        cur = i_nx(cur);
    }

    name = cur;
    while (i_cp(cur) >= 'a' && i_cp(cur) <= 'z')
    {
        cur = i_nx(cur);
        len += 1;
    }

    if (i_cp(cur) != ':' || i_cp(i_nx(cur)) != ']')
        return FALSE;

    *valid = FALSE;
    for (i = 0; i < n; ++i)
    {
        if (str_len_c(i_POSIX[i].name) == len && str_equ_cn(i_POSIX[i].name, name, len) == TRUE)
        {
            ArrSt(Range) *temp = arrst_create(Range);
            i_add_ranges(temp, i_POSIX[i].ranges);
            i_normalize(temp, negate);
            arrst_foreach_const(range, temp, Range)
                i_add_range(ranges, range->from, range->to);
            arrst_end()
            arrst_destroy(&temp, NULL, Range);
            *valid = TRUE;
            break;
        }
    }

    *ptr = i_nx(i_nx(cur));
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_class_char(Parser *parser, const char_t **ptr, uint32_t *value, bool_t *isclass)
{
    const char_t *cur = *ptr;
    *isclass = FALSE;
    if (i_cp(cur) == '\\')
    {
        escape_t escape = i_escape(&cur, TRUE, parser->ranges, value);
        if (escape == ekESC_ERROR)
        {
            i_error(parser, ekRESCAPE, *ptr);
            return FALSE;
        }

        if (escape == ekESC_CLASS)
            *isclass = TRUE;
    }
    else
    {
        *value = i_cp(cur);
        cur = i_nx(cur);
    }

    *ptr = cur;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_class(Parser *parser)
{
    const char_t *open = parser->ptr;
    const char_t *cur = i_nx(open);
    bool_t negate = FALSE;
    bool_t first = TRUE;

    arrst_clear(parser->ranges, NULL, Range);
    if (i_cp(cur) == '^')
    {
        negate = TRUE;
        cur = i_nx(cur);
    }

    for (;;)
    {
        uint32_t codepoint = i_cp(cur);
        const char_t *item = cur;
        uint32_t low = 0;
        bool_t isclass = FALSE;

        if (codepoint == 0)
        {
            i_error(parser, ekRCLASS, open);
            return;
        }

        if (codepoint == ']' && first == FALSE)
        {
            cur = i_nx(cur);
            break;
        }

        first = FALSE;

        if (codepoint == '[' && i_cp(i_nx(cur)) == ':')
        {
            bool_t valid = FALSE;
            if (i_posix_class(&cur, parser->ranges, &valid) == TRUE)
            {
                if (valid == FALSE)
                {
                    i_error(parser, ekRCLASS, item);
                    return;
                }

                continue;
            }
        }

        if (i_class_char(parser, &cur, &low, &isclass) == FALSE)
            return;

        if (i_cp(cur) == '-' && i_cp(i_nx(cur)) != ']' && i_cp(i_nx(cur)) != 0)
        {
            const char_t *hptr = i_nx(cur);
            uint32_t high = 0;
            bool_t hclass = FALSE;

            if (isclass == TRUE)
            {
                i_error(parser, ekRRANGE, item);
                return;
            }

            if (i_class_char(parser, &hptr, &high, &hclass) == FALSE)
                return;

            if (hclass == TRUE || high < low)
            {
                i_error(parser, ekRRANGE, item);
                return;
            }

            i_add_range(parser->ranges, low, high);
            cur = hptr;
        }
        else if (isclass == FALSE)
        {
            i_add_range(parser->ranges, low, low);
        }
    }

    i_normalize(parser->ranges, negate);
    i_emit_class(parser, open);
    parser->ptr = cur;
}

/*---------------------------------------------------------------------------*/

static bool_t i_number(const char_t **ptr, uint32_t *value)
{
    const char_t *cur = *ptr;
    uint32_t ndigits = 0;
    uint32_t val = 0;
    while (i_cp(cur) >= '0' && i_cp(cur) <= '9')
    {
        if (ndigits < 9)
            val = val * 10 + (i_cp(cur) - '0');
        else
            val = UINT32_MAX - 1;
        ndigits += 1;
        cur = i_nx(cur);
    }

    if (ndigits == 0)
        return FALSE;

    *ptr = cur;
    *value = val;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static repeat_t i_repeat(const char_t **ptr, uint32_t *min, uint32_t *max)
{
    const char_t *cur = i_nx(*ptr);
    if (i_number(&cur, min) == FALSE)
        return ekREP_LITERAL;

    if (i_cp(cur) == '}')
    {
        *max = *min;
    }
    else if (i_cp(cur) == ',')
    {
        cur = i_nx(cur);
        if (i_cp(cur) == '}')
        {
            *max = i_INFINITE;
        }
        else
        {
            if (i_number(&cur, max) == FALSE)
                return ekREP_LITERAL;
            if (i_cp(cur) != '}')
                return ekREP_LITERAL;
        }
    }
    else
    {
        return ekREP_LITERAL;
    }

    *ptr = i_nx(cur);

    if (*min > i_MAX_REPEAT)
        return ekREP_BAD;

    if (*max != i_INFINITE && (*max > i_MAX_REPEAT || *min > *max))
        return ekREP_BAD;

    return ekREP_OK;
}

/*---------------------------------------------------------------------------*/

static void i_quantifier(Parser *parser, const uint32_t min, const uint32_t max, const char_t *qptr, const char_t *next)
{
    cassert_no_null(parser);
    if (i_operand_end(parser->prev) == FALSE)
    {
        i_error(parser, ekRREPEAT, qptr);
        return;
    }

    i_token(parser, ekCLOSURE, min, max, qptr);
    parser->ptr = next;
    if (i_cp(parser->ptr) == '?')
        parser->ptr = i_nx(parser->ptr);
}

/*---------------------------------------------------------------------------*/

static void i_escape_token(Parser *parser)
{
    const char_t *cur = parser->ptr;
    uint32_t value = 0;
    escape_t escape = ekESC_ERROR;
    arrst_clear(parser->ranges, NULL, Range);
    escape = i_escape(&cur, FALSE, parser->ranges, &value);
    switch (escape)
    {
    case ekESC_CHAR:
        i_token(parser, ekCHAR, value, value, parser->ptr);
        break;
    case ekESC_CLASS:
        i_normalize(parser->ranges, FALSE);
        i_emit_class(parser, parser->ptr);
        break;
    case ekESC_BEGIN:
        i_token(parser, ekBEGIN, 0, 0, parser->ptr);
        break;
    case ekESC_END:
        i_token(parser, ekEND, 0, 0, parser->ptr);
        break;
    case ekESC_ERROR:
        i_error(parser, ekRESCAPE, parser->ptr);
        return;
    default:
        cassert_default(escape);
    }

    parser->ptr = cur;
}

/*---------------------------------------------------------------------------*/

static void i_parse_char(Parser *parser)
{
    const char_t *cur = parser->ptr;
    const char_t *next = i_nx(cur);
    uint32_t codepoint = i_cp(cur);

    switch (codepoint)
    {
    case '(':
    {
        uint32_t offset = (uint32_t)(cur - parser->pattern);
        if (i_cp(next) == '?' && i_cp(i_nx(next)) == ':')
            next = i_nx(i_nx(next));
        i_token(parser, ekLEFT_PAR, '(', '(', cur);
        arrst_append(parser->opens, offset, uint32_t);
        parser->ptr = next;
        break;
    }

    case ')':
        if (arrst_size(parser->opens, uint32_t) == 0)
        {
            i_error(parser, ekRCLOSE, cur);
            return;
        }

        i_empty_operand(parser);
        i_token(parser, ekRIGH_PAR, ')', ')', cur);
        arrst_pop(parser->opens, NULL, uint32_t);
        parser->ptr = next;
        break;

    case '|':
        i_empty_operand(parser);
        i_token(parser, ekOR, 0, 0, cur);
        parser->ptr = next;
        break;

    case '*':
        i_quantifier(parser, 0, i_INFINITE, cur, next);
        break;

    case '+':
        i_quantifier(parser, 1, i_INFINITE, cur, next);
        break;

    case '?':
        i_quantifier(parser, 0, 1, cur, next);
        break;

    case '{':
    {
        uint32_t min = 0, max = 0;
        const char_t *end = cur;
        repeat_t repeat = i_repeat(&end, &min, &max);
        if (repeat == ekREP_OK)
        {
            i_quantifier(parser, min, max, cur, end);
        }
        else if (repeat == ekREP_BAD)
        {
            i_error(parser, ekRCOUNT, cur);
        }
        else
        {
            i_token(parser, ekCHAR, codepoint, codepoint, cur);
            parser->ptr = next;
        }
        break;
    }

    case '^':
        i_token(parser, ekBEGIN, 0, 0, cur);
        parser->ptr = next;
        break;

    case '$':
        i_token(parser, ekEND, 0, 0, cur);
        parser->ptr = next;
        break;

    case '.':
        arrst_clear(parser->ranges, NULL, Range);
        i_add_range(parser->ranges, 1, '\n' - 1);
        i_add_range(parser->ranges, '\n' + 1, i_MAX_UNICODE);
        i_emit_class(parser, cur);
        parser->ptr = next;
        break;

    case '[':
        i_class(parser);
        break;

    case '\\':
        i_escape_token(parser);
        break;

    default:
        i_token(parser, ekCHAR, codepoint, codepoint, cur);
        parser->ptr = next;
        break;
    }
}

/*---------------------------------------------------------------------------*/

static ArrSt(NToken) *i_tokens(const char_t *pattern, rerror_t *error, const char_t **errptr)
{
    Parser parser;
    parser.pattern = pattern;
    parser.ptr = pattern;
    parser.errptr = pattern;
    parser.error = ekROK;
    parser.prev = i_NO_SYMBOL;
    parser.tokens = arrst_create(NToken);
    parser.opens = arrst_create(uint32_t);
    parser.ranges = arrst_create(Range);

    while (parser.error == ekROK && i_cp(parser.ptr) != 0)
        i_parse_char(&parser);

    if (parser.error == ekROK)
    {
        if (arrst_size(parser.opens, uint32_t) > 0)
        {
            const uint32_t *offset = arrst_last(parser.opens, uint32_t);
            i_error(&parser, ekROPEN, pattern + *offset);
        }
        else
        {
            i_empty_operand(&parser);
        }
    }

    if (parser.error != ekROK)
    {
        arrst_destroy(&parser.tokens, NULL, NToken);
    }

    arrst_destroy(&parser.opens, NULL, uint32_t);
    arrst_destroy(&parser.ranges, NULL, Range);
    *error = parser.error;
    *errptr = parser.errptr;
    return parser.tokens;
}

/*---------------------------------------------------------------------------*/

static bool_t i_starts_operand(const symbol_t symbol)
{
    return (bool_t)(symbol == ekCHAR || symbol == ekEMPTY || symbol == ekBEGIN || symbol == ekEND || symbol == ekLEFT_PAR);
}

/*---------------------------------------------------------------------------*/

static bool_t i_ends_operand(const symbol_t symbol)
{
    return (bool_t)(symbol == ekCHAR || symbol == ekEMPTY || symbol == ekBEGIN || symbol == ekEND || symbol == ekRIGH_PAR || symbol == ekCLOSURE);
}

/*---------------------------------------------------------------------------*/

static void i_implicits(ArrSt(NToken) *tokens)
{
    uint32_t i = 0, n = arrst_size(tokens, NToken);
    symbol_t prev = i_NO_SYMBOL;
    while (i < n)
    {
        NToken token = *arrst_get(tokens, i, NToken);
        if (i_starts_operand(token.symbol) == TRUE && i_ends_operand(prev) == TRUE)
        {
            NToken ntoken;
            ntoken.symbol = ekCONCAT;
            ntoken.from = 0;
            ntoken.to = 0;
            ntoken.pos = token.pos;
            arrst_insert(tokens, i, ntoken, NToken);
            i += 1;
            n += 1;
        }

        prev = token.symbol;
        i += 1;
    }
}

/*---------------------------------------------------------------------------*/
/* Shunting-yard algorithm */
static void i_regex_to_infix(ArrSt(NToken) **tokens)
{
    ArrSt(NToken) *output = arrst_create(NToken);
    ArrSt(NToken) *stack = arrst_create(NToken);
    arrst_foreach(token, *tokens, NToken)
        switch (token->symbol)
        {
        /* If the token is a number, then: Push it to the output queue. */
        case ekCHAR:
        case ekEMPTY:
        case ekBEGIN:
        case ekEND:
            arrst_append(output, *token, NToken);
            break;

        /*
         *  If the token is an operator, then:
         *  while (there is an operator at the top of the operator stack with greater precedence)
         *  and (the operator at the top of the operator stack is not a left parenthesis):
         *  pop operators from the operator stack onto the output queue.
         *  Push it onto the operator stack.
         */
        case ekOR:
        case ekCONCAT:
        case ekCLOSURE:
            while (arrst_size(stack, NToken) > 0)
            {
                const NToken *top = arrst_last(stack, NToken);
                if (top->symbol != ekLEFT_PAR && top->symbol >= token->symbol)
                {
                    arrst_append(output, *top, NToken);
                    arrst_pop(stack, NULL, NToken);
                    if (top->symbol == token->symbol)
                        break;
                }
                else
                {
                    break;
                }
            }
            arrst_append(stack, *token, NToken);
            break;

        /* If the token is a left paren (i.e. "("), then: Push it onto the operator stack. */
        case ekLEFT_PAR:
            arrst_append(stack, *token, NToken);
            break;

        /*
         * If the token is a right paren (i.e. ")"), then:
         *  while the operator at the top of the operator stack is not a left paren:
         *  pop the operator from the operator stack onto the output queue.
         */
        case ekRIGH_PAR:
            for (;;)
            {
                NToken top = *arrst_last(stack, NToken);
                arrst_pop(stack, NULL, NToken);
                if (top.symbol != ekLEFT_PAR)
                {
                    arrst_append(output, top, NToken);
                }
                else
                {
                    cassert(token->from == ')' && top.from == '(');
                    break;
                }
            }
            break;

        default:
            cassert_default(token->symbol);
        }
    arrst_end()

    while (arrst_size(stack, NToken) > 0)
    {
        const NToken *top = arrst_last(stack, NToken);
        arrst_append(output, *top, NToken);
        arrst_pop(stack, NULL, NToken);
    }

    arrst_destroy(&stack, NULL, NToken);
    arrst_destroy(tokens, NULL, NToken);
    *tokens = output;
}

/*---------------------------------------------------------------------------*/

static void i_offset(ArrSt(Trans) *ttable, const uint32_t offset)
{
    uint32_t i, n = arrst_size(ttable, Trans);
    Trans *trans = arrst_all(ttable, Trans);

    cassert(n >= 2);
    n -= 1;
    for (i = 0; i < n; ++i, ++trans)
    {
        cassert(trans->state != UINT32_MAX);
        trans->state += offset;
        if (trans->symbol == i_EPSILON)
        {
            if (trans->extra != UINT32_MAX)
                trans->extra += offset;
        }
    }

    /* Last state == Accept state */
    cassert(i_is_last(trans) == TRUE);
}

/*---------------------------------------------------------------------------*/

static NFA *i_nfa_base(const uint32_t symbol, const uint32_t extra)
{
    NFA *nfa = heap_new0(NFA);
    Trans *trans = NULL;
    nfa->ttable = arrst_create(Trans);
    trans = arrst_new(nfa->ttable, Trans);
    trans->state = 1;
    trans->symbol = symbol;
    trans->extra = extra;
    trans = arrst_new(nfa->ttable, Trans);
    trans->state = UINT32_MAX;
    trans->symbol = 0;
    trans->extra = 0;
    return nfa;
}

/*---------------------------------------------------------------------------*/

static NFA *i_nfa_copy(const NFA *nfa)
{
    NFA *copy = heap_new0(NFA);
    uint32_t n = arrst_size(nfa->ttable, Trans);
    Trans *trans = NULL;
    copy->ttable = arrst_create(Trans);
    trans = arrst_new_n(copy->ttable, n, Trans);
    bmem_copy_n(trans, arrst_all_const(nfa->ttable, Trans), n, Trans);
    return copy;
}

/*---------------------------------------------------------------------------*/

static void i_nfa_union(NFA *nfa1, NFA *nfa2)
{
    uint32_t n1, n2;
    Trans *trans = NULL;
    Trans *trans2 = NULL;
    cassert_no_null(nfa1);
    cassert_no_null(nfa2);
    n1 = arrst_size(nfa1->ttable, Trans);
    n2 = arrst_size(nfa2->ttable, Trans);

    /* 1) Move one position all states in nfa1 (the new '0' state)  */
    i_offset(nfa1->ttable, 1);

    /* 2) Move 'n1+1' positions all states in nfa2  */
    i_offset(nfa2->ttable, n1 + 1);

    /* 3) Prepends a new '0' state with two epsilon-transitions to nfa1-'0' and nfa2-'0'*/
    trans = arrst_prepend_n(nfa1->ttable, 1, Trans);
    trans->state = 1;
    trans->symbol = i_EPSILON;
    trans->extra = n1 + 1;

    /* 4) Adds an epsilon-transition from nfa1-'last' to new 'last'  */
    trans = arrst_last(nfa1->ttable, Trans);
    cassert(i_is_last(trans) == TRUE);
    trans->state = n1 + n2 + 1;
    trans->symbol = i_EPSILON;
    trans->extra = UINT32_MAX;

    /* 5) Adds an epsilon-transition from nfa2-'last' to new 'last'  */
    trans = arrst_last(nfa2->ttable, Trans);
    cassert(i_is_last(trans) == TRUE);
    trans->state = n1 + n2 + 1;
    trans->symbol = i_EPSILON;
    trans->extra = UINT32_MAX;

    /* 6) Copy all df2 transitions to df1 */
    arrst_new_n(nfa1->ttable, n2, Trans);
    trans = arrst_all(nfa1->ttable, Trans);
    trans += n1 + 1;
    trans2 = arrst_all(nfa2->ttable, Trans);
    bmem_copy_n(trans, trans2, n2, Trans);

    /* 7) Add the last state (accept) */
    trans = arrst_new(nfa1->ttable, Trans);
    trans->state = UINT32_MAX;
    trans->symbol = 0;
    trans->extra = 0;
}

/*---------------------------------------------------------------------------*/

static void i_nfa_concat(NFA *nfa1, NFA *nfa2)
{
    uint32_t n1, n2;
    Trans *trans = NULL;
    Trans *trans2 = NULL;
    cassert_no_null(nfa1);
    cassert_no_null(nfa2);
    n1 = arrst_size(nfa1->ttable, Trans);
    n2 = arrst_size(nfa2->ttable, Trans);

    /* 1) Move 'n1' positions all states in nfa2  */
    i_offset(nfa2->ttable, n1);

    /* 2) Adds an epsilon-transition from nfa1-'last' to nfa2-'0' */
    trans = arrst_last(nfa1->ttable, Trans);
    cassert(i_is_last(trans) == TRUE);
    trans->state = n1;
    trans->symbol = i_EPSILON;
    trans->extra = UINT32_MAX;

    /* 3) Copy all df2 transitions to df1 */
    arrst_new_n(nfa1->ttable, n2, Trans);
    trans = arrst_all(nfa1->ttable, Trans);
    trans += n1;
    trans2 = arrst_all(nfa2->ttable, Trans);
    bmem_copy_n(trans, trans2, n2, Trans);
}

/*---------------------------------------------------------------------------*/

static void i_nfa_closure(NFA *nfa1, const uint32_t kind)
{
    uint32_t n1;
    Trans *trans = NULL;
    cassert_no_null(nfa1);
    n1 = arrst_size(nfa1->ttable, Trans);

    /* 1) Move one position all states in nfa1 (the new '0' state)  */
    i_offset(nfa1->ttable, 1);

    /* 2) Prepends a new '0' state with epsilon-transitions to nfa1-'0' and nfa1-'newlast' (not in '+') */
    trans = arrst_prepend_n(nfa1->ttable, 1, Trans);
    trans->state = 1;
    trans->symbol = i_EPSILON;
    if (kind == '+')
        trans->extra = UINT32_MAX;
    else
        trans->extra = n1 + 1;

    /* 3) Adds epsilon-transitions from nfa1-'last' to nfa1-'0' (not in '?') and nfa1-'newlast' */
    trans = arrst_last(nfa1->ttable, Trans);
    cassert(i_is_last(trans) == TRUE);
    trans->symbol = i_EPSILON;
    if (kind == '?')
    {
        trans->state = n1 + 1;
        trans->extra = UINT32_MAX;
    }
    else
    {
        trans->state = 1;
        trans->extra = n1 + 1;
    }

    /* 4) Adds the nfa1-'newlast' (accept) */
    trans = arrst_new(nfa1->ttable, Trans);
    trans->state = UINT32_MAX;
    trans->symbol = 0;
    trans->extra = 0;
}

/*---------------------------------------------------------------------------*/

static void i_nfa_append(NFA **nfa, NFA *nfa2)
{
    cassert_no_null(nfa);
    if (*nfa == NULL)
    {
        *nfa = nfa2;
    }
    else
    {
        i_nfa_concat(*nfa, nfa2);
        _nfa_destroy(&nfa2);
    }
}

/*---------------------------------------------------------------------------*/

static NFA *i_nfa_repeat(NFA *nfa1, const uint32_t min, const uint32_t max)
{
    NFA *nfa = NULL;
    uint32_t i;
    cassert_no_null(nfa1);

    if (min == 0 && max == i_INFINITE)
    {
        i_nfa_closure(nfa1, '*');
        return nfa1;
    }

    if (min == 1 && max == i_INFINITE)
    {
        i_nfa_closure(nfa1, '+');
        return nfa1;
    }

    if (min == 0 && max == 1)
    {
        i_nfa_closure(nfa1, '?');
        return nfa1;
    }

    for (i = 0; i < min; ++i)
    {
        NFA *copy = i_nfa_copy(nfa1);
        i_nfa_append(&nfa, copy);
    }

    if (max == i_INFINITE)
    {
        NFA *copy = i_nfa_copy(nfa1);
        i_nfa_closure(copy, '*');
        i_nfa_append(&nfa, copy);
    }
    else
    {
        for (i = min; i < max; ++i)
        {
            NFA *copy = i_nfa_copy(nfa1);
            i_nfa_closure(copy, '?');
            i_nfa_append(&nfa, copy);
        }
    }

    if (nfa == NULL)
        nfa = i_nfa_base(i_EPSILON, UINT32_MAX);

    _nfa_destroy(&nfa1);
    return nfa;
}

/*---------------------------------------------------------------------------*/

static bool_t i_repeat_size(const NFA *nfa, const uint32_t min, const uint32_t max)
{
    uint64_t copies = min + 1;
    uint64_t size = arrst_size(nfa->ttable, Trans) + 2;
    if (max != i_INFINITE)
        copies = max;
    return (bool_t)(size * copies <= i_MAX_STATES);
}

/*---------------------------------------------------------------------------*/

static NFA *i_infix_to_NFA(const ArrSt(NToken) *tokens, uint32_t *errpos)
{
    ArrPt(NFA) *stack = arrpt_create(NFA);
    NFA *nfa = NULL;
    bool_t ok = TRUE;

    arrst_foreach_const(token, tokens, NToken)
        switch (token->symbol)
        {
        case ekCHAR:
        {
            NFA *nfa1 = i_nfa_base(token->from, token->to);
            cassert(i_check_nfa(nfa1) == TRUE);
            arrpt_append(stack, nfa1, NFA);
            break;
        }

        case ekEMPTY:
        {
            NFA *nfa1 = i_nfa_base(i_EPSILON, UINT32_MAX);
            arrpt_append(stack, nfa1, NFA);
            break;
        }

        case ekBEGIN:
        {
            NFA *nfa1 = i_nfa_base(i_ASSERT_BEGIN, UINT32_MAX);
            arrpt_append(stack, nfa1, NFA);
            break;
        }

        case ekEND:
        {
            NFA *nfa1 = i_nfa_base(i_ASSERT_END, UINT32_MAX);
            arrpt_append(stack, nfa1, NFA);
            break;
        }

        case ekOR:
        {
            NFA *nfa1, *nfa2;
            nfa2 = arrpt_last(stack, NFA);
            arrpt_pop(stack, NULL, NFA);
            nfa1 = arrpt_last(stack, NFA);
            i_nfa_union(nfa1, nfa2);
            cassert(i_check_nfa(nfa1) == TRUE);
            _nfa_destroy(&nfa2);
            break;
        }

        case ekCONCAT:
        {
            NFA *nfa1, *nfa2;
            nfa2 = arrpt_last(stack, NFA);
            arrpt_pop(stack, NULL, NFA);
            nfa1 = arrpt_last(stack, NFA);
            i_nfa_concat(nfa1, nfa2);
            cassert(i_check_nfa(nfa1) == TRUE);
            _nfa_destroy(&nfa2);
            break;
        }

        case ekCLOSURE:
        {
            NFA *nfa1 = arrpt_last(stack, NFA);
            if (i_repeat_size(nfa1, token->from, token->to) == TRUE)
            {
                NFA *nfa2 = NULL;
                arrpt_pop(stack, NULL, NFA);
                nfa2 = i_nfa_repeat(nfa1, token->from, token->to);
                cassert(i_check_nfa(nfa2) == TRUE);
                arrpt_append(stack, nfa2, NFA);
            }
            else
            {
                ok = FALSE;
                *errpos = token->pos;
            }
            break;
        }

        case ekLEFT_PAR:
        case ekRIGH_PAR:
        default:
            cassert_default(token->symbol);
        }

        if (ok == TRUE)
        {
            const NFA *top = arrpt_last(stack, NFA);
            if (arrst_size(top->ttable, Trans) > i_MAX_STATES)
            {
                ok = FALSE;
                *errpos = token->pos;
            }
        }

        if (ok == FALSE)
            break;
    arrst_end()

    if (ok == TRUE)
    {
        nfa = arrpt_last(stack, NFA);
        arrpt_pop(stack, NULL, NFA);
        cassert(arrpt_size(stack, NFA) == 0);
    }

    arrpt_destroy(&stack, _nfa_destroy, NFA);
    return nfa;
}

/*---------------------------------------------------------------------------*/

NFA *_nfa_regex(const char_t *regex, const bool_t verbose, rerror_t *error, uint32_t *errpos)
{
    rerror_t err = ekROK;
    const char_t *errptr = regex;
    ArrSt(NToken) *tokens = i_tokens(regex, &err, &errptr);
    NFA *nfa = NULL;

    if (tokens != NULL)
    {
        uint32_t offset = 0;

        if (verbose == TRUE)
        {
            stm_writef(kSTDOUT, "Tokens: ");
            i_write_tokens(kSTDOUT, tokens);
        }

        i_implicits(tokens);
        if (verbose == TRUE)
        {
            stm_writef(kSTDOUT, "Implic: ");
            i_write_tokens(kSTDOUT, tokens);
        }

        i_regex_to_infix(&tokens);
        if (verbose == TRUE)
        {
            stm_writef(kSTDOUT, "Infix: ");
            i_write_tokens(kSTDOUT, tokens);
        }

        nfa = i_infix_to_NFA(tokens, &offset);
        arrst_destroy(&tokens, NULL, NToken);

        if (nfa != NULL)
        {
            cassert(i_check_nfa(nfa) == TRUE);
            if (verbose == TRUE)
            {
                stm_writef(kSTDOUT, "NFA:\n");
                i_write_nfa(kSTDOUT, nfa);
                stm_flush(kSTDOUT);
            }
        }
        else
        {
            err = ekRSIZE;
            errptr = regex + offset;
        }
    }

    ptr_assign(error, err);
    if (err == ekROK)
    {
        ptr_assign(errpos, UINT32_MAX);
    }
    else
    {
        ptr_assign(errpos, i_char_pos(regex, errptr));
    }

    return nfa;
}

/*---------------------------------------------------------------------------*/

static void i_found(Simul *simul, const uint32_t start, const uint32_t end)
{
    if (simul->found == FALSE || start < simul->mstart || (start == simul->mstart && end > simul->mend))
    {
        simul->found = TRUE;
        simul->mstart = start;
        simul->mend = end;
    }
}

/*---------------------------------------------------------------------------*/

static void i_add_thread(Simul *simul, const uint32_t state, const uint32_t start, const uint32_t pos, const bool_t at_begin, const bool_t at_end)
{
    const Trans *trans = simul->ttable + state;

    if (simul->mark[state] == simul->gen)
        return;

    simul->mark[state] = simul->gen;

    if (trans->symbol == i_EPSILON)
    {
        i_add_thread(simul, trans->state, start, pos, at_begin, at_end);

        /* Two epsilons */
        if (trans->extra != UINT32_MAX)
            i_add_thread(simul, trans->extra, start, pos, at_begin, at_end);
    }
    else if (trans->symbol == i_ASSERT_BEGIN)
    {
        if (at_begin == TRUE)
            i_add_thread(simul, trans->state, start, pos, at_begin, at_end);
    }
    else if (trans->symbol == i_ASSERT_END)
    {
        if (at_end == TRUE)
            i_add_thread(simul, trans->state, start, pos, at_begin, at_end);
    }
    else if (state == simul->accept)
    {
        i_found(simul, start, pos);
    }
    else
    {
        NThread *thread = simul->nlist + simul->ncount;
        thread->state = state;
        thread->start = start;
        simul->ncount += 1;
    }
}

/*---------------------------------------------------------------------------*/

static void i_swap(Simul *simul)
{
    NThread *list = simul->clist;
    simul->clist = simul->nlist;
    simul->nlist = list;
    simul->ccount = simul->ncount;
    simul->ncount = 0;
    simul->gen += 1;
}

/*---------------------------------------------------------------------------*/

static bool_t i_simulate(const NFA *nfa, const char_t *str, const bool_t anchored, uint32_t *start, uint32_t *end, bool_t *at_end)
{
    Simul simul;
    uint32_t n = 0;
    uint32_t pos = 0;
    uint32_t codepoint = i_cp(str);
    cassert_no_null(nfa);
    n = arrst_size(nfa->ttable, Trans);
    simul.ttable = arrst_all_const(nfa->ttable, Trans);
    simul.accept = n - 1;
    simul.clist = heap_new_n(n, NThread);
    simul.nlist = heap_new_n(n, NThread);
    simul.mark = heap_new_n0(n, uint32_t);
    simul.ccount = 0;
    simul.ncount = 0;
    simul.gen = 1;
    simul.found = FALSE;
    simul.mstart = 0;
    simul.mend = 0;

    i_add_thread(&simul, 0, 0, 0, TRUE, (bool_t)(codepoint == 0));
    i_swap(&simul);

    while (codepoint != 0)
    {
        const char_t *next = NULL;
        uint32_t ncodepoint = 0;
        uint32_t i;

        if (simul.ccount == 0 && (anchored == TRUE || simul.found == TRUE))
            break;

        next = i_nx(str);
        ncodepoint = i_cp(next);
        for (i = 0; i < simul.ccount; ++i)
        {
            const NThread *thread = simul.clist + i;
            const Trans *trans = simul.ttable + thread->state;
            if (codepoint >= trans->symbol && codepoint <= trans->extra)
                i_add_thread(&simul, trans->state, thread->start, pos + 1, FALSE, (bool_t)(ncodepoint == 0));
        }

        if (anchored == FALSE && simul.found == FALSE)
            i_add_thread(&simul, 0, pos + 1, pos + 1, FALSE, (bool_t)(ncodepoint == 0));

        i_swap(&simul);
        pos += 1;
        str = next;
        codepoint = ncodepoint;
    }

    heap_delete_n(&simul.clist, n, NThread);
    heap_delete_n(&simul.nlist, n, NThread);
    heap_delete_n(&simul.mark, n, uint32_t);
    *start = simul.mstart;
    *end = simul.mend;
    *at_end = (bool_t)(codepoint == 0 && simul.mend == pos);
    return simul.found;
}

/*---------------------------------------------------------------------------*/

bool_t _nfa_match(const NFA *nfa, const char_t *str)
{
    uint32_t start = 0, end = 0;
    bool_t at_end = FALSE;
    bool_t found = i_simulate(nfa, str, TRUE, &start, &end, &at_end);
    return (bool_t)(found == TRUE && at_end == TRUE);
}

/*---------------------------------------------------------------------------*/

bool_t _nfa_search(const NFA *nfa, const char_t *str, uint32_t *pos, uint32_t *len)
{
    uint32_t start = 0, end = 0;
    bool_t at_end = FALSE;
    bool_t found = i_simulate(nfa, str, FALSE, &start, &end, &at_end);
    if (found == TRUE)
    {
        ptr_assign(pos, start);
        ptr_assign(len, end - start);
    }

    return found;
}
