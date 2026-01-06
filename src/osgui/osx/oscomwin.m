/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscomwin.m
 *
 */

/* Operating System native common windows */

#include "oscontrol_osx.inl"
#include "oscomwin.inl"
#include "../oscomwin.h"
#include <core/event.h>
#include <core/strings.h>
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

#if defined(MAC_OS_VERSION_12_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_12_0

#include <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

/*---------------------------------------------------------------------------*/

static void i_set_ftypes(NSSavePanel *panel, const char_t **ftypes, const uint32_t size)
{
    NSMutableArray< UTType * > *array = [NSMutableArray array];

    if (ftypes != NULL && size > 0)
    {
        uint32_t i;
        for (i = 0; i < size; ++i)
        {
            NSString *ext = [NSString stringWithUTF8String:ftypes[i]];
            UTType *type = [UTType typeWithFilenameExtension:ext];
            if (type != nil)
                [array addObject:type];
        }
    }

    [panel setAllowedContentTypes:array];
}

/*---------------------------------------------------------------------------*/

#else

static void i_set_ftypes(NSSavePanel *panel, const char_t **ftypes, const uint32_t size)
{
    if (ftypes != NULL && size > 0)
    {
        uint32_t i;
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:(NSUInteger)size];
        for (i = 0; i < size; ++i)
        {
            NSString *str = [NSString stringWithUTF8String:(const char *)ftypes[i]];
            [array addObject:str];
        }

        [panel setAllowedFileTypes:array];
    }
    else
    {
        [panel setAllowedFileTypes:nil];
    }
}

#endif

/*---------------------------------------------------------------------------*/

static void i_panel_info(NSSavePanel *panel, const char_t *caption, const char_t *start_dir, const char_t *filename)
{
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
    if (str_empty_c(caption) == FALSE)
    {
        NSString *str = [[NSString alloc] initWithUTF8String:caption];
        [panel setTitle:str];
        [str release];
    }

    if (str_empty_c(start_dir) == FALSE)
    {
        NSString *str = [[NSString alloc] initWithUTF8String:start_dir];
        NSURL *url = [[NSURL alloc] initWithString:str];
        [panel setDirectoryURL:url];
        [url release];
        [str release];
    }
    else
    {
        [panel setDirectoryURL:nil];
    }

    if (str_empty_c(filename) == FALSE)
    {
        NSString *str = [[NSString alloc] initWithUTF8String:filename];
        [panel setNameFieldStringValue:str];
        [str release];
    }

#else
    unref(caption);
    unref(start_dir);
    unref(filename);
#endif
}

/*---------------------------------------------------------------------------*/

static NSOpenPanel *i_open_file(const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir, const char_t *filename)
{
    NSOpenPanel *open_panel = [NSOpenPanel openPanel];
    BOOL dirsel = NO;

    i_panel_info(open_panel, caption, start_dir, filename);
    [open_panel setAllowsMultipleSelection:FALSE];
    if (ftypes != NULL)
    {
        cassert(size > 0);
        if (size == 1 && strcmp(cast_const(ftypes[0], char), "..DIR..") == 0)
            dirsel = YES;
    }

    i_set_ftypes(open_panel, ftypes, size);
    [open_panel setCanChooseFiles:!dirsel];
    [open_panel setCanChooseDirectories:dirsel];
    return open_panel;
}

/*---------------------------------------------------------------------------*/

static NSSavePanel *i_save_file(const char_t **ftypes, const uint32_t size, const char_t *caption, const char_t *start_dir, const char_t *filename)
{
    NSSavePanel *save_panel = [NSSavePanel savePanel];
    i_panel_info(save_panel, caption, start_dir, filename);
    [save_panel setCanCreateDirectories:YES];
    i_set_ftypes(save_panel, ftypes, size);
    return save_panel;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_open_file_selected(NSOpenPanel *open_panel)
{
    NSArray *urls;
    NSURL *url;
    cassert_no_null(open_panel);
    urls = [open_panel URLs];
    cassert([urls count] == 1);
    url = [urls objectAtIndex:0];
    cassert_no_null(url);
    return cast_const([[url path] UTF8String], char_t);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_save_file_selected(NSSavePanel *save_panel)
{
    NSURL *url;
    cassert_no_null(save_panel);
    url = [save_panel URL];
    cassert_no_null(url);
    return cast_const([[url path] UTF8String], char_t);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_oscomwin_file(OSWindow *parent, const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir, const char_t *filename, const bool_t foropen)
{
    unref(parent);

    if (foropen == TRUE)
    {
        NSOpenPanel *open_panel = i_open_file(caption, ftypes, size, start_dir, filename);

        if (str_empty_c(caption) == FALSE)
            [open_panel setTitle:[NSString stringWithUTF8String:caption]];

#if defined(MAC_OS_X_VERSION_10_9) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_9
        {
            NSModalResponse ret = [open_panel runModal];
            if (ret == NSModalResponseOK)
                return i_open_file_selected(open_panel);
            else
                return NULL;
        }
#else
        {
            NSUInteger ret = (NSUInteger)[open_panel runModal];
            if (ret == NSFileHandlingPanelOKButton)
                return i_open_file_selected(open_panel);
            else
                return NULL;
        }
#endif
    }
    else
    {
        NSSavePanel *save_panel = i_save_file(ftypes, size, caption, start_dir, filename);

        if (str_empty_c(caption) == FALSE)
            [save_panel setTitle:[NSString stringWithUTF8String:caption]];

#if defined(MAC_OS_X_VERSION_10_9) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_9
        {
            NSModalResponse ret = [save_panel runModal];
            if (ret == NSModalResponseOK)
                return i_save_file_selected(save_panel);
            else
                return NULL;
        }
#else
        {
            NSUInteger ret = (NSUInteger)[save_panel runModal];
            if (ret == NSFileHandlingPanelOKButton)
                return i_save_file_selected(save_panel);
            else
                return NULL;
        }
#endif
    }
}

/*---------------------------------------------------------------------------*/

const char_t *oscomwin_dir(OSWindow *parent, const char_t *caption, const char_t *start_dir)
{
    const char_t *ftypes[] = {"..DIR.."};
    return i_oscomwin_file(parent, caption, ftypes, 1, start_dir, NULL, TRUE);
}

/*---------------------------------------------------------------------------*/

const char_t *oscomwin_file(OSWindow *parent, const char_t *caption, const char_t **ftypes, const uint32_t size, const char_t *start_dir, const char_t *filename, const bool_t foropen)
{
    return i_oscomwin_file(parent, caption, ftypes, size, start_dir, filename, foropen);
}

/*---------------------------------------------------------------------------*/

@interface NSColorChoose : NSObject
{
  @public
    Listener *OnChange;
}
@end

@implementation NSColorChoose

/*---------------------------------------------------------------------------*/

- (IBAction)onColorChange:(id)sender
{
    NSColor *color = [sender color];
    color_t c = _oscontrol_from_NSColor(color);
    listener_event(self->OnChange, ekGUI_EVENT_COLOR, NULL, &c, NULL, void, color_t, void);
}

/*---------------------------------------------------------------------------*/

- (void)dealloc
{
    listener_destroy(&self->OnChange);
    [super dealloc];
}

@end

static NSColorChoose *i_COLOR_CHOOSE = nil;

/*---------------------------------------------------------------------------*/

void oscomwin_color(OSWindow *parent, const char_t *caption, const real32_t x, const real32_t y, const align_t halign, const align_t valign, const color_t current, color_t *colors, const uint32_t n, Listener *OnChange)
{
    NSColorPanel *panel = [NSColorPanel sharedColorPanel];
    unref(colors);
    unref(n);

    if (str_empty_c(caption) == FALSE)
        [panel setTitle:[NSString stringWithUTF8String:caption]];

    if (i_COLOR_CHOOSE != nil)
    {
        [i_COLOR_CHOOSE release];
        i_COLOR_CHOOSE = nil;
    }

    i_COLOR_CHOOSE = [NSColorChoose alloc];
    i_COLOR_CHOOSE->OnChange = OnChange;
    [panel setTarget:i_COLOR_CHOOSE];
    [panel setAction:@selector(onColorChange:)];

    /*[NSColorPanel setPickerMode:NSColorPanelModeRGB];*/
    [panel setColor:_oscontrol_color(current)];
    [panel setShowsAlpha:YES];

    if ([panel isVisible] == NO)
    {
        NSPoint origin = NSMakePoint((CGFloat)x, (CGFloat)y);
        NSSize size = [panel frame].size;
        CGFloat sh = [[NSScreen mainScreen] frame].size.height;
        if (halign != ekLEFT || valign != ekTOP)
        {
            switch (halign)
            {
            case ekLEFT:
            case ekJUSTIFY:
                break;
            case ekCENTER:
                origin.x -= size.width / 2;
                break;
            case ekRIGHT:
                origin.x -= size.width;
                break;
            default:
                cassert_default(halign);
            }

            switch (valign)
            {
            case ekTOP:
            case ekJUSTIFY:
                break;
            case ekCENTER:
                origin.x -= size.height / 2;
                break;
            case ekBOTTOM:
                origin.x -= size.height;
                break;
            default:
                cassert_default(valign);
            }
        }

        origin.y = sh - origin.y - size.height;
        [panel setFrameOrigin:origin];
        [panel makeKeyAndOrderFront:(NSWindow *)parent];
    }
}

/*---------------------------------------------------------------------------*/

void _oscomwin_destroy_globals(void)
{
    if (i_COLOR_CHOOSE != nil)
    {
        [i_COLOR_CHOOSE release];
        i_COLOR_CHOOSE = nil;
    }
}
