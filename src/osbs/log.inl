/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: log.inl
 *
 */

/* Manages an application log */

#include "osbs.hxx"

__EXTERN_C

void _log_start(void);

void _log_finish(void);

void _log_printf_imp(const bool_t out_std, const bool_t err_std, const char_t *filepath, const char_t *format, va_list args);

__END_C
