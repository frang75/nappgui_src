/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
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
#include "guicontexth.inl"
#include "obj.inl"
#include "cassert.h"
#include "ptr.h"

struct _progress_t
{
    GuiComponent component;
    real32_t thickness;
    real32_t min_value;
    real32_t max_value;
};

/*---------------------------------------------------------------------------*/

static Progress *i_create_progress(const GuiComponent *component, const real32_t thickness, const real32_t min_value, const real32_t max_value)
{
    Progress *progress = obj_new(Progress);
    progress->component = ptr_get(component, GuiComponent);
    progress->thickness = thickness;
    progress->min_value = min_value;
    progress->max_value = max_value;
    return progress;
}

/*---------------------------------------------------------------------------*/

static void i_set_progress_position(GuiComponent *component, const real32_t min_value, const real32_t max_value, const real32_t value)
{
    real32_t interval;
    real32_t position;
    cassert_no_null(component);
    cassert_no_null(component->context);
    cassert_no_nullf(component->context->func_progress_set_position);
    interval = max_value - min_value;
    cassert(interval > 0.f);
    position = (value - min_value) / interval;
    component->context->func_progress_set_position(component->ositem, position);
}

/*---------------------------------------------------------------------------*/

static Progress *i_create_init_progress(const fsize_t psize)
{
    const GuiContext *context = gui_context_get_current();
    real32_t thickness;
    GuiComponent component;
    Progress *progress;
    cassert_no_null(context);
    cassert_no_nullf(context->func_progress_create);
    cassert_no_nullf(context->func_progress_get_thickness);

    {
        void *ositem = context->func_progress_create((const enum_t)ekPGHORZ);
        _component_init(&component, context, PARAM(type, ekGUI_COMPONENT_PROGRESS), &ositem);
    }

    thickness = context->func_progress_get_thickness(component.ositem, (const enum_t)psize);
    progress = i_create_progress(&component, thickness, PARAM(min_value, 0.f), PARAM(max_value, 1.f));
    i_set_progress_position(&component, progress->min_value, progress->max_value, 0.f);
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
    return i_create_init_progress(ekREGULAR);
}

/*---------------------------------------------------------------------------*/

/*
Progress *progress_create_spinning(const fsize_t size)
{
    return i_create_init_progress(ekGUI_PROGRESS_STYLE_SPINNING, size, 0.f);
}
 */

/*---------------------------------------------------------------------------*/

/*
void progress_set_limits(Progress *progress, const real32_t min_value, const real32_t max_value);
void progress_set_limits(Progress *progress, const real32_t min_value, const real32_t max_value)
{
    cassert_no_null(progress);
    progress->min_value = min_value;
    progress->max_value = max_value;
}
*/
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
    i_set_progress_position(&progress->component, progress->min_value, progress->max_value, value);
}

/*---------------------------------------------------------------------------*/

void _progress_dimension(Progress *progress, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(progress);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        *dim0 = 100.f;
    }
    else
    {
        cassert(i == 1);
        *dim1 = progress->thickness;
    }
}
