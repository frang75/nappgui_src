/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nrclib.c
 *
 */

/* Resource compiler logic */

#include "nrclib.h"
#include "resgen.inl"
#include <core/arrpt.h>
#include <core/date.h>
#include <core/hfile.h>
#include <core/stream.h>
#include <core/strings.h>
#include <osbs/log.h>
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

static bool_t i_is_valid_dest(const char_t *pathname)
{
    String *path = NULL;
    String *ext = NULL;
    bool_t ok = FALSE;
    str_split_pathext(pathname, &path, NULL, &ext);

    if (hfile_dir(tc(path)) && str_equ(ext, "c"))
        ok = TRUE;

    str_destroy(&path);
    str_destroy(&ext);
    return ok;
}

/*---------------------------------------------------------------------------*/

static bool_t i_is_update(const char_t *src_dir, const char_t *dest_path, const char_t *dest_file)
{
    String *path_c = str_cpath("%s/%s.c", dest_path, dest_file);
    String *path_h = str_cpath("%s/%s.h", dest_path, dest_file);
    Date date_c = hfile_date(tc(path_c), FALSE);
    Date date_h = hfile_date(tc(path_h), FALSE);
    bool_t is_update = FALSE;

    if (date_is_null(&date_c) == TRUE)
    {
        log_printf("'%s' does not exists.", tc(path_c));
        is_update = FALSE;
    }
    else if (date_is_null(&date_h) == TRUE)
    {
        log_printf("'%s' does not exists.", tc(path_h));
        is_update = FALSE;
    }
    else
    {
        Date date_res = hfile_date(src_dir, TRUE);
        if (date_is_null(&date_res) == TRUE)
        {
            log_printf("'%s' does not exists.", src_dir);
            is_update = FALSE;
        }

        ;
        {
            String *d_c = date_format(&date_c, "%d/%m/%Y-%H:%M:%S");
            String *d_res = date_format(&date_res, "%d/%m/%Y-%H:%M:%S");
            log_printf("Date C file: %s", tc(d_c));
            log_printf("Date resdir: %s", tc(d_res));
            str_destroy(&d_c);
            str_destroy(&d_res);
        }

        /* 'date_src' = More recent date of whole resource dir
           'date_src' more recent than 'date_c' --> Is NOT up to date */
        if (date_cmp(&date_res, &date_c) > 0)
            is_update = FALSE;
        else
            is_update = TRUE;
    }

    str_destroy(&path_c);
    str_destroy(&path_h);
    return is_update;
}

/*---------------------------------------------------------------------------*/

void nrclib_serial_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors, bool_t *regenerated)
{
    cassert_no_null(src_dir);
    cassert_no_null(dest_file);
    cassert_no_null(warnings);
    cassert_no_null(errors);
    cassert_no_null(regenerated);
    *warnings = arrpt_create(String);
    *errors = arrpt_create(String);
    *regenerated = TRUE;

    if (hfile_dir(src_dir) == TRUE && i_is_valid_dest(dest_file) == TRUE)
    {
        String *path = NULL;
        String *file = NULL;
        str_split_pathext(dest_file, &path, &file, NULL);

        if (i_is_update(src_dir, tc(path), tc(file)) == FALSE)
        {
            ResourcePack *pack = resgen_pack_read(src_dir, *warnings, *errors);
            resgen_write_h_file(pack, tc(path), tc(file), *errors);
            resgen_write_c_file(pack, tc(path), tc(file), *errors);
            resgen_destroy_pack(&pack);
            log_printf("Regenerating '%s'", src_dir);
        }
        else
        {
            log_printf("Is update '%s'", src_dir);
            *regenerated = FALSE;
        }

        str_destroy(&path);
        str_destroy(&file);
    }
    else
    {
        if (hfile_dir(src_dir) == FALSE)
        {
            String *error = str_printf("Invalid source directory '%s'.", src_dir);
            arrpt_append(*errors, error, String);
        }
        else
        {
            String *error = str_printf("Invalid destiny file '%s'.", dest_file);
            arrpt_append(*errors, error, String);
        }
    }
}

/*---------------------------------------------------------------------------*/

void nrclib_pack_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors, bool_t *regenerated)
{
    cassert_no_null(src_dir);
    cassert_no_null(dest_file);
    cassert_no_null(warnings);
    cassert_no_null(errors);
    cassert_no_null(regenerated);
    *warnings = arrpt_create(String);
    *errors = arrpt_create(String);
    *regenerated = TRUE;

    if (hfile_dir(src_dir) == TRUE && i_is_valid_dest(dest_file) == TRUE)
    {
        String *path = NULL;
        String *file = NULL;
        str_split_pathext(dest_file, &path, &file, NULL);

        if (i_is_update(src_dir, tc(path), tc(file)) == FALSE)
        {
            ResourcePack *pack = resgen_pack_read(src_dir, *warnings, *errors);
            resgen_write_h_file(pack, tc(path), tc(file), *errors);
            resgen_write_packed_file(pack, tc(path), tc(file), *errors);
            resgen_write_c_packed_file(pack, tc(path), tc(file), *errors);
            resgen_destroy_pack(&pack);
            log_printf("Regenerating '%s'", src_dir);
        }
        else
        {
            log_printf("Is update '%s'", src_dir);
            *regenerated = FALSE;
        }

        str_destroy(&path);
        str_destroy(&file);
    }
    else
    {
        if (hfile_dir(src_dir) == FALSE)
        {
            String *error = str_printf("Invalid source directory '%s'.", src_dir);
            arrpt_append(*errors, error, String);
        }
        else
        {
            String *error = str_printf("Invalid destiny file '%s'.", dest_file);
            arrpt_append(*errors, error, String);
        }
    }
}
