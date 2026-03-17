/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osline.c
 *
 */

/* Operating System native line */

#include "osline.h"
#include "osline.inl"
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

void _osline_detach_and_destroy(OSLine **line, OSPanel *panel)
{
    cassert_no_null(line);
    osline_detach(*line, panel);
    osline_destroy(line);
}
