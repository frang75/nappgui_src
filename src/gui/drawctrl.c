/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawctrl.c
 *
 */

/* Drawing custom GUI controls */

#include "guicontexth.inl"
#include "drawctrl.inl"

static uint32_t kCHECK_WIDTH = 0;
static uint32_t kCHECK_HEIGHT = 0;

/*---------------------------------------------------------------------------*/

Font *drawctrl_font(DCtx *ctx)
{
    const GuiContext *context = gui_context_get_current();
    return (Font*)context->func_drawctrl_font(ctx);
}

/*---------------------------------------------------------------------------*/

uint32_t drawctrl_row_padding(DCtx *ctx)
{
    const GuiContext *context = gui_context_get_current();
    return context->func_drawctrl_row_padding(ctx);
}

/*---------------------------------------------------------------------------*/

uint32_t drawctrl_check_width(DCtx *ctx)
{
    if (kCHECK_WIDTH == 0)
    {
        const GuiContext *context = gui_context_get_current();
        kCHECK_WIDTH = context->func_drawctrl_check_width(ctx);
    }

    return kCHECK_WIDTH;
}

/*---------------------------------------------------------------------------*/

uint32_t drawctrl_check_height(DCtx *ctx)
{
    if (kCHECK_HEIGHT == 0)
    {
        const GuiContext *context = gui_context_get_current();
        kCHECK_HEIGHT = context->func_drawctrl_check_height(ctx);
    }

    return kCHECK_HEIGHT;
}

/*---------------------------------------------------------------------------*/

multisel_t drawctrl_multisel(DCtx *ctx, const vkey_t key)
{
    const GuiContext *context = gui_context_get_current();
    return (multisel_t)context->func_drawctrl_multisel(ctx, (enum_t)key);
}

/*---------------------------------------------------------------------------*/

void drawctrl_clear(DCtx *ctx)
{
    const GuiContext *context = gui_context_get_current();
    context->func_drawctrl_clear(ctx);
}

/*---------------------------------------------------------------------------*/

void drawctrl_fill(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    const GuiContext *context = gui_context_get_current();
    context->func_drawctrl_fill(ctx, x, y, width, height, (enum_t)state);
}

/*---------------------------------------------------------------------------*/

void drawctrl_focus(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
{
    const GuiContext *context = gui_context_get_current();
    context->func_drawctrl_focus(ctx, x, y, width, height, (enum_t)0);
}

/*---------------------------------------------------------------------------*/

void drawctrl_text(DCtx *ctx, const char_t *text, const uint32_t x, const uint32_t y, const cstate_t state)
{
    const GuiContext *context = gui_context_get_current();
    context->func_drawctrl_text(ctx, text, x, y, (enum_t)state);
}

/*---------------------------------------------------------------------------*/

void drawctrl_image(DCtx *ctx, const Image *image, const uint32_t x, const uint32_t y)
{
    const GuiContext *context = gui_context_get_current();
    context->func_drawctrl_image(ctx, image, x, y, (enum_t)0);
}

/*---------------------------------------------------------------------------*/

void drawctrl_checkbox(DCtx *ctx, const uint32_t x, const uint32_t y, const cstate_t state)
{
    const GuiContext *context = gui_context_get_current();
    context->func_drawctrl_checkbox(ctx, x, y, kCHECK_WIDTH, kCHECK_HEIGHT, (enum_t)state);
}

/*---------------------------------------------------------------------------*/

void drawctrl_uncheckbox(DCtx *ctx, const uint32_t x, const uint32_t y, const cstate_t state)
{
    const GuiContext *context = gui_context_get_current();
    context->func_drawctrl_uncheckbox(ctx, x, y, kCHECK_WIDTH, kCHECK_HEIGHT, (enum_t)state);
}
