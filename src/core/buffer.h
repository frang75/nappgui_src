/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: buffer.h
 * https://nappgui.com/en/core/buffer.html
 *
 */

/* Fixed size memory buffers */

#include "core.hxx"

__EXTERN_C

Buffer *buffer_create(const uint32_t size);

Buffer *buffer_with_data(const byte_t *data, const uint32_t size);

void buffer_destroy(Buffer **buffer);

uint32_t buffer_size(const Buffer *buffer);

byte_t *buffer_data(Buffer *buffer);

const byte_t *buffer_const(const Buffer *buffer);

__END_C

