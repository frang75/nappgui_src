/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: lex.inl
 *
 */

/* Lexical scanner */

#include "core.ixx"

__EXTERN_C

LexScn *lexscn_create(void);

void lexscn_destroy(LexScn **lex);

void lexscn_spaces(LexScn *lex, const bool_t activate);

void lexscn_newlines(LexScn *lex, const bool_t activate);

void lexscn_escapes(LexScn *lex, const bool_t activate);

void lexscn_comments(LexScn *lex, const bool_t activate);

ltoken_t lexscn_token(LexScn *lex, Stream *stm);

uint32_t lexscn_row(const LexScn *lex);

uint32_t lexscn_col(const LexScn *lex);

const char_t *lexscn_lexeme(const LexScn *lex, uint32_t *size);

const char_t *lexscn_string(const ltoken_t token);

__END_C
