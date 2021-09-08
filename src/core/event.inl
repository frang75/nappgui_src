/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: event.inl
 *
 */

/* Events */

#include "core.ixx"

__EXTERN_C

Listener *listener_copy(const Listener *listener);

void listener_retain(Listener *listener, FPtr_retain func_retain, FPtr_release func_release);

__END_C
