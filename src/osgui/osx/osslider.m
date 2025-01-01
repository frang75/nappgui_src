/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osslider.m
 *
 */

/* Operating System native slider */

#include "osslider_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "oswindow_osx.inl"
#include "../osslider.h"
#include "../osslider.inl"
#include "../osgui.inl"
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXSlider : NSSlider
{
  @public
    uint32_t flags;
    Listener *OnMoved;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXSlider

/*---------------------------------------------------------------------------*/

/*
 -(void)drawRect:(NSRect)rect
 {
     NSFrameRect(rect);
     [super drawRect:rect];
 }*/

/*---------------------------------------------------------------------------*/

- (IBAction)onSliderMoved:(id)sender
{
    cassert(sender == self);
    if ([self isEnabled] == YES && self->OnMoved != NULL)
    {
        EvSlider params;
        params.pos = (real32_t)[sender floatValue];
        params.incr = 0;
        params.step = UINT32_MAX;
        listener_event(self->OnMoved, ekGUI_EVENT_SLIDER, cast(sender, OSSlider), &params, NULL, OSSlider, EvSlider, void);
    }
}

/*---------------------------------------------------------------------------*/

- (BOOL)acceptsFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (BOOL)becomeFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    if (_oswindow_mouse_down(cast(self, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

@end

/*---------------------------------------------------------------------------*/

/*
static CGFloat i_slider_with_tickmarks_height(const gui_size_t knob_size)
{
    switch(knob_size)
    {
        case ekGUI_MINI:
            return 16.f;
        case ekREGULAR:
            return 18.f;
        case ekGUI_SIZE_LARGE:
            return 26.f;
        cassert_default();
    }

    return 0.f;
}*/

/*---------------------------------------------------------------------------*/

OSSlider *osslider_create(const uint32_t flags)
{
    OSXSlider *slider = nil;
    heap_auditor_add("OSXSlider");
    slider = [[OSXSlider alloc] initWithFrame:NSZeroRect];
    slider->flags = flags;
    slider->OnMoved = NULL;
    _oscontrol_init(slider);
    [slider setMinValue:0.];
    [slider setMaxValue:1.];
    [slider setFloatValue:slider_get_type(flags) == ekSLIDER_HORZ ? 0.f : 1.f];

#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    [slider setVertical:(BOOL)(slider_get_type(flags) == ekSLIDER_VERT)];
#endif

    [slider setAltIncrementValue:0.01];
    [slider setTarget:slider];
    [slider setAction:@selector(onSliderMoved:)];
    [slider setNumberOfTickMarks:0];
    return cast(slider, OSSlider);
}

/*---------------------------------------------------------------------------*/

void osslider_destroy(OSSlider **slider)
{
    OSXSlider *lslider = nil;
    cassert_no_null(slider);
    lslider = *dcast(slider, OSXSlider);
    cassert_no_null(lslider);
    listener_destroy(&lslider->OnMoved);
    [lslider release];
    heap_auditor_delete("OSXSlider");
    *slider = NULL;
}

/*---------------------------------------------------------------------------*/

void osslider_OnMoved(OSSlider *slider, Listener *listener)
{
    cassert_no_null(slider);
    cassert_no_null(listener);
    listener_update(&cast(slider, OSXSlider)->OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void osslider_tooltip(OSSlider *slider, const char_t *text)
{
    _oscontrol_tooltip_set(cast(slider, OSXSlider), text);
}

/*---------------------------------------------------------------------------*/

void osslider_tickmarks(OSSlider *slider, const uint32_t num_tickmarks, const bool_t tickmarks_at_left_top)
{
    cassert_no_null(slider);
    cassert(num_tickmarks > 0);
    unref(slider);
    unref(num_tickmarks);
    unref(tickmarks_at_left_top);
    /*
    switch (((OSXSlider*)slider)->hv)
    {
        case ekGUI_VERTICAL:

            size.width = i_slider_with_tickmarks_height(((OSXSlider*)slider)->knob_size);
            size.height = (CGFloat)((OSXSlider*)slider)->length;
            break;

        case ekGUI_HORIZONTAL:

            size.width = (CGFloat)((OSXSlider*)slider)->length;
            size.height = i_slider_with_tickmarks_height(((OSXSlider*)slider)->knob_size);
            break;

        cassert_default();
    }

    [(OSXSlider*)slider setFrameSize:size];
*/
    /*
   [(OSXSlider*)slider setNumberOfTickMarks:(NSInteger)num_tickmarks];
   [(OSXSlider*)slider setAllowsTickMarkValuesOnly:NO];

#if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    [(OSXSlider*)slider setTickMarkPosition:(tickmarks_at_left_top == TRUE) ? NSTickMarkPositionAbove : NSTickMarkPositionBelow];
#else
    [(OSXSlider*)slider setTickMarkPosition:(tickmarks_at_left_top == TRUE) ? NSTickMarkAbove : NSTickMarkBelow];
#endif
*/
}

/*---------------------------------------------------------------------------*/

void osslider_position(OSSlider *slider, const real32_t position)
{
    OSXSlider *lslider = cast(slider, OSXSlider);
    real32_t pos = position;
    cassert_no_null(lslider);
    cassert(position >= 0.f && position <= 1.f);
    if (slider_get_type(lslider->flags) == ekSLIDER_VERT)
        pos = 1.f - position;
    [lslider setFloatValue:pos];
}

/*---------------------------------------------------------------------------*/

real32_t osslider_get_position(const OSSlider *slider)
{
    cassert_no_null(slider);
    return (real32_t)[cast_const(slider, OSXSlider) floatValue];
}

/*---------------------------------------------------------------------------*/

static real32_t i_slider_without_tickmarks_height(const gui_size_t knob_size)
{
    switch (knob_size)
    {
    case ekGUI_SIZE_MINI:
        return 12.f;
    case ekGUI_SIZE_SMALL:
        return 15.f;
    case ekGUI_SIZE_REGULAR:
        return 25.f;
        cassert_default();
    }

    return 0.f;
}

/*---------------------------------------------------------------------------*/

void osslider_bounds(const OSSlider *slider, const real32_t length, const gui_size_t knob_size, real32_t *width, real32_t *height)
{
    const OSXSlider *lslider = cast_const(slider, OSXSlider);
    cassert_no_null(lslider);
    cassert_no_null(width);
    cassert_no_null(height);
    switch (slider_get_type(lslider->flags))
    {
    case ekSLIDER_HORZ:
        *width = length;
        *height = i_slider_without_tickmarks_height(knob_size);
        break;
    case ekSLIDER_VERT:
        *width = i_slider_without_tickmarks_height(knob_size);
        *height = length;
        break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void osslider_attach(OSSlider *slider, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(slider, NSView));
}

/*---------------------------------------------------------------------------*/

void osslider_detach(OSSlider *slider, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(slider, NSView));
}

/*---------------------------------------------------------------------------*/

void osslider_visible(OSSlider *slider, const bool_t visible)
{
    _oscontrol_set_visible(cast(slider, NSView), visible);
}

/*---------------------------------------------------------------------------*/

void osslider_enabled(OSSlider *slider, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(slider, NSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osslider_size(const OSSlider *slider, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast(slider, NSView), width, height);
}

/*---------------------------------------------------------------------------*/

void osslider_origin(const OSSlider *slider, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast(slider, NSView), x, y);
}

/*---------------------------------------------------------------------------*/

void osslider_frame(OSSlider *slider, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(slider, NSView), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

BOOL _osslider_is(NSView *view)
{
    return [view isKindOfClass:[OSXSlider class]];
}
