/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osgui_osx.m
 *
 */

/* Operating system native gui */

#include "osgui_osx.inl"
#include "osgui.inl"
#include "osglobals.inl"
#include "oscontrol.inl"
#include "oscomwin.inl"
#include "cassert.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

NSNumber *kUNDERLINE_STYLE_NONE = nil;
NSNumber *kUNDERLINE_STYLE_SINGLE = nil;
NSMutableParagraphStyle *kLEFT_PARAGRAPH_STYLE = nil;
NSMutableParagraphStyle *kCENTER_PARAGRAPH_STYLE = nil;
NSMutableParagraphStyle *kRIGHT_PARAGRAPH_STYLE = nil;
static NSMenu *kEMPTY_MENUBAR = nil;

/*---------------------------------------------------------------------------*/

void _osgui_start_imp(void)
{
    kUNDERLINE_STYLE_NONE = [[NSNumber alloc] initWithInt:NSUnderlineStyleNone];
    kUNDERLINE_STYLE_SINGLE = [[NSNumber alloc] initWithInt:NSUnderlineStyleSingle];
    kLEFT_PARAGRAPH_STYLE = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [kLEFT_PARAGRAPH_STYLE setAlignment:_oscontrol_text_alignment(ekLEFT)];
    kCENTER_PARAGRAPH_STYLE = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [kCENTER_PARAGRAPH_STYLE setAlignment:_oscontrol_text_alignment(ekCENTER)];
    kRIGHT_PARAGRAPH_STYLE = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [kRIGHT_PARAGRAPH_STYLE setAlignment:_oscontrol_text_alignment(ekRIGHT)];
    kEMPTY_MENUBAR = [[NSMenu alloc] initWithTitle:@""];
    osglobals_init();
}

/*---------------------------------------------------------------------------*/

void _osgui_finish_imp(void)
{
    [kLEFT_PARAGRAPH_STYLE release];
    [kCENTER_PARAGRAPH_STYLE release];
    [kRIGHT_PARAGRAPH_STYLE release];
    [kUNDERLINE_STYLE_SINGLE release];
    [kUNDERLINE_STYLE_NONE release];
    [kEMPTY_MENUBAR release];
    osglobals_finish();
    _oscomwin_destroy_globals();
}

/*---------------------------------------------------------------------------*/

void _osgui_word_size(StringSizeData *data, const char_t *word, real32_t *width, real32_t *height)
{
    NSString *str = nil;
    NSSize word_size;
    cassert_no_null(data);
    cassert_no_null(width);
    cassert_no_null(height);
    str = [NSString stringWithUTF8String:word];
    word_size = [str sizeWithAttributes:data->dict];
    *width = (real32_t)word_size.width;
    *height = (real32_t)word_size.height;
}

/*---------------------------------------------------------------------------*/

void _osgui_attach_menubar(OSWindow *window, OSMenu *menu)
{
    cassert_no_null(menu);
    cassert([NSApp mainMenu] == kEMPTY_MENUBAR || [NSApp mainMenu] == nil);
    unref(window);
    [NSApp setMainMenu:(NSMenu*)menu];
}

/*---------------------------------------------------------------------------*/

void _osgui_detach_menubar(OSWindow *window, OSMenu *menu)
{
    cassert_no_null(menu);
    cassert([NSApp mainMenu] == (NSMenu*)menu);
    unref(window);
    [NSApp setMainMenu:kEMPTY_MENUBAR];
}

/*---------------------------------------------------------------------------*/

void _osgui_change_menubar(OSWindow *window, OSMenu *previous_menu, OSMenu *new_menu)
{
    cassert_no_null(previous_menu);
    cassert_no_null(new_menu);
    cassert([NSApp mainMenu] == (NSMenu*)previous_menu);
    unref(window);
    [NSApp setMainMenu:(NSMenu*)new_menu];
}
