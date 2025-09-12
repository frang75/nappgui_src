/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol.m
 *
 */

/* Cocoa control */

#include "oscontrol_osx.inl"
#include "osgui_osx.inl"
#include "oscolor.inl"
#include "oslabel_osx.inl"
#include "osbutton_osx.inl"
#include "oscombo_osx.inl"
#include "osedit_osx.inl"
#include "ospanel_osx.inl"
#include "ospopup_osx.inl"
#include "osprogress_osx.inl"
#include "osslider_osx.inl"
#include "ossplit_osx.inl"
#include "ostext_osx.inl"
#include "osupdown_osx.inl"
#include "osview_osx.inl"
#include "osweb_osx.inl"
#include "oswindow_osx.inl"
#include "../osgui.inl"
#include "../oscontrol.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <draw2d/image.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

#if !defined(__MACOS__)
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

static ___INLINE NSTextAlignment i_text_alignment(const align_t halign)
{
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
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
    default:
        cassert_default(halign);
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
    default:
        cassert_default(halign);
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

static ___INLINE NSControlSize i_control_size(const gui_size_t size)
{
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    switch (size)
    {
    case ekGUI_SIZE_MINI:
        return NSControlSizeMini;
    case ekGUI_SIZE_SMALL:
        return NSControlSizeSmall;
    case ekGUI_SIZE_REGULAR:
        return NSControlSizeRegular;
    default:
        cassert_default(size);
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
    default:
        cassert_default(size);
    }
#endif

    return (NSControlSize)UINT32_MAX;
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

NSControlSize _oscontrol_control_size(const gui_size_t size)
{
    return i_control_size(size);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_cell_set_image(NSCell *cell, const Image *image)
{
    cassert_no_null(cell);
    if (image != NULL)
        [cell setImage:cast(image_native(image), NSImage)];
    else
        [cell setImage:nil];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_tooltip_set(NSView *view, const char_t *text)
{
    cassert_no_null(view);
    if (text != NULL)
    {
        NSString *str = [[NSString alloc] initWithUTF8String:cast_const(text, char)];
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
    return _oscolor_NSColor(color);
}

/*---------------------------------------------------------------------------*/

NSColor *_oscontrol_text_color(NSControl *control, const color_t color)
{
    if ([control isEnabled] == YES)
    {
        if (color == kCOLOR_DEFAULT)
        {
            if ([control isKindOfClass:[NSTextField class]])
            {
#if defined(MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
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

color_t _oscontrol_from_NSColor(NSColor *color)
{
    return _oscolor_from_NSColor(color);
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

    switch (align)
    {
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
    default:
        cassert_default(align);
    }

    objects[3] = _oscontrol_text_color(control, color);
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
    attr->mark = UINT32_MAX;
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
    NSCell *cell = [control cell];
    uint32_t fstyle = font_style(attrs->font);

    cassert_no_null(control);
    cassert_no_null(attrs);

    if (str == nil)
    {
        if ([cell isKindOfClass:[NSButtonCell class]])
            str = [cell title];
        else
            str = [cell stringValue];
    }

    {
        NSFont *font = cast(font_native(attrs->font), NSFont);
        NSDictionary *dict = i_text_attribs(control, attrs->align, attrs->color, fstyle, font);
        NSAttributedString *astr = [[NSAttributedString alloc] initWithString:str attributes:dict];
        NSMutableDictionary *mdict = nil;
        NSMutableAttributedString *mastr = nil;

        /* Underline mark for a character and font style is not underline */
        if (attrs->mark != UINT32_MAX && (fstyle & ekFUNDERLINE) == 0)
        {
            mdict = [[NSMutableDictionary alloc] initWithDictionary:dict];
            [mdict setValue:kUNDERLINE_STYLE_SINGLE forKey:NSUnderlineStyleAttributeName];
            mastr = [[NSMutableAttributedString alloc] initWithAttributedString:astr];
            [mastr setAttributes:mdict range:NSMakeRange(attrs->mark, 1)];
        }

        [cell setFont:font];

        if ([cell isKindOfClass:[NSButtonCell class]])
            [(NSButtonCell *)cell setAttributedTitle:mastr != nil ? mastr : astr];
        else
            [cell setAttributedStringValue:mastr != nil ? mastr : astr];

        [astr release];

        if (mdict != nil)
            [mdict release];

        if (mastr != nil)
            [mastr release];
    }
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_text(NSControl *control, const OSTextAttr *attrs, const char_t *text)
{
    NSString *str = [[NSString alloc] initWithUTF8String:cast_const(text, char)];
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

void _oscontrol_attach_to_parent(NSView *control, NSView *parent)
{
    cassert_no_null(control);
    cassert_no_null(parent);
    cassert([control superview] == nil);
    [parent addSubview:control];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_detach_from_parent(NSView *control, NSView *parent)
{
    NSView *sup = [control superview];
    cassert_no_null(control);
    cassert_no_null(parent);
    cassert([control superview] == parent);
    [control removeFromSuperviewWithoutNeedingDisplay];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_visible(NSView *object, const bool_t visible)
{
    cassert_no_null(object);
    [object setHidden:!(BOOL)visible];
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_enabled(NSControl *object, const bool_t enabled)
{
    cassert_no_null(object);
    if ([object isEnabled] != (BOOL)enabled)
        [object setEnabled:(BOOL)enabled];
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

    /* This block must be removed */
    if (*x < 0.f)
    {
        NSView *parent = [object superview];
        cassert(*y < 0.f);
        cassert(FALSE);
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

#if defined(__ASSERTS__)

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
        cassert(object_frame.origin.x >= 0.f);
        cassert(object_frame.origin.y >= 0.f);
    }
    return YES;
}

#endif

/*---------------------------------------------------------------------------*/

/*
    Changed 'oscontrol_set_size' and 'oscontrol_set_origin' for 'oscontrol_set_frame'
    From Cocoa documentation:
    Repositioning or resizing a view is a potentially complex operation.
    When a view moves or resizes it can expose portions of its superview that werenâ€™t previously visible,
    requiring the superview to redisplay. Resizing can also affect the layout of the viewâ€™s subviews.
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

/*---------------------------------------------------------------------------*/

static gui_type_t i_oscontrol_type(NSView *object)
{
    cassert_no_null(object);
    if (_oslabel_is(object) == YES)
        return ekGUI_TYPE_LABEL;

    if (_osbutton_is(object) == YES)
        return ekGUI_TYPE_BUTTON;

    if (_ospopup_is(object) == YES)
        return ekGUI_TYPE_POPUP;

    if (_osedit_is(object) == YES)
        return ekGUI_TYPE_EDITBOX;

    if (_oscombo_is(object) == YES)
        return ekGUI_TYPE_COMBOBOX;

    if (_osslider_is(object) == YES)
        return ekGUI_TYPE_SLIDER;

    if (_osupdown_is(object) == YES)
        return ekGUI_TYPE_UPDOWN;

    if (_osprogress_is(object) == YES)
        return ekGUI_TYPE_PROGRESS;

    if (_ostext_is(object) == YES)
        return ekGUI_TYPE_TEXTVIEW;

    if (_osweb_is(object) == YES)
        return ekGUI_TYPE_WEBVIEW;

    if (_ossplit_is(object) == YES)
        return ekGUI_TYPE_SPLITVIEW;

    if (_osview_is(object) == YES)
        return ekGUI_TYPE_CUSTOMVIEW;

    if (_ospanel_is(object) == YES)
        return ekGUI_TYPE_PANEL;

    /* Unknown control type */
    return ENUM_MAX(gui_type_t);
}

/*---------------------------------------------------------------------------*/

OSControl *_oscontrol_from_nsview(NSView *object)
{
    gui_type_t type = i_oscontrol_type(object);
    if (type != ENUM_MAX(gui_type_t))
        return cast(object, OSControl);
    return NULL;
}

/*---------------------------------------------------------------------------*/

gui_type_t _oscontrol_type(const OSControl *control)
{
    gui_type_t type = ENUM_MAX(gui_type_t);
    cassert_no_null(control);
    cassert([cast(control, NSObject) isKindOfClass:[NSView class]] == YES);
    type = i_oscontrol_type(cast(control, NSView));
    cassert(type != ENUM_MAX(gui_type_t));
    return type;
}

/*---------------------------------------------------------------------------*/

OSControl *_oscontrol_parent(const OSControl *control)
{
    cassert_no_null(control);
    cassert([cast(control, NSObject) isKindOfClass:[NSView class]] == YES);
    return cast([cast(control, NSView) superview], OSControl);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_frame(const OSControl *control, OSFrame *rect)
{
    real32_t x, y, w, h;
    cassert_no_null(control);
    cassert_no_null(rect);
    cassert([cast(control, NSObject) isKindOfClass:[NSView class]] == YES);
    _oscontrol_get_origin(cast(control, NSView), &x, &y);
    _oscontrol_get_size(cast(control, NSView), &w, &h);
    rect->left = (int32_t)x;
    rect->top = (int32_t)y;
    rect->right = rect->left + (int32_t)w;
    rect->bottom = rect->top + (int32_t)h;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_can_focus(OSControl *control, const bool_t can_focus)
{
    unref(control);
    unref(can_focus);
}

/*---------------------------------------------------------------------------*/

OSWidget *_oscontrol_focus_widget(const OSControl *control)
{
    cassert_no_null(control);
    cassert([cast(control, NSObject) isKindOfClass:[NSView class]] == YES);
    if (_osview_is(cast(control, NSView)) == YES)
        return cast(_osview_focus_widget(cast(control, NSView)), OSWidget);
    return cast(control, OSWidget);
}

/*---------------------------------------------------------------------------*/

bool_t _oscontrol_widget_visible(const OSWidget *widget)
{
    cassert_no_null(widget);
    return (bool_t) ![cast(widget, NSView) isHidden];
}

/*---------------------------------------------------------------------------*/

bool_t _oscontrol_widget_enable(const OSWidget *widget)
{
    gui_type_t type = i_oscontrol_type(cast(widget, NSView));
    cassert_no_null(widget);
    switch (type)
    {
    case ekGUI_TYPE_LABEL:
    case ekGUI_TYPE_PROGRESS:
        return TRUE;

    case ekGUI_TYPE_EDITBOX:
        return _osedit_is_enabled(cast(widget, NSView));

    case ekGUI_TYPE_COMBOBOX:
        return _oscombo_is_enabled(cast(widget, NSView));

    case ekGUI_TYPE_TEXTVIEW:
        return _ostext_is_enabled(cast(widget, NSView));

    case ekGUI_TYPE_BUTTON:
    case ekGUI_TYPE_POPUP:
    case ekGUI_TYPE_SLIDER:
    case ekGUI_TYPE_UPDOWN:
        cassert([cast(widget, NSObject) isKindOfClass:[NSControl class]] == YES);
        return (bool_t)[cast(widget, NSControl) isEnabled];

    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_CUSTOMVIEW:
        return TRUE;

    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
    default:
        cassert_default(type);
    }
}
