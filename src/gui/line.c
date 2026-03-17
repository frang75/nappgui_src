/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: line.c
 *
 */

/* Line */

#include "line.h"
#include "line.inl"
#include "component.inl"
#include "gui.inl"
#include <draw2d/guictx.h>
#include <core/objh.h>
#include <sewer/cassert.h>

struct _line_t
{
    GuiComponent component;
    real32_t length;
    S2Df size;
    uint32_t flags;
};

/*---------------------------------------------------------------------------*/

void _line_destroy(Line **line)
{
    cassert_no_null(line);
    cassert_no_null(*line);
    _component_destroy_imp(&(*line)->component);
    obj_delete(line, Line);
}

/*---------------------------------------------------------------------------*/

static Line *i_create(const uint32_t flags)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = NULL;
    Line *line = NULL;
    cassert_no_null(context);
    line = obj_new0(Line);
    ositem = context->func_create[ekGUI_TYPE_LINE](flags);
    _component_init(&line->component, context, ekGUI_TYPE_LINE, &ositem);
    line->length = 100;
    line->flags = flags;
    return line;
}

/*---------------------------------------------------------------------------*/

Line *line_horizontal(void)
{
    return i_create(ekLINE_HORZ);
}

/*---------------------------------------------------------------------------*/

Line *line_vertical(void)
{
    return i_create(ekLINE_VERT);
}

/*---------------------------------------------------------------------------*/

void line_length(Line *line, const real32_t length)
{
    cassert_no_null(line);
    cassert(length > 0);
    line->length = length;
}

/*---------------------------------------------------------------------------*/

void _line_natural(Line *line, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(line);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        cassert_no_nullf(line->component.context->func_line_bounds);
        line->component.context->func_line_bounds(line->component.ositem, line->length, &line->size.width, &line->size.height);
        *dim0 = line->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = line->size.height;
    }
}

/*---------------------------------------------------------------------------*/

bool_t _line_is_horizontal(const Line *line)
{
    cassert_no_null(line);
    return (bool_t)(line_get_type(line->flags) == ekLINE_HORZ);
}
