/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit_osx.inl
 *
 */

/* Operating System split view */

#include "osgui_osx.ixx"

__EXTERN_C

void _ossplit_create_tracks(void);

void _ossplit_destroy_tracks(void);

BOOL _ossplit_is(NSView *view);

__END_C
