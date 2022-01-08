/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: msgparser.c
 *
 */

/* Message files parser */

#include "msgparser.inl"
#include "arrpt.h"
#include "cassert.h"
#include "strings.h"
#include "unicode.h"

/*---------------------------------------------------------------------------*/

static void i_jump_BOM(const char_t **cp)
{
    cassert_no_null(cp);
    if ((uint8_t)((*cp)[0]) == 0xEF && (uint8_t)((*cp)[1]) == 0xBB && (uint8_t)((*cp)[2]) == 0xBF)
        *cp += 3;
}

/*---------------------------------------------------------------------------*/

static void i_jump_blanks_and_comments(const char_t **cp, const char_t *eof, uint32_t *line, const char_t *filepath, ArrPt(String) *errors)
{
    bool_t cont = TRUE;
    cassert_no_null(cp);
    cassert_no_null(line);
    while ((*cp < eof) && cont == TRUE)
    {
        if (unicode_isspace((uint32_t)(*cp)[0]) == TRUE)
        {
            if ((*cp)[0] == '\n')
                *line += 1;
            *cp += 1;
        }
        else if (*cp + 2 < eof)
        {
            /* C++ Style comment */
            if ((*cp)[0] == '/' && (*cp)[1] == '/')
            {
                *cp += 2;
                while (*cp < eof)
                {
                    if ((*cp)[0] == '\n')
                        break;
                    else
                        *cp += 1;
                }
            }
            /* Classic C comment */
            else if ((*cp)[0] == '/' && (*cp)[1] == '*')
            {
                *cp += 2;
                while (*cp < eof)
                {
                    if ((*cp)[0] == '*')
                    {
                        if (*cp + 1 < eof && (*cp)[1] == '/')
                        {
                            *cp += 2;
                            break;
                        }
                    }

                    *cp += 1;
                }

                if (*cp == eof)
                {
                    const char_t *filename = str_filename(filepath);
                    String *error = str_printf("MsgError (%s:%d): Comment not closed (%s).", filename, *line, filepath);
                    arrpt_append(errors, error, String);                    
                }
            }
            else
            {
                cont = FALSE;
            }
        }
        else
        {
            cont = FALSE;
        }
    }
}

/*---------------------------------------------------------------------------*/

static String *i_read_id(const char_t **cp, const char_t *eof, uint32_t *line, const char_t *filepath, ArrPt(String) *errors)
{
    const char_t *st = NULL;
    cassert_no_null(cp);
    cassert_no_null(line);
    st = *cp;
    if (unicode_isupper((uint32_t)(*cp)[0]) || (uint32_t)(*cp)[0] == '_')
    {
        *cp += 1;
        while (*cp < eof)
        {
            if (unicode_isupper((uint32_t)(*cp)[0])
            || (uint32_t)(*cp)[0] == '_'
            || unicode_isdigit((uint32_t)(*cp)[0]))
            {
                *cp += 1;
            }
            else if (unicode_isspace((uint32_t)(*cp)[0]) == TRUE)
            {
                return str_cn(st, (uint32_t)(*cp - st));
            }
            else
            {
                const char_t *filename = str_filename(filepath);
                String *error = str_printf("MsgError (%s:%d): Invalid TEXT_ID (%s).", filename, *line, filepath);                
                arrpt_append(errors, error, String);
                return NULL;
            }
        }

        return str_cn(st, (uint32_t)(*cp - st));
    }
    else
    {
        const char_t *filename = str_filename(filepath);
        String *error = str_printf("MsgError (%s:%d): Invalid TEXT_ID (%s).", filename, *line, filepath);
        arrpt_append(errors, error, String);
    }
     
    return NULL;       
}

/*---------------------------------------------------------------------------*/

static String *i_read_text(const char_t **cp, const char_t *eof)
{
    const char_t *st = NULL;
    cassert_no_null(cp);
    st = *cp;
    while (*cp[0] != '\n' && *cp[0] != '\r' && *cp < eof)
        *cp += 1;
    return str_cn(st, (uint32_t)(*cp - st));
}

/*---------------------------------------------------------------------------*/

void msgparser_process(const char_t *filepath, const char_t *file_data, const uint32_t file_size, ArrPt(String) **ids, ArrPt(String) **texts, ArrPt(String) *errors)
{
    const char_t *cp = NULL;
    const char_t *eof = NULL;
    uint32_t line = 1;
    cassert_no_null(file_data);
    cassert_no_null(ids);
    cassert_no_null(texts);
    *ids = arrpt_create(String);
    *texts = arrpt_create(String);
    cp = file_data;
    eof = file_data + file_size;

    if (file_size < 3)
        return;

    i_jump_BOM(&cp);

    while (cp < eof)
    {
        String *id = NULL;
        String *text = NULL;

        i_jump_blanks_and_comments(&cp, eof, &line, filepath, errors);

        if (cp == eof)
            return;

        id = i_read_id(&cp, eof, &line, filepath, errors);
        if (id == NULL)
            return;

        i_jump_blanks_and_comments(&cp, eof, &line, filepath, errors);

        if (cp == eof)
        {
            const char_t *filename = str_filename(filepath);
            String *error = str_printf("MsgError (%s:%d): Unexpected end of file after string ID (%s).", filename, line, filepath);
            arrpt_append(errors, error, String);
            str_destroy(&id);
            return;
        }

        text = i_read_text(&cp, eof);
        if (text == NULL)
        {
            str_destroy(&id);
            return;
        }

        arrpt_append(*ids, id, String);
        arrpt_append(*texts, text, String);
    }
}

