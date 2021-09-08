/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: url.h
 * https://nappgui.com/en/inet/url.html
 *
 */

/* URL parser */

#include "inet.hxx"

__EXTERN_C

Url *url_parse(const char_t *url);

void url_destroy(Url **url);

const char_t *url_scheme(const Url *url);

const char_t *url_user(const Url *url);

const char_t *url_pass(const Url *url);

const char_t *url_host(const Url *url);

const char_t *url_path(const Url *url);

const char_t *url_params(const Url *url);

const char_t *url_query(const Url *url);

const char_t *url_fragment(const Url *url);

String *url_resource(const Url *url);

uint16_t url_port(const Url *url);

__END_C
