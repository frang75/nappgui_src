/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: json.c
 *
 */

/* Json parser */

#include "json.h"
#include "base64.h"
#include <core/arrpt.h>
#include <core/dbindh.h>
#include <core/heap.h>
#include <core/stream.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/bmem.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

typedef enum _jtoken_t
{
    i_ekTRUE,
    i_ekFALSE,
    i_ekNULL,
    i_ekNUMBER,
    i_ekSTRING,
    i_ekOPEN_ARRAY,
    i_ekCLOSE_ARRAY,
    i_ekOPEN_OBJECT,
    i_ekCLOSE_OBJECT,
    i_ekCOMMA,
    i_ekCOLON,
    i_ekUNKNOWN
} jtoken_t;

typedef struct i_parser_t i_Parser;

struct i_parser_t
{
    Stream *stm;
    jtoken_t token;
    bool_t minus;
    uint32_t col;
    uint32_t row;
    uint32_t lexsize;
    const char_t *lexeme;
    char_t number[128];
    ArrPt(String) *log;
};

/*---------------------------------------------------------------------------*/

static byte_t *i_create_type(i_Parser *parser, const DBind *bind, const DBind *ebind);
static bool_t i_jump_json_value(i_Parser *parser);
static bool_t i_parse_json_value(i_Parser *parser, const DBind *bind, const DBind *ebind, const bool_t is_str_dptr, byte_t *data, bool_t *null_readed);
static bool_t i_parse_json_object(i_Parser *parser, const DBind *stbind, byte_t *obj);
static void i_write_json_value(Stream *stm, const DBind *bind, const DBind *ebind, const byte_t *data);

/*---------------------------------------------------------------------------*/

static bool_t i_error(const bool_t cond, const bool_t fatal, i_Parser *parser, const char_t *errmsg)
{
    cassert_no_null(parser);

    if (cond == FALSE && parser->log != NULL)
    {
        String *msg = NULL;

        if (parser->lexeme != NULL && parser->lexsize < 128)
            msg = str_printf("JSON(%d:%d)-%s (%s).", parser->row, parser->col, errmsg, parser->lexeme);
        else
            msg = str_printf("JSON(%d:%d)-%s.", parser->row, parser->col, errmsg);

        arrpt_append(parser->log, msg, String);
    }

    if (cond == FALSE && fatal == TRUE)
        return FALSE;
    else
        return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_new_token(i_Parser *parser)
{
    ltoken_t token;
    cassert_no_null(parser);
    token = stm_read_token(parser->stm);
    parser->row = stm_token_col(parser->stm);
    parser->col = stm_token_row(parser->stm);
    parser->lexeme = stm_token_lexeme(parser->stm, &parser->lexsize);
    switch (token)
    {
    case ekTIDENT:
        if (str_equ_c(parser->lexeme, "true") == TRUE)
            parser->token = i_ekTRUE;
        else if (str_equ_c(parser->lexeme, "false") == TRUE)
            parser->token = i_ekFALSE;
        else if (str_equ_c(parser->lexeme, "null") == TRUE)
            parser->token = i_ekNULL;
        else
            parser->token = i_ekUNKNOWN;
        break;

    case ekTINTEGER:
    case ekTREAL:
        parser->token = i_ekNUMBER;
        if (parser->minus == TRUE)
        {
            bstd_sprintf(parser->number, sizeof(parser->number), "-%s", parser->lexeme);
            parser->minus = FALSE;
        }
        else
        {
            str_copy_c(parser->number, sizeof(parser->number), parser->lexeme);
        }
        break;

    case ekTSTRING:
        cassert(parser->lexeme[0] == '\"');
        cassert(parser->lexeme[parser->lexsize - 1] == '\"');
        ((char_t *)parser->lexeme)[parser->lexsize - 1] = '\0';
        parser->lexeme += 1;
        parser->lexsize -= 2;
        parser->token = i_ekSTRING;
        break;

    case ekTOPENBRAC:
        parser->token = i_ekOPEN_ARRAY;
        break;

    case ekTCLOSBRAC:
        parser->token = i_ekCLOSE_ARRAY;
        break;

    case ekTOPENCURL:
        parser->token = i_ekOPEN_OBJECT;
        break;

    case ekTCLOSCURL:
        parser->token = i_ekCLOSE_OBJECT;
        break;

    case ekTCOMMA:
        parser->token = i_ekCOMMA;
        break;

    case ekTCOLON:
        parser->token = i_ekCOLON;
        break;

    case ekTMINUS:
        cassert(parser->minus == FALSE);
        parser->minus = TRUE;
        i_new_token(parser);
        break;

    case ekTSLCOM:
    case ekTMLCOM:
    case ekTSPACE:
    case ekTEOL:
    case ekTLESS:
    case ekTGREAT:
    case ekTPERIOD:
    case ekTSCOLON:
    case ekTOPENPAR:
    case ekTCLOSPAR:
    case ekTPLUS:
    case ekTASTERK:
    case ekTEQUALS:
    case ekTDOLLAR:
    case ekTPERCENT:
    case ekTPOUND:
    case ekTAMPER:
    case ekTAPOST:
    case ekTQUOTE:
    case ekTCIRCUM:
    case ekTTILDE:
    case ekTEXCLA:
    case ekTQUEST:
    case ekTVLINE:
    case ekTSLASH:
    case ekTBSLASH:
    case ekTAT:
    case ekTOCTAL:
    case ekTHEX:
    case ekTUNDEF:
    case ekTCORRUP:
    case ekTEOF:
    case ekTRESERVED:
    default:
        parser->token = i_ekUNKNOWN;
        break;
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_jump_json_array(i_Parser *parser)
{
    bool_t ok = i_jump_json_value(parser);

    if (ok == FALSE)
    {
        /* Empty array */
        if (parser->token == i_ekCLOSE_ARRAY)
            return TRUE;
        else
            return i_error(FALSE, TRUE, parser, "Unexpected token jumping array");
    }

    for (;;)
    {
        i_new_token(parser);
        if (parser->token == i_ekCLOSE_ARRAY)
            return TRUE;

        if (parser->token != i_ekCOMMA)
            return i_error(FALSE, TRUE, parser, "Comma expected jumping array");

        ok = i_jump_json_value(parser);
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_jump_json_object(i_Parser *parser)
{
    bool_t comma_state = FALSE;
    /* For all object members */
    for (;;)
    {
        /* '}' */
        i_new_token(parser);
        if (parser->token == i_ekCLOSE_OBJECT)
        {
            if (comma_state == FALSE)
                return TRUE;
            else
                return i_error(FALSE, TRUE, parser, "Unexpected Json '}' (member opened)");
        }

        if (parser->token == i_ekCOMMA)
        {
            if (comma_state == FALSE)
            {
                comma_state = TRUE;
                continue;
            }
            else
            {
                return i_error(FALSE, TRUE, parser, "Unexpected Json ','");
            }
        }

        /* "member_name" */
        if (parser->token != i_ekSTRING)
            return i_error(FALSE, TRUE, parser, "Expected Json 'string' (member name)");

        /* ":" */
        i_new_token(parser);
        if (parser->token != i_ekCOLON)
            return i_error(FALSE, TRUE, parser, "Expected Json ':' (object member)");

        /* "member_value" */
        if (i_jump_json_value(parser) == TRUE)
            comma_state = FALSE;
        else
            return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_jump_json_value(i_Parser *parser)
{
    i_new_token(parser);
    switch (parser->token)
    {
    case i_ekTRUE:
    case i_ekFALSE:
    case i_ekNULL:
    case i_ekNUMBER:
    case i_ekSTRING:
        return TRUE;
    case i_ekOPEN_ARRAY:
        return i_jump_json_array(parser);
    case i_ekOPEN_OBJECT:
        return i_jump_json_object(parser);
    case i_ekCLOSE_ARRAY:
        return FALSE;
    case i_ekCLOSE_OBJECT:
        return i_error(FALSE, TRUE, parser, "Unexpected Json token '}'");
    case i_ekCOMMA:
        return i_error(FALSE, TRUE, parser, "Unexpected Json token ','");
    case i_ekCOLON:
        return i_error(FALSE, TRUE, parser, "Unexpected Json token ':'");
    case i_ekUNKNOWN:
        return i_error(FALSE, TRUE, parser, "Unknown Json token");
        cassert_default();
    }

    parser->minus = FALSE;
    return i_error(FALSE, TRUE, parser, "Fatal Json parsing error");
}

/*---------------------------------------------------------------------------*/

static bool_t i_parse_json_array(i_Parser *parser, const DBind *bind, const DBind *ebind, byte_t *cont)
{
    bool_t ok = FALSE;
    byte_t *data = dbind_container_append(bind, ebind, cont);

    dbind_init_data(ebind, data);
    ok = i_parse_json_value(parser, ebind, NULL, FALSE, data, NULL);

    if (ok == FALSE)
    {
        /* Empty array, we have to remove the first element added to parse value */
        if (parser->token == i_ekCLOSE_ARRAY)
        {
            uint32_t s = dbind_container_size(bind, cont);
            byte_t *elem = dbind_container_get(bind, ebind, s - 1, cont);
            dbind_remove_data(elem, ebind);
            dbind_container_delete(bind, ebind, s - 1, cont);
            return TRUE;
        }
        else
        {
            dbind_set_value_null(ebind, NULL, FALSE, data);
        }
    }

    /* Read elements from 1 to n */
    for (;;)
    {
        i_new_token(parser);

        /* No more elements, end of array */
        if (parser->token == i_ekCLOSE_ARRAY)
            return TRUE;

        if (parser->token != i_ekCOMMA)
            return i_error(FALSE, TRUE, parser, "Comma expected in 'array'");

        data = dbind_container_append(bind, ebind, cont);
        dbind_init_data(ebind, data);
        ok = i_parse_json_value(parser, ebind, NULL, FALSE, data, NULL);
        if (ok == FALSE)
            dbind_set_value_null(ebind, NULL, FALSE, data);
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_parse_json_arrpt(i_Parser *parser, const DBind *bind, const DBind *ebind, byte_t *cont)
{
    byte_t *data = i_create_type(parser, ebind, NULL);

    if (data == NULL)
    {
        /* Empty array */
        if (parser->token == i_ekCLOSE_ARRAY)
            return TRUE;
        else
            i_error(FALSE, FALSE, parser, "Element can't be readed from 'array'");
    }

    for (;;)
    {
        byte_t *elem = dbind_container_append(bind, ebind, cont);
        *dcast(elem, byte_t) = data;

        i_new_token(parser);
        if (parser->token == i_ekCLOSE_ARRAY)
            return TRUE;

        if (parser->token != i_ekCOMMA)
            return i_error(FALSE, TRUE, parser, "Comma expected in 'array'");

        data = i_create_type(parser, ebind, NULL);
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_parse_json_value(i_Parser *parser, const DBind *bind, const DBind *ebind, const bool_t is_str_dptr, byte_t *data, bool_t *null_readed)
{
    dtype_t type = dbind_type(bind);
    bindset_t rset = ekBINDSET_NOT_ALLOWED;
    ptr_assign(null_readed, FALSE);
    i_new_token(parser);
    switch (parser->token)
    {
    case i_ekTRUE:
        rset = dbind_set_value_bool(bind, data, TRUE);
        return i_error(rset != ekBINDSET_NOT_ALLOWED, TRUE, parser, "Unexpected JSON 'true'");

    case i_ekFALSE:
        rset = dbind_set_value_bool(bind, data, FALSE);
        return i_error(rset != ekBINDSET_NOT_ALLOWED, TRUE, parser, "Unexpected JSON 'false'");

    case i_ekNULL:
        ptr_assign(null_readed, TRUE);
        rset = dbind_set_value_null(bind, ebind, is_str_dptr, data);
        return i_error(rset != ekBINDSET_NOT_ALLOWED, TRUE, parser, "Unexpected JSON 'null'");

    case i_ekNUMBER:
    {
        bool_t err;
        real64_t value = str_to_r64(parser->number, &err);
        if (err == FALSE)
        {
            rset = dbind_set_value_real(bind, data, value);
            if (rset == ekBINDSET_NOT_ALLOWED)
                err = TRUE;
        }
        return i_error(!err, TRUE, parser, "Unexpected JSON 'number'");
    }

    case i_ekSTRING:
        /* Binary objects are interpreted as B64 strings */
        if (type == ekDTYPE_BINARY)
        {
            uint32_t dsize = b64_decoded_size(parser->lexsize);
            byte_t *b64 = heap_malloc(dsize, "JsonB64Decode");
            uint32_t b64size = b64_decode(parser->lexeme, parser->lexsize, b64);
            rset = dbind_create_value_binary(bind, data, b64, b64size);
            heap_free(&b64, dsize, "JsonB64Decode");
        }
        else
        {
            rset = dbind_set_value_str(bind, data, parser->lexeme);
        }
        return i_error(rset != ekBINDSET_NOT_ALLOWED, TRUE, parser, "Unexpected JSON 'string'");

    case i_ekOPEN_ARRAY:
        if (type == ekDTYPE_CONTAINER)
        {
            byte_t *cont = *dcast(data, byte_t);
            if (cont != NULL)
                cassert(dbind_container_size(bind, cont) == 0);
            if (dbind_container_is_ptr(bind) == TRUE)
                return i_parse_json_arrpt(parser, bind, ebind, cont);
            else
                return i_parse_json_array(parser, bind, ebind, cont);
        }
        else
        {
            bool_t err = i_error(FALSE, TRUE, parser, "Unexpected Json '['");
            i_jump_json_array(parser);
            return err;
        }

    case i_ekOPEN_OBJECT:
        if (type == ekDTYPE_STRUCT)
        {
            if (is_str_dptr == TRUE)
            {
                if (*dcast(data, byte_t) == NULL)
                    *dcast(data, byte_t) = dbind_create_data(bind, ebind);
                return i_parse_json_object(parser, bind, *dcast(data, byte_t));
            }
            else
            {
                return i_parse_json_object(parser, bind, data);
            }
        }
        else
        {
            bool_t err = i_error(FALSE, TRUE, parser, "Unexpected Json '{'");
            i_jump_json_object(parser);
            return err;
        }

    case i_ekCLOSE_ARRAY:
        return FALSE;

    case i_ekCLOSE_OBJECT:
        return i_error(FALSE, TRUE, parser, "Unexpected Json token '}'");

    case i_ekCOMMA:
        return i_error(FALSE, TRUE, parser, "Unexpected Json token ','");

    case i_ekCOLON:
        return i_error(FALSE, TRUE, parser, "Unexpected Json token ':'");

    case i_ekUNKNOWN:
        return i_error(FALSE, TRUE, parser, "Unknown Json token");
        cassert_default();
    }

    return i_error(FALSE, TRUE, parser, "Fatal Json parsing error");
}

/*---------------------------------------------------------------------------*/

static bool_t i_parse_json_object(i_Parser *parser, const DBind *stbind, byte_t *obj)
{
    bool_t comma_state = FALSE;
    /* For all object members */
    for (;;)
    {
        uint32_t member_id = UINT32_MAX;

        i_new_token(parser);

        /* '}' */
        if (parser->token == i_ekCLOSE_OBJECT)
        {
            if (comma_state == FALSE)
                return TRUE;
            else
                return i_error(FALSE, TRUE, parser, "Unexpected Json '}' (member opened)");
        }

        /* ',' */
        if (parser->token == i_ekCOMMA)
        {
            if (comma_state == FALSE)
            {
                comma_state = TRUE;
                continue;
            }
            else
            {
                return i_error(FALSE, TRUE, parser, "Unexpected Json ','");
            }
        }

        /* "member_name" */
        if (parser->token != i_ekSTRING)
            return i_error(FALSE, TRUE, parser, "Expected Json 'string' (member name)");

        member_id = dbind_st_member_id(stbind, parser->lexeme);

        /* ":" */
        i_new_token(parser);
        if (parser->token != i_ekCOLON)
            return i_error(FALSE, TRUE, parser, "Expected Json ':' (object member)");

        /* "member_value" */
        if (member_id != UINT32_MAX)
        {
            const DBind *bind = dbind_st_member(stbind, member_id);
            const DBind *ebind = dbind_st_ebind(stbind, member_id);
            bool_t is_str_dptr = dbind_st_is_str_dptr(stbind, member_id);
            byte_t *data = obj + dbind_st_offset(stbind, member_id);
            bool_t null_readed = FALSE;
            if (i_parse_json_value(parser, bind, ebind, is_str_dptr, data, &null_readed) == FALSE)
                dbind_st_set_value_null(stbind, member_id, obj);
            else if (null_readed == TRUE)
                dbind_st_set_value_null(stbind, member_id, obj);
            comma_state = FALSE;
        }
        else
        {
            /* This json field doen't exists in dbind struct --> Jump the json value */
            if (i_jump_json_value(parser) == TRUE)
                comma_state = FALSE;
            else
                return FALSE;
        }
    }
}

/*---------------------------------------------------------------------------*/

static byte_t *i_create_type(i_Parser *parser, const DBind *bind, const DBind *ebind)
{
    byte_t *data = dbind_create_data(bind, ebind);
    dtype_t dtype = dbind_type(bind);
    byte_t *cdata = NULL;

    switch (dtype)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRUCT:
        cdata = data;
        break;

    case ekDTYPE_STRING:
        cassert_no_null(data);
        cdata = cast(&data, byte_t);
        break;

    case ekDTYPE_CONTAINER:
        if (data != NULL)
            cdata = cast(&data, byte_t);
        break;

    case ekDTYPE_BINARY:
        cassert(data == NULL);
        cdata = cast(&data, byte_t);
        break;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    if (cdata != NULL)
    {
        bool_t null_readed = FALSE;
        if (i_parse_json_value(parser, bind, ebind, FALSE, cdata, &null_readed) == FALSE)
        {
            if (data != NULL)
                dbind_destroy_data(&data, bind, ebind);
        }
        else if (null_readed == TRUE)
        {
            if (data != NULL)
                dbind_destroy_data(&data, bind, ebind);
        }
    }
    else
    {
        cassert(data == NULL);
    }

    return data;
}

/*---------------------------------------------------------------------------*/

static void i_bind_from_typename(const char_t *type, const DBind **bind, const DBind **ebind)
{
    cassert_no_null(bind);
    cassert_no_null(ebind);
    *bind = dbind_from_typename(type, NULL);
    *ebind = NULL;
    if (dbind_type(*bind) == ekDTYPE_CONTAINER)
        *ebind = dbind_container_type(*bind, type);
}

/*---------------------------------------------------------------------------*/

void *json_read_imp(Stream *stm, const JsonOpts *opts, const char_t *type)
{
    i_Parser parser;
    const DBind *bind = NULL;
    const DBind *ebind = NULL;
    parser.stm = stm;
    stm_token_escapes(parser.stm, TRUE);
    stm_skip_bom(parser.stm);
    parser.col = 0;
    parser.row = 0;
    parser.lexeme = NULL;
    parser.lexsize = 0;
    parser.minus = FALSE;
    parser.log = opts ? opts->log : NULL;
    i_bind_from_typename(type, &bind, &ebind);
    return i_create_type(&parser, bind, ebind);
}

/*---------------------------------------------------------------------------*/

void *json_read_str_imp(const char_t *str, const JsonOpts *opts, const char_t *type)
{
    uint32_t size = 0;
    Stream *stm = NULL;
    void *obj = NULL;
    cassert_no_null(str);
    size = str_len_c(str);
    stm = stm_from_block(cast_const(str, byte_t), size);
    if (stm != NULL)
    {
        obj = json_read_imp(stm, opts, type);
        stm_close(&stm);
    }

    return obj;
}

/*---------------------------------------------------------------------------*/

static void i_write_escape_str(Stream *stm, const char_t *cstr)
{
    uint32_t cp = unicode_to_u32(cstr, ekUTF8);
    stm_writef(stm, "\"");
    while (cp != 0)
    {
        if (cp == '"')
            stm_writef(stm, "\\\"");
        else if (cp == '\\')
            stm_writef(stm, "\\\\");
        /* else if (cp == '/')
            stm_writef(stm, "\\/"); */
        else if (cp == '\b')
            stm_writef(stm, "\\b");
        else if (cp == '\f')
            stm_writef(stm, "\\f");
        else if (cp == '\n')
            stm_writef(stm, "\\n");
        else if (cp == '\r')
            stm_writef(stm, "\\r");
        else if (cp == '\t')
            stm_writef(stm, "\\t");
        else if (cp >= 32)
            stm_write_char(stm, cp);
        cstr = unicode_next(cstr, ekUTF8);
        cp = unicode_to_u32(cstr, ekUTF8);
    }
    stm_writef(stm, "\"");
}

/*---------------------------------------------------------------------------*/

static void i_write_container(Stream *stm, const DBind *bind, const DBind *ebind, const byte_t *data)
{
    const byte_t *cont = data;
    uint32_t i, n = dbind_container_size(bind, cont);
    stm_writef(stm, "[ ");
    for (i = 0; i < n; ++i)
    {
        const byte_t *elem = dbind_container_cget(bind, ebind, i, cont);
        i_write_json_value(stm, ebind, NULL, elem);
        if (i < n - 1)
            stm_writef(stm, ", ");
    }
    stm_writef(stm, " ]");
}

/*---------------------------------------------------------------------------*/

static void i_write_object(Stream *stm, const DBind *stbind, const byte_t *obj)
{
    uint32_t i, n = dbind_st_count(stbind);
    cassert_no_null(obj);
    stm_writef(stm, "{");
    for (i = 0; i < n; ++i)
    {
        const DBind *mbind = dbind_st_member(stbind, i);
        const DBind *mebind = dbind_st_ebind(stbind, i);
        const char_t *mname = dbind_st_mname(stbind, i);
        const byte_t *mdata = obj + dbind_st_offset(stbind, i);
        bool_t is_str_dptr = dbind_st_is_str_dptr(stbind, i);
        dtype_t type = dbind_type(mbind);

        stm_printf(stm, "\"%s\" : ", mname);

        if (type == ekDTYPE_STRING || type == ekDTYPE_BINARY || type == ekDTYPE_CONTAINER || is_str_dptr)
            mdata = *dcast_const(mdata, byte_t);

        i_write_json_value(stm, mbind, mebind, mdata);
        if (i < n - 1)
            stm_writef(stm, ", ");
    }
    stm_writef(stm, " }");
}

/*---------------------------------------------------------------------------*/

static void i_write_binary(Stream *stm, const DBind *bind, const byte_t *data)
{
    Stream *objstm = stm_memory(1024);
    const byte_t *stmdata = NULL;
    uint32_t stmsize = 0;
    dbind_write_binary_value(bind, objstm, data);
    stmdata = stm_buffer(objstm);
    stmsize = stm_buffer_size(objstm);

    if (stmsize > 0)
    {
        uint32_t b64size = b64_encoded_size(stmsize);
        char_t *b64data = cast(heap_malloc(b64size, "JsonB64Encode"), char_t);
        b64_encode(stmdata, stmsize, b64data, b64size);
        stm_writef(stm, "\"");
        stm_writef(stm, b64data);
        stm_writef(stm, "\"");
        heap_free(dcast(&b64data, byte_t), b64size, "JsonB64Encode");
    }
    else
    {
        stm_writef(stm, "\"\"");
    }

    stm_close(&objstm);
}

/*---------------------------------------------------------------------------*/

static void i_write_json_value(Stream *stm, const DBind *bind, const DBind *ebind, const byte_t *data)
{
    if (data != NULL)
    {
        dtype_t type = dbind_type(bind);
        switch (type)
        {
        case ekDTYPE_BOOL:
        {
            bool_t value = dbind_get_bool_value(bind, data);
            stm_writef(stm, value ? "true" : "false");
            break;
        }

        case ekDTYPE_INT:
        {
            int64_t value = dbind_get_int_value(bind, data);
            stm_printf(stm, "%" PRId64, value);
            break;
        }

        case ekDTYPE_REAL:
        {
            real64_t value = dbind_get_real_value(bind, data);
            stm_printf(stm, "%g", value);
            break;
        }

        case ekDTYPE_ENUM:
        {
            enum_t value = dbind_get_enum_value(bind, data);
            uint32_t index = dbind_enum_index(bind, value);
            const char_t *alias = dbind_enum_alias(bind, index);
            i_write_escape_str(stm, alias);
            break;
        }

        case ekDTYPE_STRING:
        {
            const char_t *cstr = dbind_get_str_value(bind, data);
            i_write_escape_str(stm, cstr);
            break;
        }

        case ekDTYPE_STRUCT:
            i_write_object(stm, bind, data);
            break;

        case ekDTYPE_BINARY:
            i_write_binary(stm, bind, data);
            break;

        case ekDTYPE_CONTAINER:
            i_write_container(stm, bind, ebind, data);
            break;

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    }
    else
    {
        stm_writef(stm, "null");
    }
}

/*---------------------------------------------------------------------------*/

void json_write_imp(Stream *stm, const void *data, const JsonOpts *opts, const char_t *type)
{
    const DBind *bind = NULL;
    const DBind *ebind = NULL;
    i_bind_from_typename(type, &bind, &ebind);
    i_write_json_value(stm, bind, ebind, data);
    stm_write_char(stm, 0);
    unref(opts);
}

/*---------------------------------------------------------------------------*/

String *json_write_str_imp(const void *data, const JsonOpts *opts, const char_t *type)
{
    Stream *stm = stm_memory(1024);
    String *str = NULL;
    json_write_imp(stm, data, opts, type);
    str = stm_str(stm);
    stm_close(&stm);
    return str;
}

/*---------------------------------------------------------------------------*/

void json_destroy_imp(void **data, const char_t *type)
{
    const DBind *bind = NULL;
    const DBind *ebind = NULL;
    i_bind_from_typename(type, &bind, &ebind);
    dbind_destroy_data(dcast(data, byte_t), bind, ebind);
}

/*---------------------------------------------------------------------------*/

void json_destopt_imp(void **data, const char_t *type)
{
    cassert_no_null(data);
    if (*data != NULL)
        json_destroy_imp(data, type);
}
