/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: treest.h
 * https://www.nappgui.com/en/core/treest.html
 *
 */

/* Trees */

#define treest_create(type) \
    treest_##type##_create()

#define treest_copy(tree, func_copy, type) \
    treest_##type##_copy(tree, func_copy)

#define treest_read(stream, func_read, type) \
    treest_##type##_read(stream, func_read)

#define treest_read_ex(stream, func_read, data, type, dtype) \
    ((void)((data) == cast(data, dtype)), \
     FUNC_CHECK_READ_INIT_EX(func_read, type, dtype), \
     treest_##type##_read_ex(stream, (FPtr_read_init_ex)func_read, cast(data, void)))

#define treest_destroy(tree, func_remove, type) \
    treest_##type##_destroy(tree, func_remove)

#define treest_destopt(tree, func_remove, type) \
    treest_##type##_destopt(tree, func_remove)

#define treest_clear(tree, func_remove, type) \
    treest_##type##_clear(tree, func_remove)

#define treest_write(stream, tree, func_write, type) \
    treest_##type##_write(stream, tree, func_write)

#define treest_write_ex(stream, tree, func_write, data, type, dtype) \
    ((void)((data) == cast_const(data, dtype)), \
     FUNC_CHECK_WRITE_EX(func_write, type, dtype), \
     treest_##type##_write_ex(stream, tree, (FPtr_write_ex)func_write, cast_const(data, void)))

#define treest_esize(tree, type) \
    treest_##type##_esize(tree)

#define treest_mem(tree, type) \
    treest_##type##_mem(tree)

#define treest_root_get(tree, type) \
    treest_##type##_root_get(tree)

#define treest_root_get_const(tree, type) \
    treest_##type##_root_get_const(tree)

#define treest_root_new(tree, type) \
    treest_##type##_root_new(tree)

#define treest_dfs_first(tree, type) \
    treest_##type##_dfs_first(tree)

#define treest_dfs_first_const(tree, type) \
    treest_##type##_dfs_first_const(tree)

#define treest_dfs_last(tree, type) \
    treest_##type##_dfs_last(tree)

#define treest_dfs_last_const(tree, type) \
    treest_##type##_dfs_last_const(tree)

#define treest_next(tree, type) \
    treest_##type##_next(tree)

#define treest_next_const(tree, type) \
    treest_##type##_next_const(tree)

#define treest_prev(tree, type) \
    treest_##type##_prev(tree)

#define treest_prev_const(tree, type) \
    treest_##type##_prev_const(tree)

#define treest_dfs_stop(tree, type) \
    treest_##type##_dfs_stop(tree)

#define treest_node_size(node, type) \
    treest_##type##_node_size(node)

#define treest_node_depth(node, type) \
    treest_##type##_node_depth(node)

#define treest_node_index(node, type) \
    treest_##type##_node_index(node)

#define treest_node_parent(node, type) \
    treest_##type##_node_parent(node)

#define treest_node_parent_const(node, type) \
    treest_##type##_node_parent_const(node)

#define treest_node_get(node, pos, type) \
    treest_##type##_node_get(node, pos)

#define treest_node_get_const(node, pos, type) \
    treest_##type##_node_get_const(node, pos)

#define treest_node_insert(node, pos, type) \
    treest_##type##_node_insert(node, pos)

#define treest_node_delete(node, pos, func_remove, type) \
    treest_##type##_node_delete(node, pos, func_remove)

#define treest_node_data(node, type) \
    treest_##type##_node_data(node)

#define treest_node_data_const(node, type) \
    treest_##type##_node_data_const(node)

#define treest_foreach(elem, tree, type) \
    { \
        NodeSt(type) *elem##_node; \
        type *elem; \
        for (elem##_node = treest_##type##_dfs_first(tree); \
             elem##_node != NULL; \
             elem##_node = treest_##type##_next(tree)) \
        { \
            elem = treest_##type##_node_data(elem##_node);

#define treest_foreach_const(elem, tree, type) \
    { \
        const NodeSt(type) *elem##_node; \
        const type *elem; \
        for (elem##_node = treest_##type##_dfs_first_const(tree); \
             elem##_node != NULL; \
             elem##_node = treest_##type##_next_const(tree)) \
        { \
            elem = treest_##type##_node_data_const(elem##_node);

#define treest_forback(elem, tree, type) \
    { \
        NodeSt(type) *elem##_node; \
        type *elem; \
        for (elem##_node = treest_##type##_dfs_last(tree); \
             elem##_node != NULL; \
             elem##_node = treest_##type##_prev(tree)) \
        { \
            elem = treest_##type##_node_data(elem##_node);

#define treest_forback_const(elem, tree, type) \
    { \
        const NodeSt(type) *elem##_node; \
        const type *elem; \
        for (elem##_node = treest_##type##_dfs_last_const(tree); \
             elem##_node != NULL; \
             elem##_node = treest_##type##_prev_const(tree)) \
        { \
            elem = treest_##type##_node_data_const(elem##_node);

#define treest_end() \
    } \
    }
