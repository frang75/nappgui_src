/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol.m
 *
 */

/* Cocoa control */

#include "osgui_osx.inl"
#include "osgui.inl"
#include "oscontrol.inl"
#include "oscolor.inl"
#include "cassert.h"
#include "color.h"
#include "font.h"
#include "image.h"
#include "ptr.h"
#include "unicode.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

void _oscontrol_init(NSView *control)
{
    [control setHidden:YES];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_origin_in_screen_coordinates(const NSRect *frame, CGFloat *x, CGFloat *y)
{
    NSSize screen_size;
    cassert_no_null(frame);
    cassert_no_null(x);
    cassert_no_null(y);
    screen_size = [[NSScreen mainScreen] frame].size;
    *x = frame->origin.x;
    *y = screen_size.height - frame->origin.y - frame->size.height;
}

/*---------------------------------------------------------------------------*/

static __INLINE NSTextAlignment i_text_alignment(const align_t halign)
{
#if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    switch (halign)
    {
        case ekLEFT:
            return NSTextAlignmentLeft;
        case ekCENTER:
            return NSTextAlignmentCenter;
        case ekJUSTIFY:
            return NSTextAlignmentJustified;
        case ekRIGHT:
            return NSTextAlignmentRight;
        cassert_default();
    }
    return NSTextAlignmentLeft;
    
#else
    switch (halign)
    {
        case ekLEFT:
            return NSLeftTextAlignment;
        case ekCENTER:
            return NSCenterTextAlignment;
        case ekJUSTIFY:
            return NSJustifiedTextAlignment;
        case ekRIGHT:
            return NSRightTextAlignment;
        cassert_default();
    }

    return NSLeftTextAlignment;
#endif
}

/*---------------------------------------------------------------------------*/

NSTextAlignment _oscontrol_text_alignment(const align_t halign)
{
    return i_text_alignment(halign);
}

/*---------------------------------------------------------------------------*/

static __INLINE NSControlSize i_control_size(const gui_size_t size)
{
#if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    switch (size)
    {
        case ekGUI_SIZE_MINI:
            return NSControlSizeMini;
        case ekGUI_SIZE_SMALL:
            return NSControlSizeSmall;
        case ekGUI_SIZE_REGULAR:
            return NSControlSizeRegular;
        cassert_default();
    }
#else
    switch (size)
    {
        case ekGUI_SIZE_MINI:
            return NSMiniControlSize;
        case ekGUI_SIZE_SMALL:
            return NSSmallControlSize;
        case ekGUI_SIZE_REGULAR:
            return NSRegularControlSize;
        cassert_default();
    }
#endif

    return (NSControlSize)UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

NSControlSize _oscontrol_size(const gui_size_t size)
{
    return i_control_size(size);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_size_from_font(NSCell *cell, const Font *font)
{
    gui_size_t size = _osgui_size_font(font_size(font));
    [cell setControlSize:i_control_size(size)];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_cell_set_control_size(NSCell *cell, const gui_size_t size)
{
    cassert_no_null(cell);
    [cell setControlSize:i_control_size(size)];
}

/*---------------------------------------------------------------------------*/

//static __INLINE NSCellImagePosition i_position(const enum gui_position_t image_position)
//{
//    cassert(FALSE);
//    switch(image_position)
//    {
//        case ekGUI_POSITION_LEFT:
//            return NSImageLeft;
//        case ekGUI_POSITION_RIGHT:
//            return NSImageRight;
//        case ekGUI_POSITION_TOP:
//            return NSImageBelow;
//        case ekGUI_POSITION_BOTTOM:
//            return NSImageAbove;
//        /*case ekGUI_POSITION_HEXPAND:
//            return NSImageOverlaps;*/
//        cassert_default();
//    }
//
//    return NSImageLeft;
//}

/*---------------------------------------------------------------------------*/

//void _oscontrol_cell_set_image_position(NSButtonCell *cell, const enum gui_position_t position);
//void _oscontrol_cell_set_image_position(NSButtonCell *cell, const enum gui_position_t position)
//{
//    cassert_no_null(cell);
//    [cell setImagePosition:i_position(position)];
//}

/*---------------------------------------------------------------------------*/

void _oscontrol_cell_set_image(NSCell *cell, const Image *image)
{
    cassert_no_null(cell);
    [cell setImage:(NSImage*)image_native(image)];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_text_bounds(const Font *font, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height)
{
    uint32_t style;
    StringSizeData data;
    NSNumber *undertype = nil;
    NSNumber *strikeout = nil;
    style = font_style(font);
    undertype = (style & ekFUNDERLINE) ? kUNDERLINE_STYLE_SINGLE : kUNDERLINE_STYLE_NONE;
    strikeout = (style & ekFSTRIKEOUT) ? kUNDERLINE_STYLE_SINGLE : kUNDERLINE_STYLE_NONE;
    data.dict = [[NSDictionary alloc] initWithObjectsAndKeys:font_native(font), NSFontAttributeName, undertype, NSUnderlineStyleAttributeName, strikeout, NSStrikethroughStyleAttributeName, nil];
    _osgui_text_bounds(&data, text, refwidth, width, height);
    [data.dict release];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_tooltip_set(NSView *view, const char_t *text)
{
    cassert_no_null(view);
    if (text != NULL)
    {
        NSString *str = [[NSString alloc] initWithUTF8String:(const char*)text];
        [view setToolTip:str];
        [str release];
    }
    else    
    {
        [view setToolTip:nil];
    }
}

/*---------------------------------------------------------------------------*/

NSColor *_oscontrol_color(const color_t color)
{
    return oscolor_NSColor(color);
}

/*---------------------------------------------------------------------------*/

color_t _oscontrol_from_NSColor(NSColor *color)
{
    return oscolor_from_NSColor(color);
}

/*---------------------------------------------------------------------------*/

static NSColor *i_control_color(NSControl *control, const color_t color)
{
    if ([control isEnabled] == YES)
    {
        if (color == kCOLOR_TRANSPARENT)
        {
            if ([control isKindOfClass:[NSTextField class]])
            {
                #if defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
                return [NSColor labelColor];
                #else
                return [NSColor controlTextColor];
                #endif
            }
            else
            {
                return [NSColor controlTextColor];
            }
        }
        else
        {
            return _oscontrol_color(color);
        }
    }
    else
    {
        return [NSColor disabledControlTextColor];
    }
}

/*---------------------------------------------------------------------------*/

static NSDictionary *i_text_attribs(NSControl *control, const align_t align, const color_t color, const uint32_t fstyle, NSFont *font)
{
    id keys[5];
    id objects[5];

    keys[0] = NSUnderlineStyleAttributeName;
    keys[1] = NSStrikethroughStyleAttributeName;
    keys[2] = NSParagraphStyleAttributeName;
    keys[3] = NSForegroundColorAttributeName;
    keys[4] = NSFontAttributeName;
    objects[0] = (fstyle & ekFUNDERLINE) ? kUNDERLINE_STYLE_SINGLE : kUNDERLINE_STYLE_NONE;
    objects[1] = (fstyle & ekFSTRIKEOUT) ? kUNDERLINE_STYLE_SINGLE : kUNDERLINE_STYLE_NONE;
    
    switch (align) {
    case ekLEFT:
    case ekJUSTIFY:
        objects[2] = kLEFT_PARAGRAPH_STYLE;
        break;
    case ekRIGHT:
        objects[2] = kRIGHT_PARAGRAPH_STYLE;
        break;
    case ekCENTER:
        objects[2] = kCENTER_PARAGRAPH_STYLE;
        break;
    cassert_default();
    }
    
    objects[3] = i_control_color(control, color);
    objects[4] = font;
    
    return [NSDictionary dictionaryWithObjects:objects forKeys:keys count:5];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_init_textattr(OSTextAttr *attr)
{
    cassert_no_null(attr);
    attr->font = _osgui_create_default_font();
    attr->color = kCOLOR_TRANSPARENT;
    attr->align = ekLEFT;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_remove_textattr(OSTextAttr *attr)
{
    cassert_no_null(attr);
    ptr_destopt(font_destroy, &attr->font, Font);
}

/*---------------------------------------------------------------------------*/

static void i_update_text(NSControl *control, const OSTextAttr *attrs, NSString *str)
{
    register NSCell *cell = [control cell];
    register uint32_t fstyle = font_style(attrs->font);

    cassert_no_null(control);
    cassert_no_null(attrs);

    if (str == nil)
    {
        if ([cell isKindOfClass:[NSButtonCell class]])
            str = [cell title];
        else
            str = [cell stringValue];
    }

    //if (fstyle & ekFSTRIKEOUT || fstyle & ekFUNDERLINE)
    {
        NSFont *font = (NSFont*)font_native(attrs->font);
        NSDictionary *dict = i_text_attribs(control, attrs->align, attrs->color, fstyle, font);
        NSAttributedString *mstr = [[NSAttributedString alloc] initWithString:str attributes:dict];

        [cell setFont:font];
        
        if ([cell isKindOfClass:[NSButtonCell class]])
            [(NSButtonCell*)cell setAttributedTitle:mstr];
        else
            [cell setAttributedStringValue:mstr];
        
        [mstr release];
    }
    /*else
    {
        NSFont *font = (NSFont*)font_native(attrs->font);
        
        [cell setFont:font];
        
        if ([cell isKindOfClass:[NSTextFieldCell class]])
            [(NSTextFieldCell*)cell setTextColor:i_control_color(attrs->color, (bool_t)[control isEnabled])];
        if ([cell isKindOfClass:[NSButtonCell class]])
            [(NSButtonCell*)cell setTitle:str];
        else
            [cell setStringValue:str];
    }*/
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_text(NSControl *control, const OSTextAttr *attrs, const char_t *text)
{
    NSString *str = [[NSString alloc] initWithUTF8String:(const char*)text];
    i_update_text(control, attrs, str);
    [str release];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_font(NSControl *control, OSTextAttr *attrs, const Font *font)
{
    cassert_no_null(attrs);
    if (font_equals(attrs->font, font) == FALSE)
    {
        font_destroy(&attrs->font);
        attrs->font = font_copy(font);
        i_update_text(control, attrs, nil);
    }
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_align(NSControl *control, OSTextAttr *attrs, const align_t align)
{
    cassert_no_null(attrs);
    if (attrs->align != align)
    {
        attrs->align = align;
        i_update_text(control, attrs, nil);
    }
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_text_color(NSTextField *control, OSTextAttr *attrs, const color_t color)
{
    cassert_no_null(attrs);
    attrs->color = color;
    i_update_text(control, attrs, nil);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_textfield_deselect(NSTextField *control)
{
    NSText *field_editor = [[control window] fieldEditor:YES forObject:control];
    cassert_no_null(field_editor);
    [field_editor setSelectedRange:NSMakeRange([[field_editor string] length],0)];
    [field_editor setNeedsDisplay:YES];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_progress_set_control_size(NSProgressIndicator *progress, const gui_size_t size)
{
    cassert_no_null(progress);
    [progress setControlSize:i_control_size(size)];
}

/*---------------------------------------------------------------------------*/

static __INLINE NSLevelIndicatorStyle i_level_style(const enum gui_indicator_style_t style)
{
#if defined (MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
    switch (style)
    {
        case ekGUI_INDICATOR_STYLE_CONTINUOUS:
            return NSLevelIndicatorStyleContinuousCapacity;
        case ekGUI_INDICATOR_STYLE_DISCRETE:
            return NSLevelIndicatorStyleDiscreteCapacity;
            cassert_default();
    }    
#else
    switch (style)
    {
        case ekGUI_INDICATOR_STYLE_CONTINUOUS:
            return NSContinuousCapacityLevelIndicatorStyle;
        case ekGUI_INDICATOR_STYLE_DISCRETE:
            return NSDiscreteCapacityLevelIndicatorStyle;
        cassert_default();
    }
#endif
    return (NSLevelIndicatorStyle)UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_level_set_cell_style(NSLevelIndicatorCell *cell, const enum gui_indicator_style_t style)
{
    cassert_no_null(cell);
    [cell setLevelIndicatorStyle:i_level_style(style)];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_attach_to_parent(NSView *control, NSView *parent)
{
    cassert_no_null(control);
    cassert_no_null(parent);
    [parent addSubview:control];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_detach_from_parent(NSView *control, NSView *parent)
{
    register NSUInteger rc, rp;
    cassert_no_null(control);
    cassert_no_null(parent);
    rc = [control retainCount];
    rp = [parent retainCount];
    cassert([control superview] == parent);
    [control removeFromSuperviewWithoutNeedingDisplay];
    /*if ([control retainCount] == rc)
    {
        cassert([control retainCount] > 1);
        [control release];
    }*/
    rp = [parent retainCount];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_visible(NSView *object, const bool_t is_visible)
{
    cassert_no_null(object);
    if ([object isHidden] == (BOOL)is_visible)
        [object setHidden:!(BOOL)is_visible];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_enabled(NSControl *object, const bool_t is_enabled)
{
    cassert_no_null(object);
    if ([object isEnabled] != (BOOL)is_enabled)
        [object setEnabled:(BOOL)is_enabled];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_get_size(const NSView *object, real32_t *width, real32_t *height)
{
    NSSize size;
    cassert_no_null(object);
    cassert_no_null(width);
    cassert_no_null(height);
    size = [object frame].size;
    *width = (real32_t)size.width;
    *height = (real32_t)size.height;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_get_origin(const NSView *object, real32_t *x, real32_t *y)
{
    NSRect rect;
    cassert_no_null(object);
    cassert_no_null(x);
    cassert_no_null(y);
    
    rect = [object frame];
    if (*x < 0.f)
    {
        NSView *parent = [object superview];
        cassert(*y < 0.f);

        /* Control origin in MainView coordinates */
        while (parent != nil)
        {
            NSRect parent_rect = [parent frame];
            rect.origin.x += parent_rect.origin.x;
            rect.origin.y += parent_rect.origin.y;
            parent = [parent superview];
        }

        /* MainView coordinates in Window Coordinates */
        {
            NSRect wframe = NSMakeRect(0.f, 0.f, 100.f, 100.f);
            wframe = [object.window frameRectForContentRect:wframe];
            rect.origin.y += wframe.size.height - 100.f;
        }
        
        /* Window Coordinates in Screen */
        {
            NSRect window_rect = [object.window frame];
            CGFloat origin_x, origin_y;
            _oscontrol_origin_in_screen_coordinates(&window_rect, &origin_x, &origin_y);
            *x = (real32_t)(origin_x + rect.origin.x);
            *y = (real32_t)(origin_y + rect.origin.y);
        }
    }
    else
    {
        *x = (real32_t)rect.origin.x;
        *y = (real32_t)rect.origin.y;
    }
}

/*---------------------------------------------------------------------------*/

/*
static void i_print_frame(NSView *object, const real32_t x, const real32_t y, const real32_t w, const real32_t h)
{
    NSString *class_name = NSStringFromClass([object class]);
    NSString *window_name = [[object window] title];
    printf("Origin %s - %s (%.2f, %.2f):\n", [class_name UTF8String], [window_name UTF8String], x, y);
    printf("Size %s - %s (%.2f, %.2f):\n", [class_name UTF8String], [window_name UTF8String], w, h);
}*/

/*---------------------------------------------------------------------------*/

#if defined (__ASSERTS__)

static BOOL i_check_control_frame(NSView *object)
{
    cassert_no_null(object);
    if ([object superview] == nil)
        return YES;

    if ([object window] == nil)
        return YES;

    if ([[object window] contentView] == object)
        return YES;

    {
        NSRect parent_frame;
        NSRect object_frame;
        parent_frame = [[object superview] frame];
        object_frame = [object frame];
        cassert(parent_frame.origin.x >= 0.f);
        cassert(parent_frame.origin.y >= 0.f);
        cassert(parent_frame.size.width >= 0.f);
        cassert(parent_frame.size.height >= 0.f);
//        cassert(floorf((float)object_frame.origin.x) == object_frame.origin.x);
//        cassert(floorf((float)object_frame.origin.y) == object_frame.origin.y);
//        cassert(floorf((float)object_frame.size.width) == object_frame.size.width);
//        cassert(floorf((float)object_frame.size.height) == object_frame.size.height);
        cassert(object_frame.origin.x >= 0.f);
        cassert(object_frame.origin.y >= 0.f);
        //cassert(object_frame.origin.x + object_frame.size.width <= parent_frame.size.width);
        //cassert(object_frame.origin.y + object_frame.size.height <= parent_frame.size.height);
    }
    return YES;
}

#endif

/*---------------------------------------------------------------------------*/

void _oscontrol_set_origin(NSView *object, const real32_t x, const real32_t y)
{
    cassert_no_null(object);
    [object setFrameOrigin:NSMakePoint((CGFloat)x, (CGFloat)y)];
}

/*---------------------------------------------------------------------------*/

/* 
    Changed 'oscontrol_set_size' and 'oscontrol_set_origin' for 'oscontrol_set_frame'
    From Cocoa documentation:
    Repositioning or resizing a view is a potentially complex operation. 
    When a view moves or resizes it can expose portions of its superview that weren’t previously visible, 
    requiring the superview to redisplay. Resizing can also affect the layout of the view’s subviews. 
    Changes to a view's layout in either case may be of interest to other objects, which might need to be notified 
    of the change. 
*/
void _oscontrol_set_frame(NSView *object, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(object);
    if (__TRUE_EXPECTED(x >= 0.f))
    {
        [object setFrame:NSMakeRect((CGFloat)x, (CGFloat)y, (CGFloat)width, (CGFloat)height)];
    }
    else
    {
        cassert(y < 0.f);
        [object setFrameSize:NSMakeSize((CGFloat)width, (CGFloat)height)];
    }
    cassert(i_check_control_frame(object) == YES);
}

