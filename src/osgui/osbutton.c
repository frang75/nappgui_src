/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.c
 *
 */

/* Operating System native button */

#include "osbutton.h"
#include "osbutton.inl"
#include <draw2d/font.h>
#include <core/strings.h>
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
    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        return TRUE;
    default:
        cassert_default(button_get_type(flags));
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
    default:
        cassert_default(button_get_type(flags));
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

static void i_flat_content_size(const real32_t imgwidth, const real32_t imgheight, const real32_t imgsep, const gui_pos_t imgpos, const real32_t twidth, const real32_t theight, real32_t *cwidth, real32_t *cheight)
{
    cassert_no_null(cwidth);
    cassert_no_null(cheight);

    /* Draw text and image */
    if (imgwidth > 0.f && twidth > 0.f)
    {
        switch (imgpos)
        {
        case ekGUI_POS_LEFT:
        case ekGUI_POS_RIGHT:
            *cwidth = imgwidth + imgsep + twidth;
            *cheight = imgheight > theight ? imgheight : theight;
            break;

        case ekGUI_POS_TOP:
        case ekGUI_POS_BOTTOM:
            *cwidth = imgwidth > twidth ? imgwidth : twidth;
            *cheight = imgheight + imgsep + theight;
            break;

        case ekGUI_POS_NONE:
            *cwidth = imgwidth;
            *cheight = imgheight;
            break;

        default:
            cassert_default(imgpos);
        }
    }
    else if (imgwidth > 0.f)
    {
        *cwidth = imgwidth;
        *cheight = imgheight;
    }
    else
    {
        cassert(twidth > 0.f);
        *cwidth = twidth;
        *cheight = theight;
    }
}

/*---------------------------------------------------------------------------*/

void _osbutton_flat_bounds(const char_t *text, const Font *font, const real32_t imgwidth, const real32_t imgheight, const real32_t imgsep, const gui_pos_t imgpos, const uint32_t hpadding, const uint32_t vpadding, real32_t *width, real32_t *height)
{
    real32_t twidth = 0.f;
    real32_t theight = 0.f;
    real32_t cwidth = 0.f;
    real32_t cheight = 0.f;
    const bool_t draw_text = (bool_t)(str_empty_c(text) == FALSE && imgpos != ekGUI_POS_NONE);

    cassert_no_null(width);
    cassert_no_null(height);

    if (draw_text == TRUE)
        font_extents(font, text, -1.f, &twidth, &theight);

    i_flat_content_size(imgwidth, imgheight, imgsep, imgpos, twidth, theight, &cwidth, &cheight);

    if (hpadding == UINT32_MAX)
        *width = cwidth + (real32_t)(uint32_t)((imgwidth * .5f) + .5f);
    else
        *width = cwidth + (real32_t)hpadding;

    if (vpadding == UINT32_MAX)
        *height = cheight + (real32_t)(uint32_t)((imgheight * .5f) + .5f);
    else
        *height = cheight + (real32_t)vpadding;
}

/*---------------------------------------------------------------------------*/

void _osbutton_flat_position(const real32_t width, const real32_t height, const real32_t imgwidth, const real32_t imgheight, const real32_t imgsep, const gui_pos_t imgpos, const real32_t twidth, const real32_t theight, real32_t *imgx, real32_t *imgy, real32_t *tx, real32_t *ty)
{
    real32_t cwidth = 0.f, cheight = 0.f;
    real32_t ox, oy;
    cassert_no_null(imgx);
    cassert_no_null(imgy);
    cassert_no_null(tx);
    cassert_no_null(ty);

    i_flat_content_size(imgwidth, imgheight, imgsep, imgpos, twidth, theight, &cwidth, &cheight);
    ox = (width - cwidth) / 2.f;
    oy = (height - cheight) / 2.f;

    switch (imgpos)
    {
    case ekGUI_POS_LEFT:
        *imgx = ox;
        *imgy = oy + (cheight - imgheight) / 2.f;
        *tx = ox + imgwidth + imgsep;
        *ty = oy + (cheight - theight) / 2.f;
        break;

    case ekGUI_POS_RIGHT:
        *imgx = ox + twidth + imgsep;
        *imgy = oy + (cheight - imgheight) / 2.f;
        *tx = ox;
        *ty = oy + (cheight - theight) / 2.f;
        break;

    case ekGUI_POS_TOP:
        *imgx = ox + (cwidth - imgwidth) / 2.f;
        *imgy = oy;
        *tx = ox + (cwidth - twidth) / 2.f;
        *ty = oy + imgheight + imgsep;
        break;

    case ekGUI_POS_BOTTOM:
        *imgx = ox + (cwidth - imgwidth) / 2.f;
        *imgy = oy + theight + imgsep;
        *tx = ox + (cwidth - twidth) / 2.f;
        *ty = oy;
        break;

    case ekGUI_POS_NONE:
        *imgx = ox + (cwidth - imgwidth) / 2.f;
        *imgy = oy + (cheight - imgheight) / 2.f;
        *tx = 0.f;
        *ty = 0.f;
        break;

    default:
        cassert_default(imgpos);
    }
}
