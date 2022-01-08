/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nrclib.c
 *
 */

/* Resource compiler logic */

#include "nrclib.h"
#include "resgen.inl"
#include "bnfparser.inl"
#include "arrpt.h"
#include "cassert.h"
#include "date.h"
#include "hfile.h"
#include "log.h"
#include "stream.h"
#include "strings.h"

/*---------------------------------------------------------------------------*/

//Stream *nrclib_serial_file(const char_t *pathname, const char_t *variable_name);
//Stream *nrclib_serial_file(const char_t *pathname, const char_t *variable_name)
//{   
//    bool_t opened;
//    byte_t *file_data = NULL;
//    uint64_t file_size;
//    Stream *code = NULL;
//    opened = filedir_load_file(pathname, &file_data, &file_size, NULL);
//    cassert(opened == TRUE);
//    code = i_binary_to_ascii(file_data, (uint32_t)file_size, 50, variable_name);
//    heap_deletes(&file_data, file_size, byte_t);
//    return code;
//}

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
    Date date = kDATE_NULL;

    {
        String *path = str_cpath("%s/%s.c", dest_path, dest_file);
        date = hfile_date(tc(path), FALSE);
        str_destroy(&path);
    }

    if (date_is_null(&date) == FALSE)
    {
        String *path = str_cpath("%s/%s.h", dest_path, dest_file);
        Date date2 = hfile_date(tc(path), FALSE);

        if (date_is_null(&date2) || date_cmp(&date2, &date) > 0)
            date = date2;

        str_destroy(&path);
    }
    else
    {
        return FALSE;
    }

    if (date_is_null(&date) == FALSE)
    {
        // 'date' = More recent date of 'respack.c' and 'respack.h'
        // 'date2' = More recent date of whole resource dir
        // 'date2' more recent than 'date' --> Is NOT up to date
        Date date2 = hfile_date(src_dir, TRUE);
        if (date_is_null(&date2) || date_cmp(&date2, &date) > 0)
            return FALSE;
        else
            return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

void nrclib_serial_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors)
{
    cassert_no_null(src_dir);
    cassert_no_null(dest_file);
    cassert_no_null(warnings);
    cassert_no_null(errors);
    *warnings = arrpt_create(String);
    *errors = arrpt_create(String);
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

void nrclib_pack_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors)
{
    cassert_no_null(src_dir);
    cassert_no_null(dest_file);
    cassert_no_null(warnings);
    cassert_no_null(errors);
    *warnings = arrpt_create(String);
    *errors = arrpt_create(String);
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

bool_t nrclib_bnfparser(const char_t *src_file, const char_t *dest_file, String **error)
{
    Stream *stm = stm_from_file(src_file, NULL);
    if (stm)
    {
        bool_t ok = bnfparser_create(stm, dest_file, error);
        stm_close(&stm);
        return ok;
    }
    else
    {
        cassert_no_null(error);
        *error = str_printf("Error opening '%s' file.", src_file);
        return FALSE;
    }
}
