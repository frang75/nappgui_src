/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bnfparser.c
 *
 */

/* Parser generator from BNF */

#include "bnfparser.inl"
#include "cassert.h"
//#include "bnfgen.h"
//#include "ll1gen.h"
#include "hfile.h"
#include "strings.h"

//#include "nrclib.h"
//#include "msgparser.inl"
//#include "arrpt.h"
//#include "arrst.h"
//#include "bmem.h"
//#include "buffer.h"
//#include "event.h"
//#include "heap.h"
//#include "listener.h"
//#include "ptr.h"
//#include "stream.h"
//#include "unicode.h"

/*---------------------------------------------------------------------------*/

//static bool_t i_is_valid_dest(const char_t *pathname)
//{
//    String *path = NULL;
//    String *ext = NULL;
//    bool_t ok = FALSE;
//    str_split_pathext(pathname, &path, NULL, &ext);
//
//    if (hfile_dir(tc(path)) && str_equ(ext, "c"))
//        ok = TRUE;
//
//    str_destroy(&path);
//    str_destroy(&ext);
//    return ok;
//}

/*---------------------------------------------------------------------------*/

bool_t bnfparser_create(Stream *stm, const char_t *dest_file, String **error)
{
    unref(stm);
    unref(dest_file);
    unref(error);
    return FALSE;
}

//bool_t bnfparser_create(Stream *stm, const char_t *dest_file, String **error)
//{
//    if (i_is_valid_dest(dest_file))
//    {
//        LL1Gen *ll1gen = bnfgen_create(stm, error);
//        if (ll1gen)
//        {
//            ll1gen_destroy(&ll1gen);
//            return TRUE;
//        }
//    }
//    else
//    {
//        cassert_no_null(error);
//        *error = str_printf("Invalid destiny file '%s'.", dest_file);
//    }
//
//    return FALSE;
//}

