/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: strings.h
 * https://nappgui.com/en/core/strings.html
 *
 */

/* UTF8 strings */

#include "core.hxx"

__EXTERN_C

const char_t *tc(const String *str);

char_t *tcc(String *str);

String *str_c(const char_t *str);

String *str_cn(const char_t *str, const uint32_t n);

String *str_trim(const char_t *str);

String *str_trim_n(const char_t *str, const uint32_t n);

String *str_copy(const String *str);

String *str_printf(const char_t *format, ...) __PRINTF(1, 2);

String *str_path(const platform_t platform, const char_t *format, ...) __PRINTF(2, 3);

String *str_cpath(const char_t *format, ...) __PRINTF(1, 2);

String *str_relpath(const char_t *path1, const char_t *path2);

String *str_repl(const char_t *str, ...);

String *str_reserve(const uint32_t n);

String *str_fill(const uint32_t n, const char_t c);

String *str_read(Stream *stream);

void str_write(Stream *stream, const String *str);

void str_writef(Stream *stream, const String *str);

void str_copy_c(char_t *dest, const uint32_t size, const char_t *src);

void str_copy_cn(char_t *dest, const uint32_t size, const char_t *src, const uint32_t n);

void str_cat(String **dest, const char_t *src);

void str_cat_c(char_t *dest, const uint32_t size, const char_t *src);

void str_upd(String **str, const char_t *new_str);

void str_destroy(String **str);

void str_destopt(String **str);

uint32_t str_len(const String *str);

uint32_t str_len_c(const char_t *str);

uint32_t str_nchars(const String *str);

uint32_t str_prefix(const char_t *str1, const char_t *str2);

bool_t str_is_prefix(const char_t *str, const char_t *prefix);

bool_t str_is_sufix(const char_t *str, const char_t *sufix);

int str_scmp(const String *str1, const String *str2);

int str_cmp(const String *str1, const char_t *str2);

int str_cmp_c(const char_t *str1, const char_t *str2);

int str_cmp_cn(const char_t *str1, const char_t *str2, const uint32_t n);

bool_t str_empty(const String *str);

bool_t str_empty_c(const char_t *str);

bool_t str_equ(const String *str1, const char_t *str2);

bool_t str_equ_c(const char_t *str1, const char_t *str2);

bool_t str_equ_cn(const char_t *str1, const char_t *str2, const uint32_t n);

bool_t str_equ_nocase(const char_t *str1, const char_t *str2);

bool_t str_equ_end(const char_t *str, const char_t *end);

void str_upper(String *str);

void str_lower(String *str);

void str_upper_c(char_t *dest, const uint32_t size, const char_t *str);

void str_lower_c(char_t *dest, const uint32_t size, const char_t *str);

void str_subs(String *str, const char_t replace, const char_t with);

void str_repl_c(char_t *str, const char_t *replace, const char_t *with);

const char_t *str_str(const char_t *str, const char_t *substr);

bool_t str_split(const char_t *str, const char_t *substr, String **left, String **right);

bool_t str_split_trim(const char_t *str, const char_t *substr, String **left, String **right);

void str_split_pathname(const char_t *pathname, String **path, String **file);

void str_split_pathext(const char_t *pathname, String **path, String **file, String **ext);

const char_t *str_filename(const char_t *pathname);

const char_t *str_filext(const char_t *pathname);

uint32_t str_find(const ArrPt(String) *array, const char_t *str);

int8_t str_to_i8(const char_t *str, const uint32_t base, bool_t *error);

int16_t str_to_i16(const char_t *str, const uint32_t base, bool_t *error);

int32_t str_to_i32(const char_t *str, const uint32_t base, bool_t *error);

int64_t str_to_i64(const char_t *str, const uint32_t base, bool_t *error);

uint8_t str_to_u8(const char_t *str, const uint32_t base, bool_t *error);

uint16_t str_to_u16(const char_t *str, const uint32_t base, bool_t *error);

uint32_t str_to_u32(const char_t *str, const uint32_t base, bool_t *error);

uint64_t str_to_u64(const char_t *str, const uint32_t base, bool_t *error);

real32_t str_to_r32(const char_t *str, bool_t *error);

real64_t str_to_r64(const char_t *str, bool_t *error);

__END_C
