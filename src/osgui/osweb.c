/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osweb.c
 *
 */

/* Operating System native web view */

#include "osweb.h"
#include "osweb.inl"
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

void osweb_detach_and_destroy(OSWeb **view, OSPanel *panel)
{
    cassert_no_null(view);
    osweb_detach(*view, panel);
    osweb_destroy(view);
}

/*---------------------------------------------------------------------------*/
