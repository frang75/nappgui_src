/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nlib.hxx
 *
 */

/* Commons for NAppGUI Utilities */

#ifndef __NLIB_HXX__
#define __NLIB_HXX__

#include "core.hxx"

typedef struct _login_t Login;

struct _login_t
{
    String *ip;
    String *user;
    String *pass;
    platform_t platform;
};

#endif
