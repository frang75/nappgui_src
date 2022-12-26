/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: hfileh.h
 *
 */

/* Undocumented (hidden) API about files and directories */

#include "coreh.hxx"

__EXTERN_C

_core_api String *hfile_exename(void);

_core_api String *hfile_build_dir(const char_t *dir, const char_t *target);

_core_api String *hfile_src_dir(const char_t *file);

_core_api String *hfile_root_dir(const char_t *file);

__END_C
