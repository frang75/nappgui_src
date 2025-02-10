/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: progress.c
 *
 */

/* Progress indicator */

#include "progress.h"
#include "progress.inl"
#include "component.inl"
#include <draw2d/guictx.h>
#include <core/objh.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

struct _progress_t
{
    GuiComponent component;
    real32_t width;
    real32_t height;
};

/*---------------------------------------------------------------------------*/

static Progress *i_create(const gui_size_t psize)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = NULL;
    Progress *progress;
    cassert_no_null(context);
    progress = obj_new(Progress);
    ositem = context->func_create[ekGUI_TYPE_PROGRESS](ekPROGRESS_HORZ);
    _component_init(&progress->component, context, PARAM(type, ekGUI_TYPE_PROGRESS), &ositem);
    progress->width = 100;
    progress->height = context->func_progress_get_thickness(progress->component.ositem, (enum_t)psize);
    return progress;
}

/*---------------------------------------------------------------------------*/

void _progress_destroy(Progress **progress)
{
    cassert_no_null(progress);
    cassert_no_null(*progress);
    _component_destroy_imp(&(*progress)->component);
    obj_delete(progress, Progress);
}

/*---------------------------------------------------------------------------*/

Progress *progress_create(void)
{
    return i_create(ekGUI_SIZE_REGULAR);
}

/*---------------------------------------------------------------------------*/

void progress_min_width(Progress *progress, const real32_t width)
{
    cassert_no_null(progress);
    cassert(width > 0);
    progress->width = width;
}

/*---------------------------------------------------------------------------*/

void progress_undefined(Progress *progress, const bool_t running)
{
    real32_t position = -2.f;
    cassert_no_null(progress);
    cassert_no_null(progress->component.context);
    cassert_no_nullf(progress->component.context->func_progress_set_position);
    if (running == FALSE)
        position = -.5f;
    progress->component.context->func_progress_set_position(progress->component.ositem, position);
}

/*---------------------------------------------------------------------------*/

void progress_value(Progress *progress, const real32_t value)
{
    cassert_no_null(progress);
    cassert_no_null(progress->component.context);
    cassert_no_nullf(progress->component.context->func_progress_set_position);
    progress->component.context->func_progress_set_position(progress->component.ositem, value);
}

/*---------------------------------------------------------------------------*/

void _progress_dimension(Progress *progress, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(progress);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        *dim0 = progress->width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = progress->height;
    }
}