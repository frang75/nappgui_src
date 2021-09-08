/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: respackh.h
 * https://nappgui.com/en/core/respackh.html
 *
 */

/* Resource Packages */

#include "core.hxx"

__EXTERN_C

ResPack *respack_embedded(const char_t *name);

ResPack *respack_packed(const char_t *name, const char_t *locale);

void respack_add_msg(ResPack *pack, const char_t *msg);

void respack_add_cdata(ResPack *pack, const uint32_t type, const byte_t *data, const uint32_t data_size);

__END_C

    