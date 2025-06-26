/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: base64.h
 * https://nappgui.com/en/encode/base64.html
 *
 */

/* Base64 encoding */

#include "encode.hxx"

__EXTERN_C

_encode_api uint32_t b64_encoded_size(const uint32_t data_size);

_encode_api uint32_t b64_decoded_size(const uint32_t encoded_size);

_encode_api uint32_t b64_encode(const byte_t *data, const uint32_t size, char_t *base64, const uint32_t esize);

_encode_api uint32_t b64_decode(const char_t *base64, const uint32_t size, byte_t *data);

_encode_api String *b64_encode_from_stm(Stream *stm);

_encode_api String *b64_encode_from_file(const char_t *pathname, ferror_t *error);

_encode_api String *b64_encode_from_data(const byte_t *data, const uint32_t size);

_encode_api String *b64_encode_from_str(const String *str);

_encode_api String *b64_encode_from_cstr(const char_t *str);

_encode_api Buffer *b64_decode_from_str(const String *base64);

_encode_api Buffer *b64_decode_from_data(const byte_t *data, const uint32_t size);

__END_C
