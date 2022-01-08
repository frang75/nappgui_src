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

@interface OSXEdit : NSTextField 
{
    @public
    NSTextFieldCell *cell;
    NSSecureTextFieldCell *scell;
    uint32_t flags;
    OSTextAttr attrs;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXEdit

/*---------------------------------------------------------------------------*/

-(BOOL)becomeFirstResponder
{
    BOOL ret = YES;
    NSText *text = nil;

    if ([self isEnabled] == YES && self->OnFocus != NULL)
    {
        bool_t params = TRUE;
        listener_event(self->OnFocus, ekEVFOCUS, (OSEdit*)self, &params, NULL, OSEdit, bool_t, void);
    }

    ret = [super becomeFirstResponder];
    text = [[self window] fieldEditor:YES forObject:self];

    if (BIT_TEST(self->flags, ekEDAUTOSEL) == TRUE)
    {
        [text selectAll:nil];
    }
    else
    {
        NSRange range = [text selectedRange];
        [text setSelectedRange:NSMakeRange(range.length, 0)];
    }
    
    return ret;
}

/*---------------------------------------------------------------------------*/

- (BOOL) resignFirstResponder
{
    return YES;
}

/*---------------------------------------------------------------------------*/

- (void)textDidChange:(NSNotification*)notification
{
    unref(notification);
    if ([self isEnabled] == YES && self->OnFilter != NULL)
    {
        EvText params;
        EvTextFilter result;
        NSText *text = NULL;
        params.text = (const char_t*)[[self stringValue] UTF8String];
        text = [[self window] fieldEditor:YES forObject:self];
        params.cpos = (uint32_t)[text selectedRange].location;
        result.apply = FALSE;
        result.text[0] = '\0';
        result.cpos = UINT32_MAX;
        listener_event(self->OnFilter, ekEVTXTFILTER, (OSEdit*)self, &params, &result, OSEdit, EvText, EvTextFilter);

        if (result.apply == TRUE)
            _oscontrol_set_text(self, &self->attrs, result.text);

        if (result.cpos != UINT32_MAX)
            [text setSelectedRange:NSMakeRange((NSUInteger)result.cpos, 0)];
        else
            [text setSelectedRange:NSMakeRange((NSUInteger)params.cpos, 0)];
    }
}

/*---------------------------------------------------------------------------*/

//- (void)controlTextDidBeginEditing:(NSNotification *)obj
//{
//    unref(obj);
//    if (BIT_TEST(self->flags, ekEDAUTOSEL) == TRUE)
//        [[self currentEditor] selectAll:nil];
//}

/*---------------------------------------------------------------------------*/

- (void)textDidEndEditing:(NSNotification*)notification
{
    unref(notification);
    if ([self isEnabled] == YES && self->OnChange != NULL
        && _oswindow_in_destroy([self window]) == NO)
    {
        EvText params;
        params.text = (const char_t*)[[self stringValue] UTF8String];
        listener_event(self->OnChange, ekEVTXTCHANGE, (OSEdit*)self, &params, NULL, OSEdit, EvText, void);
    }
    
    [[self window] endEditingFor:nil];

    if (self->OnFocus != NULL)
    {
        bool_t params = FALSE;
        listener_event(self->OnFocus, ekEVFOCUS, (OSEdit*)self, &params, NULL, OSEdit, bool_t, void);
    }

    {
        unsigned int whyEnd = [[[notification userInfo] objectForKey:@"NSTextMovement"] unsignedIntValue];
        NSView *nextView = nil;

        if (whyEnd == NSReturnTextMovement)
        {
            [[self window] keyDown:(NSEvent*)231];
            nextView = self;
        }        
        else if (whyEnd == NSTabTextMovement)
        {
            nextView = [self nextValidKeyView];
        }
        else if (whyEnd == NSBacktabTextMovement)
        {
            nextView = [self previousValidKeyView];
        }

        if (nextView != nil)
            [[self window] makeFirstResponder:nextView];
    }
}

@end

/*---------------------------------------------------------------------------*/

OSEdit *osedit_create(const edit_flag_t flags)
{
    OSXEdit *edit = nil;
    NSTextFieldCell *cell = nil;
    heap_auditor_add("OSXEdit");
    edit = [[OSXEdit alloc] initWithFrame:NSZeroRect];
    edit->flags = flags;
    edit->OnFilter = NULL;
    edit->OnChange = NULL;
    edit->OnFocus = NULL;
    _oscontrol_init(edit);
    _oscontrol_init_textattr(&edit->attrs);
    cell = [edit cell];
    [cell setEditable:YES];
    [cell setSelectable:YES];
    [cell setBordered:YES];
    [cell setBezeled:YES];
    [cell setDrawsBackground:YES];
    [cell setStringValue:@""];
    [cell setAlignment:_oscontrol_text_alignment(ekLEFT)];
    _oscontrol_set_align(edit, &edit->attrs, ekLEFT);
    _oscontrol_set_font(edit,  &edit->attrs, edit->attrs.font);
    edit->cell = [cell retain];
    edit->scell = [[NSSecureTextFieldCell alloc] init];
    [edit->scell setEchosBullets:YES];
    [edit->scell setEditable:YES];
    [edit->scell setSelectable:YES];
    [edit->scell setBordered:YES];
    [edit->scell setBezeled:YES];
    [edit->scell setDrawsBackground:YES];
    [edit->scell setAlignment:_oscontrol_text_alignment(ekLEFT)];
    #if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5
    [edit->cell setUsesSingleLineMode:((flags & 1) == 1) ? NO : YES];
    [edit->scell setUsesSingleLineMode:((flags & 1) == 1) ? NO : YES];
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
    [ledit->cell release];
    [ledit->scell release];
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
//#include "log.h"
void osedit_text(OSEdit *edit, const char_t *text)
{
/*    NSString *str = [(OSXEdit*)edit stringValue];
    if (str_equ_c([str UTF8String], text) == FALSE)*/
    // Check Crahs with string "ASUS STRIX RAID PRO Sound Card"
    // Position 74 (id = 75) of Products
    _oscontrol_set_text((OSXEdit*)edit, &((OSXEdit*)edit)->attrs, text);
    /*
    if (edit_type(((OSXEdit*)edit)->flags) == ekEDIT_MULTI_LINE)
        log_printf("Text: %s", text);

    bool_t ok = str_equ_c("ASUS STRIX RAID PRO Sound Card", text);
    */
//    _oscontrol_set_text((OSXEdit*)edit, &((OSXEdit*)edit)->attrs, ok ? "ASUS STRIX  RAID PRO Sound Card" : text);
//    _oscontrol_set_text((OSXEdit*)edit, &((OSXEdit*)edit)->attrs, text);
}

/*---------------------------------------------------------------------------*/

void osedit_tooltip(OSEdit *edit, const char_t *text)
{
    _oscontrol_tooltip_set((OSXEdit*)edit, text);    
}

/*---------------------------------------------------------------------------*/

void osedit_font(OSEdit *edit, const Font *font)
{
    _oscontrol_set_font((OSXEdit*)edit, &((OSXEdit*)edit)->attrs, font);
}

/*---------------------------------------------------------------------------*/

void osedit_align(OSEdit *edit, const align_t align)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    [ledit->cell setAlignment:_oscontrol_text_alignment(align)];
    [ledit->scell setAlignment:_oscontrol_text_alignment(align)];
    _oscontrol_set_align(ledit, &ledit->attrs, align);
}

/*---------------------------------------------------------------------------*/

void osedit_passmode(OSEdit *edit, const bool_t passmode)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    NSString *current_text = nil;
    cassert_no_null(ledit);
    if (passmode == TRUE)
    {
        if ([ledit cell] == ledit->cell)
        {
            [ledit->scell setEditable:[ledit->cell isEditable]];
            [ledit->scell setBackgroundColor:[ledit->cell backgroundColor]];
            current_text = [ledit->cell stringValue];
            [ledit setCell:ledit->scell];
        }
    }
    else
    {
        if ([ledit cell] == ledit->scell)
        {
            [ledit->cell setEditable:[ledit->scell isEditable]];
            [ledit->cell setBackgroundColor:[ledit->scell backgroundColor]];
            current_text = [ledit->scell stringValue];
            [ledit setCell:ledit->cell];
        }
    }
    
    /* Cell has changed */
    if (current_text != nil)
    {
        _oscontrol_set_font(ledit, &ledit->attrs, ledit->attrs.font);
        _oscontrol_set_text_color(ledit, &ledit->attrs, ledit->attrs.color);
        _oscontrol_set_align(ledit, &ledit->attrs, ledit->attrs.align);
        _oscontrol_set_text(ledit, &ledit->attrs, (const char_t*)[current_text UTF8String]);
    }
}

/*---------------------------------------------------------------------------*/

void osedit_editable(OSEdit *edit, const bool_t is_editable)
{
    cassert_no_null(edit);
    [(OSXEdit*)edit setEditable:is_editable == TRUE ? YES : NO];
}

/*---------------------------------------------------------------------------*/

void osedit_autoselect(OSEdit *edit, const bool_t autoselect)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    if (autoselect == TRUE)
        BIT_SET(ledit->flags, ekEDAUTOSEL);
    else
        BIT_CLEAR(ledit->flags, ekEDAUTOSEL);
}

/*---------------------------------------------------------------------------*/

void osedit_color(OSEdit *edit, const color_t color)
{
    _oscontrol_set_text_color((OSXEdit*)edit, &((OSXEdit*)edit)->attrs, color);
}

/*---------------------------------------------------------------------------*/

void osedit_bgcolor(OSEdit *edit, const color_t color)
{
    NSColor *nscolor = color != 0 ? oscolor_NSColor(color) : [NSColor textBackgroundColor];
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    [ledit->scell setBackgroundColor:nscolor];
    [ledit->cell setBackgroundColor:nscolor];
    [ledit setBackgroundColor:nscolor];
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
        cassert(edit_type(ledit->flags) == ekEDMULT);
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
static bool i_has_focus(id control)
{
    NSWindow *window = [control window];
    id first = [window firstResponder];
    return [first isKindOfClass:[NSTextView class]]
    && [window fieldEditor:NO forObject:nil] != nil
    && (first == control || [first delegate] == control);
}

/*---------------------------------------------------------------------------*/

void osedit_enabled(OSEdit *edit, const bool_t is_enabled)
{
    OSXEdit *ledit = (OSXEdit*)edit;
    cassert_no_null(ledit);
    if (is_enabled == FALSE)
    {
        if (i_has_focus(ledit) == YES)
            [[ledit window] endEditingFor:ledit];
    }

    _oscontrol_set_enabled(ledit, is_enabled);
    _oscontrol_set_text_color(ledit, &ledit->attrs, ledit->attrs.color);
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
    _oscontrol_set_frame((NSView*)edit, x, y, width, height);
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
