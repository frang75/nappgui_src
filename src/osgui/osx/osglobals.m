/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osglobals.m
 *
 */

/* Operating System globals */

#include "osgui_osx.inl"
#include "osglobals.h"
#include "osglobals.inl"
#include "oscolor.inl"
#include "oscontrol.inl"
#include "cassert.h"
#include "color.h"
#include "heap.h"
#include "image.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

static NSImage *i_UNCHECKBOX_NORMAL_IMAGE = nil;
static NSImage *i_UNCHECKBOX_PRESSED_IMAGE = nil;
static NSImage *i_UNCHECKBOX_DISABLE_IMAGE = nil;
static NSImage *i_CHECKBOX_NORMAL_IMAGE = nil;
static NSImage *i_CHECKBOX_PRESSED_IMAGE = nil;
static NSImage *i_CHECKBOX_DISABLE_IMAGE = nil;
static NSRect i_CHECKBOX_RECT;
static CGFloat i_TEXT_COLOR[4];
static CGFloat i_SELTX_COLOR[4];
static CGFloat i_HOTTX_COLOR[4];
static CGFloat i_TEXTBACKDROP_COLOR[4];
static CGFloat i_SELTXBACKDROP_COLOR[4];
static CGFloat i_HOTTXBACKDROP_COLOR[4];
static CGFloat i_BACK_COLOR[4];
static CGFloat i_SELBG_COLOR[4];
static CGFloat i_HOTBG_COLOR[4];
static CGFloat i_BACKBACKDROP_COLOR[4];
static CGFloat i_SELBGBACKDROP_COLOR[4];
static CGFloat i_HOTBGBACKDROP_COLOR[4];
static bool_t i_DARK_MODE = FALSE;

/*---------------------------------------------------------------------------*/

#define i_SET_COLOR(v, r, g, b, a) ((v)[0]=(CGFloat)(r),(v)[1]=(CGFloat)(g),(v)[2]=(CGFloat)(b),(v)[3]=(CGFloat)(a))

/*---------------------------------------------------------------------------*/

static void i_theme_colors(void)
{
    CGFloat r, g, b, a;
    oscolor_NSColor_rgba([NSColor windowBackgroundColor], &r, &g, &b, &a);
    i_DARK_MODE = (.21 * r + .72 * g + .07 * b) < .5 ? TRUE : FALSE;

    if (i_DARK_MODE == TRUE)
    {
        i_SET_COLOR(i_TEXT_COLOR, .86, .86, .86, 1);
        i_SET_COLOR(i_SELTX_COLOR, .86, .86, .86, 1);
        i_SET_COLOR(i_HOTTX_COLOR, .86, .86, .86, 1);
        i_SET_COLOR(i_TEXTBACKDROP_COLOR, .86, .86, .86, 1);
        i_SET_COLOR(i_SELTXBACKDROP_COLOR, .86, .86, .86, 1);
        i_SET_COLOR(i_HOTTXBACKDROP_COLOR, .86, .86, .86, 1);
        i_SET_COLOR(i_BACK_COLOR, r, g, b, 1);
        i_SET_COLOR(i_SELBG_COLOR, .14, .35, .79, 1);
        i_SET_COLOR(i_HOTBG_COLOR, r, g, b, 1);
        i_SET_COLOR(i_BACKBACKDROP_COLOR, r, g, b, 1);
        i_SET_COLOR(i_SELBGBACKDROP_COLOR, .27, .27, .27, 1);
        i_SET_COLOR(i_HOTBGBACKDROP_COLOR, r, g, b, 1);
    }
    else
    {
        i_SET_COLOR(i_TEXT_COLOR, 0, 0, 0, 1);
        i_SET_COLOR(i_SELTX_COLOR, 1, 1, 1, 1);
        i_SET_COLOR(i_HOTTX_COLOR, 0, 0, 0, 1);
        i_SET_COLOR(i_TEXTBACKDROP_COLOR, 0, 0, 0, 1);
        i_SET_COLOR(i_SELTXBACKDROP_COLOR, 0, 0, 0, 1);
        i_SET_COLOR(i_HOTTXBACKDROP_COLOR, 0, 0, 0, 1);
        i_SET_COLOR(i_BACK_COLOR, r, g, b, 1);
        i_SET_COLOR(i_SELBG_COLOR, .2, .447, .855, 1);
        i_SET_COLOR(i_HOTBG_COLOR, .95, .96, .98, 1);
        i_SET_COLOR(i_BACKBACKDROP_COLOR, r, g, b, 1);
        i_SET_COLOR(i_SELBGBACKDROP_COLOR, .827, .827, .827, 1);
        i_SET_COLOR(i_HOTBGBACKDROP_COLOR, .95, .96, .98, 1);
    }

    unref(a);
}

/*---------------------------------------------------------------------------*/

#if defined (MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
#define REGULAR_SQUARE_BEZEL NSBezelStyleRegularSquare
#define SWITCH_BUTTON NSButtonTypeSwitch
#define RADIO_BUTTON NSButtonTypeRadio
#else
#define REGULAR_SQUARE_BEZEL NSRegularSquareBezelStyle
#define PUSH_IN_BUTTON NSMomentaryPushInButton
#define SWITCH_BUTTON NSSwitchButton
#define RADIO_BUTTON NSRadioButton
#endif

#if defined (MAC_OS_X_VERSION_10_13) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
#define BUTTON_ON NSControlStateValueOn
#define BUTTON_OFF NSControlStateValueOff
#else
#define BUTTON_ON NSOnState
#define BUTTON_OFF NSOffState
#endif

/*---------------------------------------------------------------------------*/

static NSImage *i_image_from_view(NSView *view, NSRect *pixrect)
{
    NSRect rect = [view frame];
    NSBitmapImageRep *irep = [view bitmapImageRepForCachingDisplayInRect:rect];
    NSImage *image = [[NSImage alloc] initWithSize:rect.size];
    [irep setSize:rect.size];
    [view cacheDisplayInRect:rect toBitmapImageRep:irep];
    [image addRepresentation:irep];
    [image setSize:rect.size];

    // Locate the exact frame of pixels block
    if (pixrect != NULL)
    {
        NSInteger w = [irep pixelsWide];
        NSInteger h = [irep pixelsHigh];
        NSInteger line = [irep bytesPerRow];
        NSInteger x0 = w, x1 = 0, yy0 = h, yy1 = 0;
        unsigned char *pixplanes[5] = {NULL, NULL, NULL, NULL, NULL};
        uint32_t *buffer = NULL;
        CGFloat scale = w / rect.size.width;
        cassert([irep bitsPerPixel] == 32);
        [irep getBitmapDataPlanes:pixplanes];
        buffer = (uint32_t*)pixplanes[0];

        for (NSInteger j = 0; j < h; ++j)
        {
            for (NSInteger i = 0; i < w; ++i)
            {
                if (buffer[i] != 0)
                {
                    if (i < x0) x0 = i;
                    if (i > x1) x1 = i;
                    if (j < yy0) yy0 = j;
                    if (j > yy1) yy1 = j;
                }
            }

            buffer += line / 4;
        }

        pixrect->origin.x = (CGFloat)ceil((CGFloat)x0 / scale);
        pixrect->origin.y = (CGFloat)ceil((CGFloat)yy0 / scale);
        pixrect->size.width = (CGFloat)ceil((CGFloat)(x1 - x0 + 1) / scale);
        pixrect->size.height = (CGFloat)ceil((CGFloat)(yy1 - yy0 + 1) / scale);
    }

    return image;
}

/*---------------------------------------------------------------------------*/

static void i_init_checkbox(void)
{
    NSInteger width = 30, height = 30;
    NSButton *button = [[NSButton alloc] initWithFrame:NSMakeRect(0.0, 0.0, width, height)];
    cassert(i_UNCHECKBOX_NORMAL_IMAGE == nil);
    cassert(i_UNCHECKBOX_PRESSED_IMAGE == nil);
    cassert(i_UNCHECKBOX_DISABLE_IMAGE == nil);
	cassert(i_CHECKBOX_NORMAL_IMAGE == nil);
	cassert(i_CHECKBOX_PRESSED_IMAGE == nil);
	cassert(i_CHECKBOX_DISABLE_IMAGE == nil);

    [button setTitle:@""];
    [button setBezelStyle:REGULAR_SQUARE_BEZEL];
    [button setBordered:NO];
    [button setShowsBorderOnlyWhileMouseInside:NO];
    [[button cell] setImageScaling:NSImageScaleNone];
    [button setButtonType:SWITCH_BUTTON];
    [button setState:BUTTON_OFF];
    [button highlight:NO];
    [button setEnabled:YES];
    i_UNCHECKBOX_NORMAL_IMAGE = i_image_from_view(button, &i_CHECKBOX_RECT);

    [button highlight:YES];
    i_UNCHECKBOX_PRESSED_IMAGE = i_image_from_view(button, NULL);

    [button highlight:NO];
    [button setEnabled:NO];
    i_UNCHECKBOX_DISABLE_IMAGE = i_image_from_view(button, NULL);

    [button setState:BUTTON_ON];
    [button highlight:NO];
    [button setEnabled:YES];
    i_CHECKBOX_NORMAL_IMAGE = i_image_from_view(button, NULL);

    [button highlight:YES];
    i_CHECKBOX_PRESSED_IMAGE = i_image_from_view(button, NULL);

    [button highlight:NO];
    [button setEnabled:NO];
    i_CHECKBOX_DISABLE_IMAGE = i_image_from_view(button, NULL);

    [button release];
}

/*---------------------------------------------------------------------------*/

device_t osglobals_device(const void *non_used)
{
    cassert(non_used == NULL);
    unref(non_used);
    return ekDESKTOP;
}

/*---------------------------------------------------------------------------*/

color_t osglobals_color(const syscolor_t *color)
{
    cassert_no_null(color);
    switch (*color) {
	case ekSYSCOLOR_DARKMODE:
		return (color_t)i_DARK_MODE;

    case ekSYSCOLOR_LABEL:
	#if defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
    	return oscolor_from_NSColor([NSColor labelColor]);
	#else
	    return oscolor_from_NSColor([NSColor controlTextColor]);
	#endif

    case ekSYSCOLOR_VIEW:
		return oscolor_from_NSColor([NSColor windowBackgroundColor]);

    case ekSYSCOLOR_LINE:
		return oscolor_from_NSColor([NSColor gridColor]);

    case ekSYSCOLOR_LINK:
	#if defined (MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
    	return oscolor_from_NSColor([NSColor linkColor]);
	#else
    	return oscolor_from_NSColor([NSColor colorWithCalibratedRed:0 green:(CGFloat).410 blue:(CGFloat).853 alpha:1]);
	#endif

    case ekSYSCOLOR_BORDER:
		return oscolor_from_NSColor([NSColor gridColor]);

	cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void osglobals_resolution(const void *non_used, real32_t *width, real32_t *height)
{
    NSRect frame;
    cassert(non_used == NULL);
    unref(non_used);
    cassert_no_null(width);
    cassert_no_null(height);
    frame = [[NSScreen mainScreen] frame];
    *width = (real32_t)frame.size.width;
    *height = (real32_t)frame.size.height;
}

/*---------------------------------------------------------------------------*/

void osglobals_mouse_position(const void *non_used, real32_t *x, real32_t *y)
{
    NSRect mouse_frame;
    NSPoint origin;
    cassert(non_used == NULL);
    unref(non_used);
    cassert_no_null(x);
    cassert_no_null(y);
    mouse_frame.origin = [NSEvent mouseLocation];
    mouse_frame.size = NSZeroSize;
    _oscontrol_origin_in_screen_coordinates(&mouse_frame, &origin.x, &origin.y);
    *x = (real32_t)origin.x;
    *y = (real32_t)origin.y;
}

/*---------------------------------------------------------------------------*/

Cursor *osglobals_cursor(const gui_cursor_t cursor, const Image *image, const real32_t hot_x, const real32_t hot_y)
{
    NSCursor *nscursor = nil;
    switch (cursor) {
    case ekGUI_CURSOR_ARROW:
        nscursor = [[NSCursor arrowCursor] retain];
        break;
    case ekGUI_CURSOR_HAND:
        nscursor = [[NSCursor openHandCursor] retain];
        break;
    case ekGUI_CURSOR_IBEAM:
        nscursor = [[NSCursor IBeamCursor] retain];
        break;
    case ekGUI_CURSOR_CROSS:
        nscursor = [[NSCursor crosshairCursor] retain];
        break;
    case ekGUI_CURSOR_SIZEWE:
        nscursor = [[NSCursor resizeLeftRightCursor] retain];
        break;
    case ekGUI_CURSOR_SIZENS:
        nscursor = [[NSCursor resizeDownCursor] retain];
        break;
    case ekGUI_CURSOR_USER:
    {
        NSImage *nsimage = (NSImage*)image_native(image);
        nscursor = [[NSCursor alloc] initWithImage:nsimage hotSpot:NSMakePoint((CGFloat)hot_x, (CGFloat)hot_y)];
        break;
    }

    cassert_default();
    }

    heap_auditor_add("NSCursor");
    return (Cursor*)nscursor;
}

/*---------------------------------------------------------------------------*/

void osglobals_cursor_destroy(Cursor **cursor)
{
    cassert_no_null(cursor);
    cassert_no_null(*cursor);
    [(NSCursor*)*cursor release];
    heap_auditor_delete("NSCursor");
    *cursor = NULL;
}

/*---------------------------------------------------------------------------*/

void osglobals_value(const uint32_t index, void *value)
{
    cassert_no_null(value);
    switch (index) {
    case 0:
        (*(uint32_t*)value) = 0;
        break;

    case 1:
        (*(uint32_t*)value) = 0;
        break;

    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void osglobals_init(void)
{
	i_theme_colors();
}

/*---------------------------------------------------------------------------*/

static void i_destroy_checkbox(void)
{
    if (i_UNCHECKBOX_NORMAL_IMAGE != nil)
    {
        [i_UNCHECKBOX_NORMAL_IMAGE release];
        [i_UNCHECKBOX_PRESSED_IMAGE release];
        [i_UNCHECKBOX_DISABLE_IMAGE release];
        [i_CHECKBOX_NORMAL_IMAGE release];
        [i_CHECKBOX_PRESSED_IMAGE release];
        [i_CHECKBOX_DISABLE_IMAGE release];
        i_UNCHECKBOX_NORMAL_IMAGE = nil;
        i_UNCHECKBOX_PRESSED_IMAGE = nil;
        i_UNCHECKBOX_DISABLE_IMAGE = nil;
        i_CHECKBOX_NORMAL_IMAGE = nil;
        i_CHECKBOX_PRESSED_IMAGE = nil;
        i_CHECKBOX_DISABLE_IMAGE = nil;
    }
}

/*---------------------------------------------------------------------------*/

void osglobals_finish(void)
{
    i_destroy_checkbox();
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_check_width(void)
{
	if (i_UNCHECKBOX_NORMAL_IMAGE == nil)
        i_init_checkbox();
    return (uint32_t)i_CHECKBOX_RECT.size.width;
}

/*---------------------------------------------------------------------------*/

uint32_t osglobals_check_height(void)
{
	if (i_UNCHECKBOX_NORMAL_IMAGE == nil)
        i_init_checkbox();
    return (uint32_t)i_CHECKBOX_RECT.size.height;
}

/*---------------------------------------------------------------------------*/

NSSize osglobals_check_size(void)
{
	if (i_UNCHECKBOX_NORMAL_IMAGE == nil)
        i_init_checkbox();
    return i_CHECKBOX_RECT.size;
}

/*---------------------------------------------------------------------------*/

NSRect osglobals_check_rect(void)
{
	if (i_UNCHECKBOX_NORMAL_IMAGE == nil)
        i_init_checkbox();
	return i_CHECKBOX_RECT;
}

/*---------------------------------------------------------------------------*/

NSImage *osglobals_checkbox_image(const bool_t pressed, const ctrl_state_t state)
{
	if (i_UNCHECKBOX_NORMAL_IMAGE == nil)
        i_init_checkbox();

    if (pressed == TRUE)
    {
        switch (state) {
        case ekCTRL_STATE_NORMAL:
        case ekCTRL_STATE_BKNORMAL:
        case ekCTRL_STATE_HOT:
        case ekCTRL_STATE_BKHOT:
            return i_CHECKBOX_NORMAL_IMAGE;

        case ekCTRL_STATE_PRESSED:
        case ekCTRL_STATE_BKPRESSED:
            return i_CHECKBOX_PRESSED_IMAGE;

        case ekCTRL_STATE_DISABLED:
            return i_CHECKBOX_DISABLE_IMAGE;

		cassert_default();
        }
    }
    else
    {
        switch (state) {
        case ekCTRL_STATE_NORMAL:
        case ekCTRL_STATE_BKNORMAL:
        case ekCTRL_STATE_HOT:
        case ekCTRL_STATE_BKHOT:
            return i_UNCHECKBOX_NORMAL_IMAGE;

        case ekCTRL_STATE_PRESSED:
        case ekCTRL_STATE_BKPRESSED:
            return i_UNCHECKBOX_PRESSED_IMAGE;

        case ekCTRL_STATE_DISABLED:
            return i_UNCHECKBOX_DISABLE_IMAGE;

		cassert_default();
        }
    }

    return i_CHECKBOX_NORMAL_IMAGE;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_text_color(void)
{
	return i_TEXT_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_seltx_color(void)
{
	return i_SELTX_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_hottx_color(void)
{
	return i_HOTTX_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_textbackdrop_color(void)
{
	return i_TEXTBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_seltxbackdrop_color(void)
{
    return i_SELTXBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_hottxbackdrop_color(void)
{
	return i_HOTTXBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_back_color(void)
{
    return i_BACK_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_hotbg_color(void)
{
    return i_HOTBG_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_selbg_color(void)
{
    return i_SELBG_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_backbackdrop_color(void)
{
    return i_BACKBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_hotbgbackdrop_color(void)
{
    return i_HOTBGBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

const CGFloat *osglobals_selbgbackdrop_color(void)
{
    return i_SELBGBACKDROP_COLOR;
}

/*---------------------------------------------------------------------------*/

void osglobals_theme_changed(void)
{
	i_theme_colors();
    i_destroy_checkbox();
}
