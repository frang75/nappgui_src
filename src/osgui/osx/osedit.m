/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.m
 *
 */

/* Operating System edit box */

#include "osgui_osx.inl"
#include "osedit.h"
#include "osedit.inl"
#include "osx/oscolor.inl"
#include "osgui.inl"
#include "oscontrol.inl"
#include "ospanel.inl"
#include "oswindow.inl"
#include "cassert.h"
#include "color.h"
#include "heap.h"
#include "event.h"
#include "strings.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXTextField : NSTextField
{
    @public
    NSView *parent;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXSecureTextField : NSSecureTextField
{
    @public
    NSView *parent;
}
@end

/*---------------------------------------------------------------------------*/

@interface OSXEdit : NSView
{
    @public
    NSTextField *field;
    uint32_t flags;
    OSTextAttr attrs;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXEdit

-(BOOL)acceptsFirstResponder
{
    return [self->field acceptsFirstResponder];
}

/*---------------------------------------------------------------------------*/

-(BOOL)makeFirstResponder:(NSResponder*)responder
{
    unref(responder);
    return YES;
}

/*---------------------------------------------------------------------------*/

-(BOOL)becomeFirstResponder
{
    return [self->field becomeFirstResponder];
}

/*---------------------------------------------------------------------------*/

- (BOOL)resignFirstResponder
{
    return [self->field resignFirstResponder];
}

@end

/*---------------------------------------------------------------------------*/

static void OSX_becomeFirstResponder(OSXEdit *edit, NSTextField *field)
{
    NSText *text = nil;

    if ([field isEnabled] == YES && edit->OnFocus != NULL)
    {
        bool_t params = TRUE;
        listener_event(edit->OnFocus, ekGUI_EVENT_FOCUS, (OSEdit*)edit, &params, NULL, OSEdit, bool_t, void);
    }

    text = [[field window] fieldEditor:YES forObject:field];

    if (BIT_TEST(edit->flags, ekEDIT_AUTOSEL) == TRUE)
    {
        [text selectAll:nil];
    }
    else
    {
        NSRange range = [text selectedRange];
        [text setSelectedRange:NSMakeRange(range.length, 0)];
    }
}

/*---------------------------------------------------------------------------*/

static void OSX_textDidChange(OSXEdit *edit, NSTextField *field)
{
    if ([field isEnabled] == YES && edit->OnFilter != NULL)
    {
        EvText params;
        EvTextFilter result;
        NSText *text = NULL;
        params.text = (const char_t*)[[field stringValue] UTF8String];
        text = [[field window] fieldEditor:YES forObject:field];
        params.cpos = (uint32_t)[text selectedRange].location;
        result.apply = FALSE;
        result.text[0] = '\0';
        result.cpos = UINT32_MAX;
        listener_event(edit->OnFilter, ekGUI_EVENT_TXTFILTER, (OSEdit*)edit, &params, &result, OSEdit, EvText, EvTextFilter);

        if (result.apply == TRUE)
            _oscontrol_set_text(field, &edit->attrs, result.text);

        if (result.cpos != UINT32_MAX)
            [text setSelectedRange:NSMakeRange((NSUInteger)result.cpos, 0)];
        else
            [text setSelectedRange:NSMakeRange((NSUInteger)params.cpos, 0)];
    }
}

/*---------------------------------------------------------------------------*/

static void OSX_textDidEndEditing(OSXEdit *edit, NSTextField *field, NSNotification *notification)
{
    if ([field isEnabled] == YES && edit->OnChange != NULL
        && _oswindow_in_destroy([field window]) == NO)
    {
        EvText params;
        params.text = (const char_t*)[[field stringValue] UTF8String];
        listener_event(edit->OnChange, ekGUI_EVENT_TXTCHANGE, (OSEdit*)edit, &params, NULL, OSEdit, EvText, void);
    }

    [[field window] endEditingFor:nil];

    if (edit->OnFocus != NULL)
    {
        bool_t params = FALSE;
        listener_event(edit->OnFocus, ekGUI_EVENT_FOCUS, (OSEdit*)edit, &params, NULL, OSEdit, bool_t, void);
    }

    {
        unsigned int whyEnd = [[[notification userInfo] objectForKey:@"NSTextMovement"] unsignedIntValue];
        NSView *nextView = nil;

        if (whyEnd == NSReturnTextMovement)
        {
            [[edit window] keyDown:(NSEvent*)231];
            nextView = edit;
        }
        else if (whyEnd == NSTabTextMovement)
        {
            nextView = [edit nextValidKeyView];
        }
        else if (whyEnd == NSBacktabTextMovement)
        {
            nextView = [edit previousValidKeyView];
        }

        if (nextView != nil)
            [[edit window] makeFirstResponder:nextView];
    }
}

/*---------------------------------------------------------------------------*/

@implementation OSXTextField

/*---------------------------------------------------------------------------*/

-(BOOL)becomeFirstResponder
{
    BOOL ret = [super becomeFirstResponder];
    OSX_becomeFirstResponder((OSXEdit*)self->parent, self);
    return ret;
}

/*---------------------------------------------------------------------------*/

- (BOOL)resignFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)textDidChange:(NSNotification*)notification
{
    unref(notification);
    OSX_textDidChange((OSXEdit*)self->parent, self);
}

/*---------------------------------------------------------------------------*/

- (void)textDidEndEditing:(NSNotification*)notification
{
    OSX_textDidEndEditing((OSXEdit*)self->parent, self, notification);
}

@end

/*---------------------------------------------------------------------------*/

@implementation OSXSecureTextField

/*---------------------------------------------------------------------------*/

-(BOOL)becomeFirstResponder
{
    BOOL ret = [super becomeFirstResponder];
    OSX_becomeFirstResponder((OSXEdit*)self->parent, self);
    return ret;
}

/*---------------------------------------------------------------------------*/

- (BOOL)resignFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)textDidChange:(NSNotification*)notification
{
    unref(notification);
    OSX_textDidChange((OSXEdit*)self->parent, self);
}

/*---------------------------------------------------------------------------*/

- (void)textDidEndEditing:(NSNotification*)notification
{
    OSX_textDidEndEditing((OSXEdit*)self->parent, self, notification);
}

@end

/*---------------------------------------------------------------------------*/

OSEdit *osedit_create(const uint32_t flags)
{
    OSXEdit *edit = nil;
    OSXTextField *field = nil;
    heap_auditor_add("OSXEdit");
    edit = [[OSXEdit alloc] initWithFrame:NSZeroRect];
    field = [[OSXTextField alloc] initWithFrame:NSZeroRect];
    edit->flags = flags;
    edit->OnFilter = NULL;
    edit->OnChange = NULL;
    edit->OnFocus = NULL;
    [edit addSubview:field];
    edit->field = field;
    field->parent = edit;
    _oscontrol_init(edit);
    _oscontrol_init_textattr(&edit->attrs);
    [edit->field setEditable:YES];
    [edit->field setSelectable:YES];
    [edit->field setBordered:YES];
    [edit->field setBezeled:YES];
    [edit->field setDrawsBackground:YES];
    [edit->field setStringValue:@""];
    [edit->field setAlignment:_oscontrol_text_alignment(ekLEFT)];
    _oscontrol_set_align(edit->field, &edit->attrs, ekLEFT);
    _oscontrol_set_font(edit->field,  &edit->attrs, edit->attrs.font);
    #if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [[edit->field cell] setUsesSingleLineMode:((flags & 1) == 1) ? NO : YES];
    #endif
    return (OSEdit*)edit;
}

/*---------------------------------------------------------------------------*/

void osedit_destroy(OSEdit **edit)
{
    OSXEdit *ledit = nil;
    cassert_no_null(edit);
    ledit = *((OSXEdit**)edit);
    cassert_no_null(ledit);
    listener_destroy(&ledit->OnFilter);
    listener_destroy(&ledit->OnChange);
    listener_destroy(&ledit->OnFocus);
    _oscontrol_remove_textattr(&ledit->attrs);
    [ledit release];
    *edit = NULL;
    heap_auditor_delete("OSXEdit");
}

/*---------------------------------------------------------------------------*/

void osedit_OnFilter(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([(NSObject*)edit isKindOfClass:[OSXEdit class]] == YES);
    listener_update(&((OSXEdit*)edit)->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnChange(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([(NSObject*)edit isKindOfClass:[OSXEdit class]] == YES);
    listener_update(&((OSXEdit*)edit)->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnFocus(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    cassert([(NSObject*)edit isKindOfClass:[OSXEdit class]] == YES);
    listener_update(&((OSXEdit*)edit)->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_text(OSEdit *edit, const char_t *text)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    _oscontrol_set_text(ledit->field, &ledit->attrs, text);
}

/*---------------------------------------------------------------------------*/

void osedit_tooltip(OSEdit *edit, const char_t *text)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    _oscontrol_tooltip_set(ledit->field, text);
}

/*---------------------------------------------------------------------------*/

void osedit_font(OSEdit *edit, const Font *font)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    _oscontrol_set_font(ledit->field, &ledit->attrs, font);
}

/*---------------------------------------------------------------------------*/

void osedit_align(OSEdit *edit, const align_t align)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    [ledit->field setAlignment:_oscontrol_text_alignment(align)];
    _oscontrol_set_align(ledit->field, &ledit->attrs, align);
}

/*---------------------------------------------------------------------------*/

void osedit_passmode(OSEdit *edit, const bool_t passmode)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    NSTextField *field = nil;
    cassert_no_null(ledit);
    if (passmode == TRUE)
    {
        if ([ledit->field isKindOfClass:[OSXTextField class]])
        {
            NSRect rect = [ledit->field frame];
            OSXSecureTextField *nfield = [[OSXSecureTextField alloc] initWithFrame:rect];
            nfield->parent = ledit;
            field = nfield;
        }
    }
    else
    {
        if ([ledit->field isKindOfClass:[OSXSecureTextField class]])
        {
            NSRect rect = [ledit->field frame];
            OSXTextField *nfield = [[OSXTextField alloc] initWithFrame:rect];
            nfield->parent = ledit;
            field = nfield;
        }
    }
    
    if (field != nil)
    {
        NSString *text = [[ledit->field cell] stringValue];
        [field setEditable:[ledit->field isEditable]];
        [field setSelectable:[ledit->field isSelectable]];
        [field setBordered:[ledit->field isBordered]];
        [field setBezeled:[ledit->field isBezeled]];
        [field setDrawsBackground:[ledit->field drawsBackground]];
        _oscontrol_set_font(field, &ledit->attrs, ledit->attrs.font);
        _oscontrol_set_text_color(field, &ledit->attrs, ledit->attrs.color);
        _oscontrol_set_align(field, &ledit->attrs, ledit->attrs.align);
        _oscontrol_set_text(field, &ledit->attrs, (const char_t*)[text UTF8String]);
        _oscontrol_detach_from_parent(ledit->field, ledit);        
		#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
        [[field cell] setUsesSingleLineMode:((ledit->flags & 1) == 1) ? NO : YES];
		#endif
        [ledit->field release];
        ledit->field = field;
        [ledit addSubview:field];
    }
}

/*---------------------------------------------------------------------------*/

void osedit_editable(OSEdit *edit, const bool_t is_editable)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    [ledit->field setEditable:is_editable == TRUE ? YES : NO];
}

/*---------------------------------------------------------------------------*/

void osedit_autoselect(OSEdit *edit, const bool_t autoselect)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    if (autoselect == TRUE)
        BIT_SET(ledit->flags, ekEDIT_AUTOSEL);
    else
        BIT_CLEAR(ledit->flags, ekEDIT_AUTOSEL);
}

/*---------------------------------------------------------------------------*/

void osedit_color(OSEdit *edit, const color_t color)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    _oscontrol_set_text_color(ledit->field, &ledit->attrs, color);
}

/*---------------------------------------------------------------------------*/

void osedit_bgcolor(OSEdit *edit, const color_t color)
{
    NSColor *nscolor = color != 0 ? oscolor_NSColor(color) : [NSColor textBackgroundColor];
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    [ledit->field setBackgroundColor:nscolor];
}

/*---------------------------------------------------------------------------*/

void osedit_bounds(const OSEdit *edit, const real32_t refwidth, const uint32_t lines, real32_t *width, real32_t *height)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    uint32_t padding = 0;
    cassert_no_null(ledit);
    cassert_no_null(width);
    cassert_no_null(height);

    _oscontrol_text_bounds(ledit->attrs.font, "OO", -1.f, width, height);
    padding = (uint32_t)((.3f * *height) + .5f);
    if (padding % 2 == 1)
        padding += 1;

    if (lines > 1)
    {
        register uint32_t i;
        char_t text[256] = "";
        cassert(edit_get_type(ledit->flags) == ekEDIT_MULTI);
        cassert(lines < 100);
        for (i = 0; i < lines - 1; ++i)
            str_cat_c(text, 256, "O\n");
        str_cat_c(text, 256, "O");
        _oscontrol_text_bounds(ledit->attrs.font, text, -1.f, width, height);
    }

    *width = refwidth;
    *height += (real32_t)padding;
}

/*---------------------------------------------------------------------------*/

void osedit_attach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_attach_control(panel, (NSView*)edit);
}

/*---------------------------------------------------------------------------*/

void osedit_detach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_detach_control(panel, (NSView*)edit);
}

/*---------------------------------------------------------------------------*/

void osedit_visible(OSEdit *edit, const bool_t is_visible)
{
    _oscontrol_set_visible((NSView*)edit, is_visible);
}

/*---------------------------------------------------------------------------*/

/* http://alienryderflex.com/hasFocus.html */
static bool_t i_has_focus(id control)
{
    NSWindow *window = [control window];
    id first = [window firstResponder];
    return (bool_t)([first isKindOfClass:[NSTextView class]]
    && [window fieldEditor:NO forObject:nil] != nil
    && (first == control || [first delegate] == control));
}

/*---------------------------------------------------------------------------*/

void osedit_enabled(OSEdit *edit, const bool_t is_enabled)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    if (is_enabled == FALSE)
    {
        if (i_has_focus(ledit->field) == TRUE)
            [[ledit->field window] endEditingFor:ledit->field];
    }

    _oscontrol_set_enabled(ledit->field, is_enabled);
    _oscontrol_set_text_color(ledit->field, &ledit->attrs, ledit->attrs.color);
}

/*---------------------------------------------------------------------------*/

void osedit_size(const OSEdit *edit, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((NSView*)edit, width, height);
}

/*---------------------------------------------------------------------------*/

void osedit_origin(const OSEdit *edit, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((NSView*)edit, x, y);
}

/*---------------------------------------------------------------------------*/

void osedit_frame(OSEdit *edit, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    _oscontrol_set_frame((NSView*)ledit, x, y, width, height);
    _oscontrol_set_frame((NSView*)ledit->field, 0, 0, width, height);
}

/*---------------------------------------------------------------------------*/

BOOL _osedit_is(NSView *view)
{
    return [view isKindOfClass:[OSXEdit class]];
}

/*---------------------------------------------------------------------------*/

void _osedit_detach_and_destroy(OSEdit **edit, OSPanel *panel)
{
    cassert_no_null(edit);
    osedit_detach(*edit, panel);
    osedit_destroy(edit);
}
