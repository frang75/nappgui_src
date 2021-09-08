/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: stream.inl
 *
 */

/* Data streams */

#include "core.ixx"

__EXTERN_C

void _stm_start(void);

void _stm_finish(void);

void _stm_restore(Stream *stm, const byte_t *data, const uint32_t size);

void _stm_restore_col(Stream *stm, const uint32_t col);

void _stm_restore_row(Stream *stm, const uint32_t row);

bool_t _stm_memory(const Stream *stm);

uint32_t _stm_get_roffset(const Stream *stm);

void _stm_set_roffset(Stream *stm, const uint32_t offset);

__END_C
    
