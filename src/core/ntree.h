/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: ntree.h
 *
 */

/* N-Ary tree structure */

#include "core.hxx"

__EXTERN_C

_core_api NTree *ntree_create(const uint16_t esize, const char_t *type);

_core_api NTree *ntree_copy(const NTree *tree, FPtr_scopy func_copy, const char_t *type);

_core_api NTree *ntree_copy_ptr(const NTree *tree, FPtr_copy func_copy, const char_t *type);

_core_api NTree *ntree_read(Stream *stream, const uint16_t esize, FPtr_read_init func_read_init, const char_t *type);

_core_api NTree *ntree_read_ex(Stream *stream, const uint16_t esize, FPtr_read_init_ex func_read_init, void *data, const char_t *type);

_core_api NTree *ntree_read_ptr(Stream *stream, FPtr_read func_read, const char_t *type);

_core_api NTree *ntree_read_ptr_ex(Stream *stream, FPtr_read_ex func_read, void *data, const char_t *type);

_core_api void ntree_destroy(NTree **tree, FPtr_remove func_remove, const char_t *type);

_core_api void ntree_destopt(NTree **tree, FPtr_remove func_remove, const char_t *type);

_core_api void ntree_destroy_ptr(NTree **tree, FPtr_destroy func_destroy, const char_t *type);

_core_api void ntree_destopt_ptr(NTree **tree, FPtr_destroy func_destroy, const char_t *type);

_core_api void ntree_clear(NTree *tree, FPtr_remove func_remove);

_core_api void ntree_clear_ptr(NTree *tree, FPtr_destroy func_destroy);

_core_api void ntree_write(Stream *stream, const NTree *tree, FPtr_write func_write);

_core_api void ntree_write_ex(Stream *stream, const NTree *tree, FPtr_write_ex func_write, const void *data);

_core_api void ntree_write_ptr(Stream *stream, const NTree *tree, FPtr_write func_write);

_core_api void ntree_write_ptr_ex(Stream *stream, const NTree *tree, FPtr_write_ex func_write, const void *data);

_core_api uint32_t ntree_esize(const NTree *tree);

_core_api uint32_t ntree_mem(const NTree *tree);

_core_api NNode *ntree_root_get(const NTree *tree);

_core_api NNode *ntree_root_new(NTree *tree);

_core_api NNode *ntree_dfs_first(NTree *tree);

_core_api NNode *ntree_dfs_last(NTree *tree);

_core_api NNode *ntree_next(NTree *tree);

_core_api NNode *ntree_prev(NTree *tree);

_core_api void ntree_dfs_stop(NTree *tree);

_core_api uint32_t ntree_node_size(const NNode *node);

_core_api uint32_t ntree_node_depth(const NNode *node);

_core_api uint32_t ntree_node_index(const NNode *node);

_core_api NNode *ntree_node_parent(const NNode *node);

_core_api NNode *ntree_node_get(const NNode *node, const uint32_t pos);

_core_api NNode *ntree_node_insert(NNode *node, const uint32_t pos);

_core_api byte_t *ntree_node_data(NNode *node);

_core_api byte_t *ntree_node_data0(NNode *node);

_core_api void ntree_node_delete(NNode *node, const uint32_t pos, FPtr_remove func_remove);

_core_api void ntree_node_delete_ptr(NNode *node, const uint32_t pos, FPtr_destroy func_destroy);

__END_C
