/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: inet.cpp
 *
 */

/* inet library */

#include "inet.h"
#include "inet.inl"
#include "core.h"
#include "oshttpreq.inl"
#include "cassert.h"
#include "log.h"

/*---------------------------------------------------------------------------*/

class Inet
{
public:
    static uint32_t NUM_USERS;

    ~Inet()
    {
        if (NUM_USERS != 0)
        {
            log_printf("Error! inet is not properly closed (%d)\n", NUM_USERS);
        }
    }
};

static Inet i_INET;
uint32_t Inet::NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

void inet_start(void)
{
    Inet::NUM_USERS += 1;
    core_start();
    oshttp_init();
}

/*---------------------------------------------------------------------------*/

void inet_finish(void)
{
    if (Inet::NUM_USERS == 1)
    {
        Inet::NUM_USERS = 0;
        oshttp_finish();
        core_finish();
    }
    else
    {
        Inet::NUM_USERS -= 1;
    }
}

/*---------------------------------------------------------------------------*/

static uint32_t i_hex_char(const char_t c)
{
    switch (c)
    {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

uint32_t _inet_hex_value(const char_t *hex)
{
    uint32_t value = 0, i = 0;
    value = i_hex_char(hex[i++]);
    while (hex[i] != '\0' && i < 8)
    {
        value <<= 4;
        value |= i_hex_char(hex[i]);
        i++;
    }

    return value;
}
