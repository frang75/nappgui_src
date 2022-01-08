/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: blib.inl
 *
 */

/* C library funcions */

#include "sewer.hxx"

__EXTERN_C

uint32_t blib_strlen(const char_t *str);

char_t* blib_strstr(const char_t *str, const char_t *substr);

void blib_strcpy(char_t *dest, const uint32_t size, const char_t *src);

void blib_strncpy(char_t *dest, const uint32_t size, const char_t *src, const uint32_t n);

void blib_strcat(char_t *dest, const uint32_t size, const char_t *src);

int blib_strcmp(const char_t *str1, const char_t *str2);

int blib_strncmp(const char_t *str1, const char_t *str2, const uint32_t n);

int64_t blib_strtol(const char_t* str, char_t** endptr, uint32_t base, bool_t *err);

uint64_t blib_strtoul(const char_t* str, char_t** endptr, uint32_t base, bool_t *err);

real32_t blib_strtof(const char_t* str, char_t** endptr, bool_t *err);

real64_t blib_strtod(const char_t* str, char_t** endptr, bool_t *err);

void blib_qsort(byte_t *array, const uint32_t nelems, const uint32_t size, FPtr_compare func_compare);

void blib_qsort_ex(const byte_t *array, const uint32_t nelems, const uint32_t size, FPtr_compare_ex func_compare, const byte_t *data);

bool_t blib_bsearch(const byte_t *array, const byte_t *key, const uint32_t nelems, const uint32_t size, FPtr_compare func_compare, uint32_t *pos);

bool_t blib_bsearch_ex(const byte_t *array, const byte_t *key, const uint32_t nelems, const uint32_t size, FPtr_compare_ex func_compare, const byte_t *data, uint32_t *pos);

__END_C
