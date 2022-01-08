/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: hfile.inl
 *
 */

/* High-level file operations */

#include "core.hxx"

__EXTERN_C

String *hfile_exename(void);

String *hfile_home_dir(const char_t *path);

String *hfile_build_dir(const char_t *dir, const char_t *target);

String *hfile_src_dir(const char_t *file);

String *hfile_root_dir(const char_t *file);

__END_C
