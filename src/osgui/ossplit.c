/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.c
 *
 */

/* Operating System split view */

#include "ossplit.h"
#include "ossplit.inl"
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

void _ossplit_detach_and_destroy(OSSplit **view, OSPanel *panel)
{
    cassert_no_null(view);
    ossplit_detach(*view, panel);
    ossplit_destroy(view);
}

/*---------------------------------------------------------------------------*/
