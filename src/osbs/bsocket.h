/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bsocket.h
 * https://nappgui.com/en/osbs/bsocket.html
 *
 */

/* Sockets base API */

#include "osbs.hxx"

__EXTERN_C

Socket *bsocket_connect(const uint32_t ip, const uint16_t port, const uint32_t timeout_ms, serror_t *error);

Socket *bsocket_server(const uint16_t port, const uint32_t max_connect, serror_t *error);

Socket *bsocket_accept(Socket *socket, const uint32_t timeout_ms, serror_t *error);

void bsocket_close(Socket **socket);

void bsocket_local_ip(Socket *socket, uint32_t *ip, uint16_t *port);

void bsocket_remote_ip(Socket *socket, uint32_t *ip, uint16_t *port);

void bsocket_read_timeout(Socket *socket, const uint32_t timeout_ms);

void bsocket_write_timeout(Socket *socket, const uint32_t timeout_ms);

bool_t bsocket_read(Socket *socket, byte_t *data, const uint32_t size, uint32_t *rsize, serror_t *error);

bool_t bsocket_write(Socket *socket, const byte_t *data, const uint32_t size, uint32_t *wsize, serror_t *error);


uint32_t bsocket_url_ip(const char_t *url, serror_t *error);

uint32_t bsocket_str_ip(const char_t *ip);

const char_t* bsocket_ip_str(const uint32_t ip);

void bsocket_hton2(byte_t *dest, const byte_t *src);

void bsocket_hton4(byte_t *dest, const byte_t *src);

void bsocket_hton8(byte_t *dest, const byte_t *src);

void bsocket_ntoh2(byte_t *dest, const byte_t *src);

void bsocket_ntoh4(byte_t *dest, const byte_t *src);

void bsocket_ntoh8(byte_t *dest, const byte_t *src);

__END_C
