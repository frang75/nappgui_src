/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: sewer.cpp
 *
 */

/* Sewer library */

#include "sewer.h"
#include "cassert.h"
#include "bmath.inl"

/*---------------------------------------------------------------------------*/

class Sewer
{
public:
    static uint32_t NUM_USERS;

    ~Sewer()
    {
        if (NUM_USERS != 0)
        {
            //_osbs_finish();
            //log_printf("Error! osbs is not properly closed (%d)\n", NUM_USERS);
        }
    }
};

uint32_t Sewer::NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

void sewer_start(void)
{
    Sewer::NUM_USERS += 1;
}

/*---------------------------------------------------------------------------*/

void sewer_finish(void)
{
    cassert(Sewer::NUM_USERS > 0);
    if (Sewer::NUM_USERS == 1)
    {
        bmath_finish();
        Sewer::NUM_USERS = 0;
    }
    else
    {
        Sewer::NUM_USERS -= 1;
    }
}

