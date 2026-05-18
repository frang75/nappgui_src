/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabs.c
 *
 */

/* Operating System native tabctrl */

#include "ostabs.h"
#include "ostabs.inl"
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

void _ostabs_detach_and_destroy(OSTabs **tabs, OSPanel *panel)
{
    cassert_no_null(tabs);
    ostabs_detach(*tabs, panel);
    ostabs_destroy(tabs);
}

/*---------------------------------------------------------------------------*/
