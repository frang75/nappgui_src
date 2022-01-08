/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: respack.inl
 *
 */

/* Resource Packages */

#include "core.ixx"

__EXTERN_C

void *respack_object_imp(const ResPack *pack, const ResId id, FPtr_create_from_data func_create, FPtr_destroy func_destroy);

const char_t *respack_atext(const ArrPt(ResPack) *packs, const ResId id, bool_t *is_resid);

const byte_t *respack_afile(const ArrPt(ResPack) *packs, const ResId id, uint32_t *size, bool_t *is_resid);

void *respack_aobj_imp(const ArrPt(ResPack) *packs, const ResId id, FPtr_create_from_data func_create, FPtr_destroy func_destroy, bool_t *is_resid);


__END_C

#define respack_object(pack, id, func_create, func_destroy, type)\
    ((void)\
    FUNC_CHECK_CREATE_FROM_DATA(func_create, type),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
    (type*)respack_object_imp(pack, id, (FPtr_create_from_data)func_create, (FPtr_destroy)func_destroy))

#define respack_aobj(packs, id, func_create, func_destroy, is_resid, type)\
    ((void)\
    FUNC_CHECK_CREATE_FROM_DATA(func_create, type),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
    (type*)respack_aobj_imp(packs, id, (FPtr_create_from_data)func_create, (FPtr_destroy)func_destroy, is_resid))
    