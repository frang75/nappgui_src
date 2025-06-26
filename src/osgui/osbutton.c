/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.c
 *
 */

/* Operating System native button */

#include "osbutton.h"
#include "osbutton.inl"
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

bool_t _osbutton_text_allowed(const uint32_t flags)
{
    switch (button_get_type(flags))
    {
    case ekBUTTON_PUSH:
    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_RADIO:
        return TRUE;
    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        return FALSE;
        cassert_default();
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t _osbutton_image_allowed(const uint32_t flags)
{
    switch (button_get_type(flags))
    {
    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_RADIO:
        return FALSE;
    case ekBUTTON_PUSH:
    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        return TRUE;
        cassert_default();
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*/

void _osbutton_detach_and_destroy(OSButton **button, OSPanel *panel)
{
    cassert_no_null(button);
    osbutton_detach(*button, panel);
    osbutton_destroy(button);
}

/*---------------------------------------------------------------------------*/
