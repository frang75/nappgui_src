/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nglob.c
 *
 */

/* NAppGUI project globals */

#include "nglob.h"
#include <osbs/btime.h>
#include <core/strings.h>

/*---------------------------------------------------------------------------*/

uint32_t nglob_begin_year(void)
{
    return 2015;
}

/*---------------------------------------------------------------------------*/

String *nglob_copyright(void)
{
    Date date;
    btime_date(&date);
    return str_printf("(CC) %d-%d Francisco Garcia Collado", nglob_begin_year(), date.year);
}
