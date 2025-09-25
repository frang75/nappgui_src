/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.m
 *
 */

/* Operating System native button */

#include "osbutton_osx.inl"
#include "oscontrol_osx.inl"
#include "ospanel_osx.inl"
#include "oswindow_osx.inl"
#include "osgui_osx.inl"
#include "osglobals.inl"
#include "../osbutton.h"
#include "../osbutton.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <draw2d/image.h>
#include <core/event.h>
#include <core/heap.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXButtonCell : NSButtonCell
{
  @public
    uint32_t flags;
    gui_size_t size;
    real32_t text_width;
    Image *image;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXButton : NSButton
{
  @public
    uint32_t flags;
    uint32_t hpadding;
    uint32_t vpadding;
    OSTextAttr attrs;
    NSString *keyEquivalent;
    Listener *OnClick;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXButton

/*---------------------------------------------------------------------------*/

/* From Mountain Lion to HighSierra render focus issue */
#if (defined MAC_OS_X_VERSION_10_7 && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_7) || (!defined MAC_OS_X_VERSION_10_7) || (defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
#else
- (void)drawRect:(NSRect)rect
{
    [super drawRect:rect];
    if ([[self window] firstResponder] == self)
    {
        NSSetFocusRingStyle(NSFocusRingOnly);
        if (button_get_type(self->flags) == ekBUTTON_FLAT || button_get_type(self->flags) == ekBUTTON_FLATGLE)
        {
            rect.origin.x += 2;
            rect.origin.y += 2;
            rect.size.width -= 4;
            rect.size.height -= 4;
        }
        NSRectFill(rect);
    }
}
#endif

/*---------------------------------------------------------------------------*/

static gui_state_t i_get_state(const OSXButton *button)
{
#if defined(MAC_OS_X_VERSION_10_13) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
    NSControlStateValue state = 0;
    cassert_no_null(button);
    state = [button state];
    if (state == NSControlStateValueOn)
    {
        return ekGUI_ON;
    }
    else if (state == NSControlStateValueOff)
    {
        return ekGUI_OFF;
    }
    else
    {
        cassert(state == NSControlStateValueMixed);
        return ekGUI_MIXED;
    }
#else
    NSInteger state = 0;
    cassert_no_null(button);
    state = [button state];
    if (state == NSOnState)
    {
        return ekGUI_ON;
    }
    else if (state == NSOffState)
    {
        return ekGUI_OFF;
    }
    else
    {
        cassert(state == NSMixedState);
        return ekGUI_MIXED;
    }
#endif
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(OSXButton *button)
{
    if ([button isEnabled] == YES)
    {
        gui_state_t state = i_get_state(button);
        if (button_get_type(button->flags) == ekBUTTON_FLATGLE)
        {
            switch (state)
            {
            case ekGUI_ON:
                [[button cell] setShowsBorderOnlyWhileMouseInside:NO];
                break;
            case ekGUI_OFF:
                [[button cell] setShowsBorderOnlyWhileMouseInside:YES];
                break;
            default:
                cassert_default(state);
            }
        }
        else if (button_get_type(button->flags) == ekBUTTON_CHECK3)
        {
            if (state == ekGUI_MIXED)
            {
                state = ekGUI_ON;
                osbutton_state(cast(button, OSButton), state);
            }
        }

        if (button->OnClick != NULL)
        {
            EvButton params;
            params.index = 0;
            params.state = state;
            params.text = NULL;
            listener_event(button->OnClick, ekGUI_EVENT_BUTTON, cast(button, OSButton), &params, NULL, OSButton, EvButton, void);
        }
    }
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickButton:(id)sender
{
    cassert_no_null(sender);
    cassert(sender == self);
    i_OnClick(self);
    _oswindow_release_transient_focus(cast(self, OSControl));
}

/*---------------------------------------------------------------------------*/

- (void)mouseDown:(NSEvent *)theEvent
{
    if (_oswindow_mouse_down(cast(self, OSControl)) == TRUE)
        [super mouseDown:theEvent];
}

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_8

- (IBAction)onClickRadio00:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio01:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio02:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio03:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio04:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio05:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio06:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio07:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio08:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio09:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio10:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio11:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio12:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio13:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio14:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio15:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio16:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio17:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio18:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio19:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio20:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio21:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio22:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio23:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio24:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio25:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio26:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio27:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio28:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio29:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio30:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio31:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio32:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio33:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio34:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio35:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio36:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio37:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio38:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio39:(id)sender
{
    [self onClickButton:sender];
}

/*---------------------------------------------------------------------------*/

- (IBAction)onClickRadio40:(id)sender
{
    [self onClickButton:sender];
}

#endif

@end

/*---------------------------------------------------------------------------*/

@implementation OSXButtonCell

/*---------------------------------------------------------------------------*/

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView
{
    if (button_get_type(self->flags) == ekBUTTON_PUSH)
    {
        if (size == ekGUI_SIZE_MINI)
        {
            NSImage *nsimage = [self image];
            if (nsimage != nil)
            {
                NSBitmapImageRep *image_rep = cast([[cast(nsimage, NSImage) representations] objectAtIndex:0], NSBitmapImageRep);
                NSInteger width = [image_rep pixelsWide];
                frame.origin.x -= (CGFloat)(width - 4);
                frame.size.width += (CGFloat)(width - 4);
            }
        }
    }

    return [super drawTitle:title withFrame:frame inView:controlView];
}

/*---------------------------------------------------------------------------*/

- (void)drawImage:(NSImage *)nsimage withFrame:(NSRect)frame inView:(NSView *)controlView
{
    if (nsimage != nil)
    {
        if (button_get_type(self->flags) == ekBUTTON_PUSH)
        {
            NSBitmapImageRep *image_rep = nil;
            NSInteger width, height;
            NSInteger offset_x = 0, offset_y = 0;
            NSRect rframe = [controlView frame];
            image_rep = cast([[nsimage representations] objectAtIndex:0], NSBitmapImageRep);
            cassert_no_null(image_rep);
            width = [image_rep pixelsWide];
            height = [image_rep pixelsHigh];
            offset_x = ((NSInteger)rframe.size.width - (NSInteger)self->text_width) / 2;
            offset_y = ((NSInteger)rframe.size.height - height) / 2;

            if (self->size == ekGUI_SIZE_REGULAR)
                offset_y -= 1;
            else if (self->size == ekGUI_SIZE_SMALL)
                offset_y -= 1;

            frame.size.width = (CGFloat)width;
            frame.size.height = (CGFloat)height;
            frame.origin.x = (CGFloat)offset_x;
            frame.origin.y = (CGFloat)offset_y;
            [super drawImage:nsimage withFrame:frame inView:controlView];
        }
        else
        {
            [super drawImage:nsimage withFrame:frame inView:controlView];
        }
    }
}

/*---------------------------------------------------------------------------*/

static NSRect i_pushbutton_cell_frame(NSRect rect, const gui_size_t size)
{
#if defined(MAC_OS_VERSION_26_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_26_0
    /* Tahoe fits the pushbutton cell to frame */
    unref(size);
    return rect;
#else
    switch (size)
    {
    case ekGUI_SIZE_MINI:
        rect.origin.y -= 0.f;
        break;
    case ekGUI_SIZE_REGULAR:
        rect.origin.x -= 5.f;
        rect.origin.y -= 1.f;
        rect.size.width += 10.f;
        rect.size.height += 4.f;
        break;
    case ekGUI_SIZE_SMALL:
        rect.origin.x -= 5.f;
        rect.origin.y += 1.f;
        rect.size.width += 10.f;
        break;
    default:
        break;
    }

    return rect;
#endif
}

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7

- (void)drawFocusRingMaskWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if (button_get_type(self->flags) == ekBUTTON_PUSH)
        cellFrame = i_pushbutton_cell_frame(cellFrame, self->size);
    [super drawFocusRingMaskWithFrame:cellFrame inView:controlView];
}

#endif

/*---------------------------------------------------------------------------*/

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    /*NSFrameRect(cellFrame);*/
    if (button_get_type(self->flags) == ekBUTTON_PUSH)
    {
        cellFrame = i_pushbutton_cell_frame(cellFrame, self->size);
    }
    else if (button_get_type(self->flags) == ekBUTTON_FLAT || button_get_type(self->flags) == ekBUTTON_FLATGLE)
    {
        cellFrame.origin.x -= 1.f;
        cellFrame.origin.y -= 1.f;
        cellFrame.size.width += 2.f;
        cellFrame.size.height += 2.f;
    }

    [super drawWithFrame:cellFrame inView:controlView];
}

@end

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_8

static void i_set_button_action(OSXButton *button, NSUInteger radio_index)
{
    cassert(radio_index < 40);
    radio_index = radio_index % 40;
    switch (radio_index)
    {
    case 0:
        [button setAction:@selector(onClickRadio00:)];
        break;
    case 1:
        [button setAction:@selector(onClickRadio01:)];
        break;
    case 2:
        [button setAction:@selector(onClickRadio02:)];
        break;
    case 3:
        [button setAction:@selector(onClickRadio03:)];
        break;
    case 4:
        [button setAction:@selector(onClickRadio04:)];
        break;
    case 5:
        [button setAction:@selector(onClickRadio05:)];
        break;
    case 6:
        [button setAction:@selector(onClickRadio06:)];
        break;
    case 7:
        [button setAction:@selector(onClickRadio07:)];
        break;
    case 8:
        [button setAction:@selector(onClickRadio08:)];
        break;
    case 9:
        [button setAction:@selector(onClickRadio09:)];
        break;
    case 10:
        [button setAction:@selector(onClickRadio10:)];
        break;
    case 11:
        [button setAction:@selector(onClickRadio11:)];
        break;
    case 12:
        [button setAction:@selector(onClickRadio12:)];
        break;
    case 13:
        [button setAction:@selector(onClickRadio13:)];
        break;
    case 14:
        [button setAction:@selector(onClickRadio14:)];
        break;
    case 15:
        [button setAction:@selector(onClickRadio15:)];
        break;
    case 16:
        [button setAction:@selector(onClickRadio16:)];
        break;
    case 17:
        [button setAction:@selector(onClickRadio17:)];
        break;
    case 18:
        [button setAction:@selector(onClickRadio18:)];
        break;
    case 19:
        [button setAction:@selector(onClickRadio19:)];
        break;
    case 20:
        [button setAction:@selector(onClickRadio20:)];
        break;
    case 21:
        [button setAction:@selector(onClickRadio21:)];
        break;
    case 22:
        [button setAction:@selector(onClickRadio22:)];
        break;
    case 23:
        [button setAction:@selector(onClickRadio23:)];
        break;
    case 24:
        [button setAction:@selector(onClickRadio24:)];
        break;
    case 25:
        [button setAction:@selector(onClickRadio25:)];
        break;
    case 26:
        [button setAction:@selector(onClickRadio26:)];
        break;
    case 27:
        [button setAction:@selector(onClickRadio27:)];
        break;
    case 28:
        [button setAction:@selector(onClickRadio28:)];
        break;
    case 29:
        [button setAction:@selector(onClickRadio29:)];
        break;
    case 30:
        [button setAction:@selector(onClickRadio30:)];
        break;
    case 31:
        [button setAction:@selector(onClickRadio31:)];
        break;
    case 32:
        [button setAction:@selector(onClickRadio32:)];
        break;
    case 33:
        [button setAction:@selector(onClickRadio33:)];
        break;
    case 34:
        [button setAction:@selector(onClickRadio34:)];
        break;
    case 35:
        [button setAction:@selector(onClickRadio35:)];
        break;
    case 36:
        [button setAction:@selector(onClickRadio36:)];
        break;
    case 37:
        [button setAction:@selector(onClickRadio37:)];
        break;
    case 38:
        [button setAction:@selector(onClickRadio38:)];
        break;
    case 39:
        [button setAction:@selector(onClickRadio39:)];
        break;
    default:
        cassert_default(radio_index);
    }
}

/*---------------------------------------------------------------------------*/

static void i_recompute_button_action(OSXButton *button, NSView *parent_view)
{
    cassert_no_null(button);
    cassert_no_null(parent_view);
    /* Use of NSMatrix is informally deprecated. We expect to add the formal deprecation macros in
     * a subsequent release, but its use is discouraged in the mean time.
     * The primary use of NSMatrix is for radio button groups, so recall that for applications linked
     * on 10.8 or later, radio buttons that share the same parent view and ACTION will operate as a group.
     *
     * This avoid the automatic radiobutton grouping in OSX 10.8 and later.
     * It's assing a different ACTION selector for each radio button in superview
     */
    if (button_get_type(button->flags) == ekBUTTON_RADIO)
    {
        NSUInteger radio_index = 0;
        NSArray *children = [parent_view subviews];
        NSUInteger i, count;
        count = [children count];
        for (i = 0; i < count; ++i)
        {
            NSView *child = cast([children objectAtIndex:i], NSView);
            if ([child isKindOfClass:[OSXButton class]])
            {
                OSXButton *child_button = cast(child, OSXButton);
                if (button_get_type(child_button->flags) == ekBUTTON_RADIO)
                {
                    i_set_button_action(child_button, radio_index);
                    radio_index += 1;
                }
            }
        }
    }
}

#else

#define i_recompute_button_action(button, parent_view) ((void)button, (void)parent_view)

#endif

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
#define ROUNDED_BEZEL NSBezelStyleRounded
#define REGULAR_SQUARE_BEZEL NSBezelStyleRegularSquare
#define PUSH_IN_BUTTON NSButtonTypeMomentaryPushIn
#define ON_OFF_BUTTON NSButtonTypeOnOff
#define SWITCH_BUTTON NSButtonTypeSwitch
#define RADIO_BUTTON NSButtonTypeRadio
#else
#define ROUNDED_BEZEL NSRoundedBezelStyle
#define REGULAR_SQUARE_BEZEL NSRegularSquareBezelStyle
#define PUSH_IN_BUTTON NSMomentaryPushInButton
#define ON_OFF_BUTTON NSOnOffButton
#define SWITCH_BUTTON NSSwitchButton
#define RADIO_BUTTON NSRadioButton
#endif

#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
#define KEY_MODIFIER_OPTION NSEventModifierFlagCommand
#else
#define KEY_MODIFIER_OPTION NSCommandKeyMask
#endif

static void i_set_button_type(OSXButton *button, OSXButtonCell *cell, const uint32_t flags)
{
    switch (button_get_type(flags))
    {
    case ekBUTTON_PUSH:
        [cell setBezelStyle:ROUNDED_BEZEL];
        [cell setBordered:YES];
        [cell setShowsBorderOnlyWhileMouseInside:NO];
        [cell setImagePosition:NSNoImage];
        [button setButtonType:PUSH_IN_BUTTON];
        break;

    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
        [cell setBezelStyle:REGULAR_SQUARE_BEZEL];
        [cell setBordered:NO];
        /* macos 10.15 Catalina hides the check image */
        /* [cell setImagePosition:NSImageLeft]; */
        [cell setShowsBorderOnlyWhileMouseInside:NO];
        [cell setImageScaling:NSImageScaleNone];
        [button setButtonType:SWITCH_BUTTON];
        [button setAllowsMixedState:button_get_type(flags) == ekBUTTON_CHECK3 ? YES : NO];
        break;

    case ekBUTTON_RADIO:
        [cell setBezelStyle:REGULAR_SQUARE_BEZEL];
        [cell setBordered:NO];
        [cell setShowsBorderOnlyWhileMouseInside:NO];
        [cell setImageScaling:NSImageScaleNone];
        [button setButtonType:RADIO_BUTTON];
        break;

    case ekBUTTON_FLAT:
        [cell setBezelStyle:REGULAR_SQUARE_BEZEL];
        [cell setBordered:YES];
        [cell setShowsBorderOnlyWhileMouseInside:YES];
        [cell setImageScaling:NSImageScaleNone];
        [button setButtonType:PUSH_IN_BUTTON];
        break;

    case ekBUTTON_FLATGLE:
        [cell setBezelStyle:REGULAR_SQUARE_BEZEL];
        [cell setBordered:YES];
        [cell setShowsBorderOnlyWhileMouseInside:YES];
        [cell setImageScaling:NSImageScaleNone];
        [button setButtonType:PUSH_IN_BUTTON];
        break;

    default:
        cassert_default(button_get_type(flags));
    }
}

/*---------------------------------------------------------------------------*/

OSButton *osbutton_create(const uint32_t flags)
{
    OSXButton *button = nil;
    OSXButtonCell *cell = nil;
    heap_auditor_add("OSXButton");
    heap_auditor_add("OSXButtonCell");
    button = [[OSXButton alloc] initWithFrame:NSZeroRect];
    button->flags = flags;
    button->hpadding = UINT32_MAX;
    button->vpadding = UINT32_MAX;
    button->OnClick = NULL;
    _oscontrol_init(button);
    cell = [[OSXButtonCell alloc] init];
    cell->flags = button->flags;
    cell->image = NULL;
    [button setCell:cell];
    [button setTarget:button];
    [button setAction:@selector(onClickButton:)];
    button->keyEquivalent = nil;
    i_set_button_type(button, cell, flags);

    if (_osbutton_text_allowed(flags) == TRUE)
    {
        _oscontrol_init_textattr(&button->attrs);
        _oscontrol_set_align(button, &button->attrs, ekCENTER);
        _oscontrol_set_font(button, &button->attrs, button->attrs.font);
        cell->size = _osgui_size_font(font_size(button->attrs.font));
        _oscontrol_set_text(button, &button->attrs, "");
        _oscontrol_size_from_font(cell, button->attrs.font);
    }
    else
    {
        bmem_zero(&button->attrs, OSTextAttr);
        cell->size = ENUM_MAX(gui_size_t);
    }

    return cast(button, OSButton);
}

/*---------------------------------------------------------------------------*/

void osbutton_destroy(OSButton **button)
{
    OSXButton *buttonp = nil;
    OSXButtonCell *cell = nil;
    cassert_no_null(button);
    buttonp = *dcast(button, OSXButton);
    cassert_no_null(buttonp);
    listener_destroy(&buttonp->OnClick);
    _oscontrol_remove_textattr(&buttonp->attrs);
    cell = [buttonp cell];

    if (buttonp->keyEquivalent != nil)
        [buttonp->keyEquivalent release];

    ptr_destopt(image_destroy, &cell->image, Image);
    [cell release];
    [buttonp release];
    *button = NULL;
    heap_auditor_delete("OSXButton");
    heap_auditor_delete("OSXButtonCell");
}

/*---------------------------------------------------------------------------*/

void osbutton_OnClick(OSButton *button, Listener *listener)
{
    cassert_no_null(button);
    listener_update(&cast(button, OSXButton)->OnClick, listener);
}

/*---------------------------------------------------------------------------*/

void osbutton_text(OSButton *button, const char_t *text)
{
    OSXButton *lbutton = cast(button, OSXButton);
    char_t tbuff[256];
    cassert_no_null(lbutton);
    cassert(_osbutton_text_allowed(lbutton->flags) == TRUE);
    lbutton->attrs.mark = _osgui_key_equivalent_text(text, tbuff, sizeof(tbuff));
    _oscontrol_set_text(lbutton, &lbutton->attrs, tbuff);

    if (lbutton->keyEquivalent != nil)
    {
        [lbutton->keyEquivalent release];
        lbutton->keyEquivalent = nil;
    }

    if (lbutton->attrs.mark != UINT32_MAX)
    {
        const char_t *pos = unicode_move(tbuff, lbutton->attrs.mark, ekUTF8);
        uint32_t cp = unicode_to_u32(pos, ekUTF8);
        unichar c = (unichar)unicode_tolower(cp);
        lbutton->keyEquivalent = [[NSString alloc] initWithCharacters:&c length:1];
        [lbutton setKeyEquivalent:lbutton->keyEquivalent];
        [lbutton setKeyEquivalentModifierMask:KEY_MODIFIER_OPTION];
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_tooltip(OSButton *button, const char_t *text)
{
    cassert_no_null(button);
    _oscontrol_tooltip_set(cast(button, OSXButton), text);
}

/*---------------------------------------------------------------------------*/

void osbutton_font(OSButton *button, const Font *font)
{
    OSXButton *lbutton = cast(button, OSXButton);
    OSXButtonCell *cell = [lbutton cell];
    cassert_no_null(lbutton);
    cassert(_osbutton_text_allowed(lbutton->flags) == TRUE);
    _oscontrol_set_font(lbutton, &lbutton->attrs, font);
    _oscontrol_size_from_font([lbutton cell], lbutton -> attrs.font);
    cell->size = _osgui_size_font(font_size(lbutton->attrs.font));
}

/*---------------------------------------------------------------------------*/

void osbutton_align(OSButton *button, const align_t align)
{
    OSXButton *lbutton = cast(button, OSXButton);
    cassert_no_null(lbutton);
    cassert(_osbutton_text_allowed(lbutton->flags) == TRUE);
    _oscontrol_set_align(lbutton, &lbutton->attrs, align);
}

/*---------------------------------------------------------------------------*/

void osbutton_image(OSButton *button, const Image *image)
{
    OSXButton *lbutton = nil;
    OSXButtonCell *cell = nil;
    lbutton = cast(button, OSXButton);
    cassert_no_null(lbutton);
    cassert(_osbutton_image_allowed(lbutton->flags) == TRUE);
    cell = [lbutton cell];
    if (button_get_type(cell->flags) == ekBUTTON_PUSH)
    {
        ptr_destopt(image_destroy, &cell->image, Image);
        if (image != NULL)
        {
            cell->image = image_copy(image);
            [cell setImagePosition:NSImageLeft];
            _oscontrol_cell_set_image(cell, image);
        }
        else
        {
            [cell setImagePosition:NSNoImage];
            _oscontrol_cell_set_image(cell, NULL);
        }
    }
    else if (button_get_type(cell->flags) == ekBUTTON_FLAT || button_get_type(cell->flags) == ekBUTTON_FLATGLE)
    {
        _oscontrol_cell_set_image(cell, image);
    }
    else
    {
        cassert_msg(FALSE, "Button doesn't accept images.");
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_state(OSButton *button, const gui_state_t state)
{
#if defined(MAC_OS_X_VERSION_10_13) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
    NSControlStateValue nsstate = NSControlStateValueOn;
    cassert_no_null(button);
    switch (state)
    {
    case ekGUI_ON:
        nsstate = NSControlStateValueOn;
        break;
    case ekGUI_OFF:
        nsstate = NSControlStateValueOff;
        break;
    case ekGUI_MIXED:
        nsstate = NSControlStateValueMixed;
        break;
    default:
        cassert_default(state);
    }
#else
    NSInteger nsstate = NSOnState;
    switch (state)
    {
    case ekGUI_ON:
        nsstate = NSOnState;
        break;
    case ekGUI_OFF:
        nsstate = NSOffState;
        break;
    case ekGUI_MIXED:
        nsstate = NSMixedState;
        break;
    default:
        cassert_default(state);
    }
#endif
    [cast(button, OSXButton) setState:nsstate];
}

/*---------------------------------------------------------------------------*/

gui_state_t osbutton_get_state(const OSButton *button)
{
    return i_get_state(cast_const(button, OSXButton));
}

/*---------------------------------------------------------------------------*/

void osbutton_hpadding(OSButton *button, const real32_t padding)
{
    OSXButton *lbutton = cast(button, OSXButton);
    cassert_no_null(lbutton);
    if (padding >= 0)
        lbutton->hpadding = (uint32_t)padding;
    else
        lbutton->hpadding = UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

void osbutton_vpadding(OSButton *button, const real32_t padding)
{
    OSXButton *lbutton = cast(button, OSXButton);
    cassert_no_null(lbutton);
    if (padding >= 0)
        lbutton->vpadding = (uint32_t)padding;
    else
        lbutton->vpadding = UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

void osbutton_bounds(const OSButton *button, const char_t *text, const real32_t refwidth, const real32_t refheight, real32_t *width, real32_t *height)
{
    OSXButton *lbutton = cast(button, OSXButton);
    cassert_no_null(lbutton);
    cassert_no_null(width);
    cassert_no_null(height);

    switch (button_get_type(lbutton->flags))
    {
    case ekBUTTON_PUSH:
    {
        char_t tbuff[256];
        real32_t woff, hoff;
        OSXButtonCell *cell = [lbutton cell];
        real32_t xscale = font_xscale(lbutton->attrs.font);
        uint32_t imgwidth = 0;
        _osgui_key_equivalent_text(text, tbuff, sizeof(tbuff));
        font_extents(lbutton->attrs.font, tbuff, -1.f, width, height);
        font_extents(lbutton->attrs.font, "O", -1.f, &woff, &hoff);

        if (lbutton->hpadding == UINT32_MAX)
        {
            *width += 4 * woff;
        }
        else
        {
            if (lbutton->hpadding < 2 * woff)
                *width += 2 * woff;
            else
                *width += (real32_t)lbutton->hpadding;
        }

        cell->text_width = *width;

        if (cell->image != NULL)
            imgwidth = image_width(cell->image);

        switch (cell->size)
        {
        case ekGUI_SIZE_REGULAR:
            if (imgwidth > 0)
                *width += (real32_t)imgwidth;
                /* Tahoe best fit */
#if defined(MAC_OS_VERSION_26_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_26_0
            *height = 25.f;
#else
            *height = 22.f;
#endif
            break;

        case ekGUI_SIZE_SMALL:
            if (imgwidth > 0)
                *width += (real32_t)imgwidth;
            *height = 20.f;
            break;

        case ekGUI_SIZE_MINI:
            if (imgwidth > 0)
                *width += (real32_t)imgwidth;
            else
                *width += 2.f;
            *height = 16.f;
            break;
        }

        if (lbutton->vpadding != UINT32_MAX)
        {
            *height += (real32_t)lbutton->vpadding;
            [[lbutton cell] setBezelStyle:REGULAR_SQUARE_BEZEL];
        }

        break;
    }

    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_RADIO:
    {
        OSXButtonCell *cell = [lbutton cell];
#if (defined MAC_OS_X_VERSION_10_7 && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_7) || (defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
        static const real32_t i_CHECK_TEXT_SEP = 6.f;
#else
        /* Inexplicable crash in HighSierra and lowers */
        static const real32_t i_CHECK_TEXT_SEP = 8.f;
#endif
        font_extents(lbutton->attrs.font, text, -1.f, width, height);
        switch (cell->size)
        {
        case ekGUI_SIZE_REGULAR:
            *width += (real32_t)_osglobals_check_width() + i_CHECK_TEXT_SEP;
            *height = 18.f;
            break;
        case ekGUI_SIZE_SMALL:
            *width += (real32_t)_osglobals_check_width() + i_CHECK_TEXT_SEP;
            *height = 16.f;
            break;
        case ekGUI_SIZE_MINI:
            *width += (real32_t)_osglobals_check_width() + i_CHECK_TEXT_SEP;
            *height = 12.f;
            break;
        }
        break;
    }

    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        if (lbutton->hpadding == UINT32_MAX)
            *width = (real32_t)(uint32_t)((refwidth * 1.5f) + .5f);
        else
            *width = refwidth + (real32_t)lbutton->hpadding;

        if (lbutton->vpadding == UINT32_MAX)
            *height = (real32_t)(uint32_t)((refheight * 1.5f) + .5f);
        else
            *height = refheight + (real32_t)lbutton->vpadding;

        if (refwidth <= 16.f)
            *width += 4.f;

        if (refheight <= 16.f)
            *height += 4.f;
        break;

    default:
        cassert_default(button_get_type(lbutton->flags));
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_attach(OSButton *button, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(button, NSView));
    i_recompute_button_action(cast(button, OSXButton), cast(panel, NSView));
}

/*---------------------------------------------------------------------------*/

void osbutton_detach(OSButton *button, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(button, NSView));
    i_recompute_button_action(cast(button, OSXButton), cast(panel, NSView));
}

/*---------------------------------------------------------------------------*/

void osbutton_visible(OSButton *button, const bool_t visible)
{
    _oscontrol_set_visible(cast(button, NSView), visible);
}

/*---------------------------------------------------------------------------*/

void osbutton_enabled(OSButton *button, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(button, NSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osbutton_size(const OSButton *button, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast(button, NSView), width, height);
}

/*---------------------------------------------------------------------------*/

void osbutton_origin(const OSButton *button, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast(button, NSView), x, y);
}

/*---------------------------------------------------------------------------*/

void osbutton_frame(OSButton *button, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(button, NSView), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

BOOL _osbutton_is(NSView *view)
{
    return [view isKindOfClass:[OSXButton class]];
}

/*---------------------------------------------------------------------------*/

BOOL _osbutton_OnIntro(NSResponder *resp)
{
    if (resp != nil && [resp isKindOfClass:[OSXButton class]])
    {
        OSXButton *button = cast(resp, OSXButton);
        if (button_get_type(button->flags) == ekBUTTON_PUSH)
        {
            i_OnClick(button);
            return YES;
        }
    }

    return NO;
}

/*---------------------------------------------------------------------------*/

void _osbutton_set_default(OSButton *button, const bool_t is_default)
{
    OSXButton *lbutton = cast(button, OSXButton);
    cassert_no_null(lbutton);
    if (is_default == TRUE)
    {
        [lbutton setKeyEquivalent:@"\r"];
    }
    else
    {
        if (lbutton->keyEquivalent != nil)
        {
            [lbutton setKeyEquivalent:lbutton->keyEquivalent];
            [lbutton setKeyEquivalentModifierMask:KEY_MODIFIER_OPTION];
        }
        else
        {
            [lbutton setKeyEquivalent:@""];
        }
    }
}
