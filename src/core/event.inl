/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: event.inl
 *
 */

/* Events */

#include "core.ixx"

__EXTERN_C

void _listener_retain(Listener *listener, FPtr_retain func_retain, FPtr_release func_release);

__END_C
