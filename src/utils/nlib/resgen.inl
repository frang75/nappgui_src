/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: resgen.inl
 *
 */

/* Application resource packs generation */

#include "nlib.ixx"

__EXTERN_C

ResourcePack *resgen_pack_read(const char_t *src_dir, ArrPt(String) *warnings, ArrPt(String) *errors);

void resgen_destroy_pack(ResourcePack **pack);

void resgen_write_h_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors);

void resgen_write_c_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors);

void resgen_write_packed_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors);

void resgen_write_c_packed_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors);

__END_C

