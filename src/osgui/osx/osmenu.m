/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu.m
 *
 */

/* Operating system native menu */

#include "osmenu_osx.inl"
#include "osmenuitem_osx.inl"
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
    BOOL is_menubar;
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
    menu->is_menubar = FALSE;
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

static void i_force_submenu(OSXMenu *lmenu, NSMenuItem *litem)
{
    cassert_no_null(lmenu);
    cassert_no_null(litem);
    cassert_unref(lmenu->is_menubar == TRUE, lmenu);
    if ([litem submenu] == nil)
    {
        /* In macOS a menu item in menubar without submenu is not visible. */
        NSMenu *submenu = [[NSMenu alloc] initWithTitle:[litem title]];
        [litem setSubmenu:submenu];
        [submenu release];
    }
}

/*---------------------------------------------------------------------------*/

static void i_revert_forced_submenu(OSXMenu *lmenu, NSMenuItem *litem)
{
    cassert_no_null(lmenu);
    cassert_no_null(litem);
    cassert_unref(lmenu->is_menubar == TRUE, lmenu);
    /* Unset the submenu added by 'i_force_submenu' */
    if ([litem submenu] != nil && [[litem submenu] numberOfItems] == 0)
        [litem setSubmenu:nil];
}

/*---------------------------------------------------------------------------*/

void osmenu_insert_item(OSMenu *menu, const uint32_t pos, OSMenuItem *item)
{
    NSUInteger num_items = 0;
    OSXMenu *lmenu = cast(menu, OSXMenu);
    NSMenuItem *litem = cast(item, NSMenuItem);
    cassert_no_null(lmenu);
    cassert_no_null(litem);
    cassert([cast(lmenu, NSObject) isKindOfClass:[OSXMenu class]] == YES);
    cassert([cast(litem, NSObject) isKindOfClass:[NSMenuItem class]] == YES);
    num_items = [[lmenu itemArray] count];
    cassert([litem menu] == nil);
    [lmenu insertItem:litem atIndex:(NSInteger)pos];
    cassert([litem menu] == lmenu);
    cassert_unref([[lmenu itemArray] count] == num_items + 1, num_items);
    if (lmenu->is_menubar == TRUE)
        i_force_submenu(lmenu, litem);
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

/*---------------------------------------------------------------------------*/

bool_t osmenu_is_menubar(const OSMenu *menu)
{
    OSXMenu *lmenu = cast(menu, OSXMenu);
    cassert_no_null(lmenu);
    cassert([cast(lmenu, NSObject) isKindOfClass:[OSXMenu class]] == YES);
    return lmenu->is_menubar;
}

/*---------------------------------------------------------------------------*/

void _osmenu_set_menubar(OSMenu *menu)
{
    OSXMenu *lmenu = cast(menu, OSXMenu);
    NSInteger i = 0, n = 0;
    cassert_no_null(lmenu);
    cassert([cast(lmenu, NSObject) isKindOfClass:[OSXMenu class]] == YES);
    cassert(lmenu->is_menubar == FALSE);
    lmenu->is_menubar = TRUE;

    n = [lmenu numberOfItems];
    for (i = 0; i < n; ++i)
    {
        NSMenuItem *item = [lmenu itemAtIndex:i];
        if (item != nil && [item isSeparatorItem] == NO)
        {
            i_force_submenu(lmenu, item);
            /* In macOS icons in items of menubar are not allowed */
            if ([item image] != nil)
                [item setImage:nil];
        }
    }
}

/*---------------------------------------------------------------------------*/

void _osmenu_unset_menubar(OSMenu *menu)
{
    OSXMenu *lmenu = cast(menu, OSXMenu);
    NSInteger i = 0, n = 0;
    cassert_no_null(lmenu);
    cassert([cast(lmenu, NSObject) isKindOfClass:[OSXMenu class]] == YES);
    cassert(lmenu->is_menubar == TRUE);
    n = [lmenu numberOfItems];
    for (i = 0; i < n; ++i)
    {
        NSMenuItem *item = [lmenu itemAtIndex:i];
        if (item != nil && [item isSeparatorItem] == NO)
        {
            /* Restore the icon removed by '_osmenu_set_menubar' */
            NSImage *image = _osmenuitem_image(cast(item, OSMenuItem));
            [item setImage:image];
            i_revert_forced_submenu(lmenu, item);
        }
    }

    lmenu->is_menubar = FALSE;
}
