/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: treept.h
 * https://www.nappgui.com/en/core/treept.html
 *
 */

/* Trees (pointer) */

#define treept_create(type) \
    treept_##type##_create()

#define treept_copy(tree, func_copy, type) \
    treept_##type##_copy(tree, func_copy)

#define treept_read(stream, func_read, type) \
    treept_##type##_read(stream, func_read)

#define treept_read_ex(stream, func_read, data, type, dtype) \
    ((void)((data) == cast_const(data, dtype)), \
     FUNC_CHECK_READ_EX(func_read, type, dtype), \
     treept_##type##_read_ex(stream, (FPtr_read_ex)func_read, cast(data, void)))

#define treept_destroy(tree, func_destroy, type) \
    treept_##type##_destroy(tree, func_destroy)

#define treept_destopt(tree, func_destroy, type) \
    treept_##type##_destopt(tree, func_destroy)

#define treept_clear(tree, func_destroy, type) \
    treept_##type##_clear(tree, func_destroy)

#define treept_write(stream, tree, func_write, type) \
    treept_##type##_write(stream, tree, func_write)

#define treept_write_ex(stream, tree, func_write, data, type, dtype) \
    ((void)((data) == cast_const(data, dtype)), \
     FUNC_CHECK_WRITE_EX(func_write, type, dtype), \
     treept_##type##_write_ex(stream, tree, (FPtr_write_ex)func_write, cast_const(data, void)))

#define treept_esize(tree, type) \
    treept_##type##_esize(tree)

#define treept_mem(tree, type) \
    treept_##type##_mem(tree)

#define treept_root_get(tree, type) \
    treept_##type##_root_get(tree)

#define treept_root_get_const(tree, type) \
    treept_##type##_root_get_const(tree)

#define treept_root_new(tree, ptr, type) \
    treept_##type##_root_new(tree, ptr)

#define treept_dfs_first(tree, type) \
    treept_##type##_dfs_first(tree)

#define treept_dfs_first_const(tree, type) \
    treept_##type##_dfs_first_const(tree)

#define treept_dfs_last(tree, type) \
    treept_##type##_dfs_last(tree)

#define treept_dfs_last_const(tree, type) \
    treept_##type##_dfs_last_const(tree)

#define treept_next(tree, type) \
    treept_##type##_next(tree)

#define treept_next_const(tree, type) \
    treept_##type##_next_const(tree)

#define treept_prev(tree, type) \
    treept_##type##_prev(tree)

#define treept_prev_const(tree, type) \
    treept_##type##_prev_const(tree)

#define treept_dfs_stop(tree, type) \
    treept_##type##_dfs_stop(tree)

#define treept_node_size(node, type) \
    treept_##type##_node_size(node)

#define treept_node_depth(node, type) \
    treept_##type##_node_depth(node)

#define treept_node_index(node, type) \
    treept_##type##_node_index(node)

#define treept_node_parent(node, type) \
    treept_##type##_node_parent(node)

#define treept_node_parent_const(node, type) \
    treept_##type##_node_parent_const(node)

#define treept_node_get(node, pos, type) \
    treept_##type##_node_get(node, pos)

#define treept_node_get_const(node, pos, type) \
    treept_##type##_node_get_const(node, pos)

#define treept_node_insert(node, pos, ptr, type) \
    treept_##type##_node_insert(node, pos, ptr)

#define treept_node_delete(node, pos, func_destroy, type) \
    treept_##type##_node_delete(node, pos, func_destroy)

#define treept_node_data(node, type) \
    treept_##type##_node_data(node)

#define treept_node_data_const(node, type) \
    treept_##type##_node_data_const(node)

#define treept_foreach(elem, tree, type) \
    { \
        NodePt(type) *elem##_node; \
        type *elem; \
        for (elem##_node = treept_##type##_dfs_first(tree); \
             elem##_node != NULL; \
             elem##_node = treept_##type##_next(tree)) \
        { \
            elem = treept_##type##_node_data(elem##_node);

#define treept_foreach_const(elem, tree, type) \
    { \
        const NodePt(type) *elem##_node; \
        const type *elem; \
        for (elem##_node = treept_##type##_dfs_first_const(tree); \
             elem##_node != NULL; \
             elem##_node = treept_##type##_next_const(tree)) \
        { \
            elem = treept_##type##_node_data_const(elem##_node);

#define treept_forback(elem, tree, type) \
    { \
        NodePt(type) *elem##_node; \
        type *elem; \
        for (elem##_node = treept_##type##_dfs_last(tree); \
             elem##_node != NULL; \
             elem##_node = treept_##type##_prev(tree)) \
        { \
            elem = treept_##type##_node_data(elem##_node);

#define treept_forback_const(elem, tree, type) \
    { \
        const NodePt(type) *elem##_node; \
        const type *elem; \
        for (elem##_node = treept_##type##_dfs_last_const(tree); \
             elem##_node != NULL; \
             elem##_node = treept_##type##_prev_const(tree)) \
        { \
            elem = treept_##type##_node_data_const(elem##_node);

#define treept_end() \
    } \
    }
