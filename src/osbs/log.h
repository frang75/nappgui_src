/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: log.h
 * https://nappgui.com/en/osbs/log.html
 *
 */

/* Manages an application log */

#include "osbs.hxx"

__EXTERN_C

uint32_t log_printf(const char_t *format, ...) __PRINTF(1, 2);

void log_output(const bool_t std, const bool_t err);

void log_file(const char_t *pathname);

const char_t *log_get_file(void);

__END_C
