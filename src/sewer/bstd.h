/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bstd.h
 * https://nappgui.com/en/sewer/bstd.html
 *
 */

/* Basic standard functions */

#include "sewer.hxx"

__EXTERN_C

uint32_t bstd_sprintf(char_t *str, const uint32_t size, const char_t *format, ...) __PRINTF(3, 4);

uint32_t bstd_vsprintf(char_t *str, const uint32_t size, const char_t *format, va_list args);

uint32_t bstd_printf(const char_t *format, ...) __PRINTF(1, 2);

uint32_t bstd_eprintf(const char_t *format, ...) __PRINTF(1, 2);

uint32_t bstd_writef(const char_t *str);

uint32_t bstd_ewritef(const char_t *str);

bool_t bstd_read(byte_t *data, const uint32_t size, uint32_t *rsize);

bool_t bstd_write(const byte_t *data, const uint32_t size, uint32_t *wsize);

bool_t bstd_ewrite(const byte_t *data, const uint32_t size, uint32_t *wsize);

void bstd_abort(void);

void bstd_debug_break(void);

__END_C

