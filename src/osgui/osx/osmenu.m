/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu.m
 *
 */

/* Cocoa NSMenu wrapper */

#include "osgui_osx.inl"
#include "oswindow_osx.inl"
#include "../osmenu.h"
#include "../osgui.inl"
#include <sewer/cassert.h>
#include <core/heap.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

@interface OSXMenu : NSMenu
{
  @public
    void *non_used;
}
@end

/*---------------------------------------------------------------------------*/

@implementation OSXMenu

- (void)dealloc
{
    [super dealloc];
}

@end

/*---------------------------------------------------------------------------*/

OSMenu *osmenu_create(const enum_t flags)
{
    OSXMenu *menu = [[OSXMenu alloc] initWithTitle:[NSString string]];
    unref(flags);
    heap_auditor_add("OSXMenu");
    [menu setAutoenablesItems:NO];
    [menu setShowsStateColumn:YES];
    return cast(menu, OSMenu);
}

/*---------------------------------------------------------------------------*/

void osmenu_destroy(OSMenu **menu)
{
    OSXMenu *menup = nil;
    cassert_no_null(menu);
    menup = *dcast(menu, OSXMenu);
    cassert_no_null(menup);
    cassert([menup supermenu] == nil);
    cassert([NSApp mainMenu] != menup);
    heap_auditor_delete("OSXMenu");
    [menup release];
    *menu = NULL;
}

/*---------------------------------------------------------------------------*/

void osmenu_add_item(OSMenu *menu, OSMenuItem *menuitem)
{
    /* In BigSur, the retainCount after addItem is not +1
     * NSUInteger retain_count = 0;
     * NSUInteger retain_count2 = 0;
     */
    NSUInteger num_items = 0;
    OSXMenu *lmenu = cast(menu, OSXMenu);
    NSMenuItem *litem = cast(menuitem, NSMenuItem);
    cassert_no_null(lmenu);
    cassert_no_null(litem);
    cassert([cast(lmenu, NSObject) isKindOfClass:[OSXMenu class]] == YES);
    cassert([cast(litem, NSObject) isKindOfClass:[NSMenuItem class]] == YES);
    num_items = [[lmenu itemArray] count];
    cassert([litem menu] == nil);
    /* retain_count = [litem retainCount]; */
    [lmenu addItem:litem];
    /* retain_count2 = [litem retainCount]; */
    /* cassert([litem retainCount] == retain_count + 1); */
    cassert([litem menu] == lmenu);
    cassert_unref([[lmenu itemArray] count] == num_items + 1, num_items);
}

/*---------------------------------------------------------------------------*/

void osmenu_delete_item(OSMenu *menu, OSMenuItem *menuitem)
{
    NSUInteger num_items = 0;
    OSXMenu *lmenu = cast(menu, OSXMenu);
    NSMenuItem *litem = cast(menuitem, NSMenuItem);
    cassert_no_null(lmenu);
    cassert_no_null(litem);
    cassert([cast(lmenu, NSObject) isKindOfClass:[OSXMenu class]] == YES);
    num_items = [[lmenu itemArray] count];
    cassert(num_items > 0);
    cassert([litem menu] == lmenu);
    [lmenu removeItem:litem];
    cassert([litem menu] == nil);
    cassert_unref([[lmenu itemArray] count] == num_items - 1, num_items);
}

/*---------------------------------------------------------------------------*/

void osmenu_launch(OSMenu *menu, OSWindow *window, const real32_t x, const real32_t y)
{
    OSXMenu *lmenu = cast(menu, OSXMenu);
    NSView *view = nil;
    CGFloat ly = 0.f;
    cassert_no_null(lmenu);
    cassert([cast(lmenu, NSObject) isKindOfClass:[OSXMenu class]] == YES);
    /* TODO: Use view (convert from screen to view coordinate) */
    view = _oswindow_main_view(window);
    unref(view);
    ly = [[NSScreen mainScreen] frame].size.height - (CGFloat)y;
    [lmenu popUpMenuPositioningItem:nil atLocation:NSMakePoint((CGFloat)x, ly) inView:nil];
}

/*---------------------------------------------------------------------------*/

void osmenu_hide(OSMenu *menu)
{
    unref(menu);
    cassert(FALSE);
}
