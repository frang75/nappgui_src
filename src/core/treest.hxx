/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: treest.hxx
 * https://www.nappgui.com/en/core/treest.html
 *
 */

/* Tree macros for type checking at compile time */

#if defined __ASSERTS__
#define TreeStDebug(type) \
    struct Node##St##type \
    { \
        NTree *tree; \
        struct Node##St##type *parent; \
        Array *children; \
        type data; \
    }; \
\
    struct Tree##St##type \
    { \
        uint16_t esize; \
        bool_t traversing; \
        struct Node##St##type *root; \
        struct Node##St##type *current; \
    }
#else
#define TreeStDebug(type) \
    typedef struct _treestnonused##type##_t treestnonused##type
#endif

#define TreeStFuncs(type) \
    TreeSt(type); \
    NodeSt(type); \
\
    static __TYPECHECK TreeSt(type) *treest_##type##_create(void); \
    static TreeSt(type) *treest_##type##_create(void) \
    { \
        return cast(ntree_create((uint16_t)sizeof(type), cast_const(TREEST #type, char_t)), TreeSt(type)); \
    } \
\
    static __TYPECHECK TreeSt(type) *treest_##type##_copy(const TreeSt(type) *tree, void(func_copy)(type *, const type *)); \
    static TreeSt(type) *treest_##type##_copy(const TreeSt(type) *tree, void(func_copy)(type *, const type *)) \
    { \
        return cast(ntree_copy(cast_const(tree, NTree), (FPtr_scopy)func_copy, cast_const(TREEST #type, char_t)), TreeSt(type)); \
    } \
\
    static __TYPECHECK TreeSt(type) *treest_##type##_read(Stream *stream, void(func_read)(Stream *, type *)); \
    static TreeSt(type) *treest_##type##_read(Stream *stream, void(func_read)(Stream *, type *)) \
    { \
        return cast(ntree_read(stream, (uint16_t)sizeof(type), (FPtr_read_init)func_read, cast_const(TREEST #type, char_t)), TreeSt(type)); \
    } \
\
    static __TYPECHECK void treest_##type##_destroy(struct Tree##St##type **tree, void(func_remove)(type *)); \
    static void treest_##type##_destroy(struct Tree##St##type **tree, void(func_remove)(type *)) \
    { \
        ntree_destroy(dcast(tree, NTree), (FPtr_remove)func_remove, cast_const(TREEST #type, char_t)); \
    } \
\
    static __TYPECHECK void treest_##type##_destopt(struct Tree##St##type **tree, void(func_remove)(type *)); \
    static void treest_##type##_destopt(struct Tree##St##type **tree, void(func_remove)(type *)) \
    { \
        ntree_destopt(dcast(tree, NTree), (FPtr_remove)func_remove, cast_const(TREEST #type, char_t)); \
    } \
\
    static __TYPECHECK void treest_##type##_clear(TreeSt(type) *tree, void(func_remove)(type *)); \
    static void treest_##type##_clear(TreeSt(type) *tree, void(func_remove)(type *)) \
    { \
        ntree_clear(cast(tree, NTree), (FPtr_remove)func_remove); \
    } \
\
    static __TYPECHECK void treest_##type##_write(Stream *stream, const TreeSt(type) *tree, void(func_write)(Stream *, const type *)); \
    static void treest_##type##_write(Stream *stream, const TreeSt(type) *tree, void(func_write)(Stream *, const type *)) \
    { \
        ntree_write(stream, cast_const(tree, NTree), (FPtr_write)func_write); \
    } \
\
    static __TYPECHECK uint32_t treest_##type##_esize(const TreeSt(type) *tree); \
    static uint32_t treest_##type##_esize(const TreeSt(type) *tree) \
    { \
        return ntree_esize(cast_const(tree, NTree)); \
    } \
\
    static __TYPECHECK uint32_t treest_##type##_mem(const TreeSt(type) *tree); \
    static uint32_t treest_##type##_mem(const TreeSt(type) *tree) \
    { \
        return ntree_mem(cast_const(tree, NTree)); \
    } \
\
    static __TYPECHECK TreeSt(type) *treest_##type##_read_ex(Stream *stream, FPtr_read_init_ex func_read, void *data); \
    static TreeSt(type) *treest_##type##_read_ex(Stream *stream, FPtr_read_init_ex func_read, void *data) \
    { \
        return cast(ntree_read_ex(stream, (uint16_t)sizeof(type), func_read, data, cast_const(TREEST #type, char_t)), TreeSt(type)); \
    } \
\
    static __TYPECHECK void treest_##type##_write_ex(Stream *stream, const TreeSt(type) *tree, FPtr_write_ex func_write, const void *data); \
    static void treest_##type##_write_ex(Stream *stream, const TreeSt(type) *tree, FPtr_write_ex func_write, const void *data) \
    { \
        ntree_write_ex(stream, cast_const(tree, NTree), func_write, data); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_root_get(TreeSt(type) *tree); \
    static NodeSt(type) *treest_##type##_root_get(TreeSt(type) *tree) \
    { \
        return cast(ntree_root_get(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK const NodeSt(type) *treest_##type##_root_get_const(const TreeSt(type) *tree); \
    static const NodeSt(type) *treest_##type##_root_get_const(const TreeSt(type) *tree) \
    { \
        return cast_const(ntree_root_get(cast_const(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_root_new(TreeSt(type) *tree); \
    static NodeSt(type) *treest_##type##_root_new(TreeSt(type) *tree) \
    { \
        return cast(ntree_root_new(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_dfs_first(TreeSt(type) *tree); \
    static NodeSt(type) *treest_##type##_dfs_first(TreeSt(type) *tree) \
    { \
        return cast(ntree_dfs_first(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK const NodeSt(type) *treest_##type##_dfs_first_const(const TreeSt(type) *tree); \
    static const NodeSt(type) *treest_##type##_dfs_first_const(const TreeSt(type) *tree) \
    { \
        return cast_const(ntree_dfs_first(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_dfs_last(TreeSt(type) *tree); \
    static NodeSt(type) *treest_##type##_dfs_last(TreeSt(type) *tree) \
    { \
        return cast(ntree_dfs_last(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK const NodeSt(type) *treest_##type##_dfs_last_const(const TreeSt(type) *tree); \
    static const NodeSt(type) *treest_##type##_dfs_last_const(const TreeSt(type) *tree) \
    { \
        return cast_const(ntree_dfs_last(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_next(TreeSt(type) *tree); \
    static NodeSt(type) *treest_##type##_next(TreeSt(type) *tree) \
    { \
        return cast(ntree_next(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK const NodeSt(type) *treest_##type##_next_const(const TreeSt(type) *tree); \
    static const NodeSt(type) *treest_##type##_next_const(const TreeSt(type) *tree) \
    { \
        return cast_const(ntree_next(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_prev(TreeSt(type) *tree); \
    static NodeSt(type) *treest_##type##_prev(TreeSt(type) *tree) \
    { \
        return cast(ntree_prev(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK const NodeSt(type) *treest_##type##_prev_const(const TreeSt(type) *tree); \
    static const NodeSt(type) *treest_##type##_prev_const(const TreeSt(type) *tree) \
    { \
        return cast_const(ntree_prev(cast(tree, NTree)), NodeSt(type)); \
    } \
\
    static __TYPECHECK void treest_##type##_dfs_stop(TreeSt(type) *tree); \
    static void treest_##type##_dfs_stop(TreeSt(type) *tree) \
    { \
        ntree_dfs_stop(cast(tree, NTree)); \
    } \
\
    static __TYPECHECK uint32_t treest_##type##_node_size(const NodeSt(type) *node); \
    static uint32_t treest_##type##_node_size(const NodeSt(type) *node) \
    { \
        return ntree_node_size(cast_const(node, NNode)); \
    } \
\
    static __TYPECHECK uint32_t treest_##type##_node_depth(const NodeSt(type) *node); \
    static uint32_t treest_##type##_node_depth(const NodeSt(type) *node) \
    { \
        return ntree_node_depth(cast_const(node, NNode)); \
    } \
\
    static __TYPECHECK uint32_t treest_##type##_node_index(const NodeSt(type) *node); \
    static uint32_t treest_##type##_node_index(const NodeSt(type) *node) \
    { \
        return ntree_node_index(cast_const(node, NNode)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_node_parent(NodeSt(type) *node); \
    static NodeSt(type) *treest_##type##_node_parent(NodeSt(type) *node) \
    { \
        return cast(ntree_node_parent(cast(node, NNode)), NodeSt(type)); \
    } \
\
    static __TYPECHECK const NodeSt(type) *treest_##type##_node_parent_const(const NodeSt(type) *node); \
    static const NodeSt(type) *treest_##type##_node_parent_const(const NodeSt(type) *node) \
    { \
        return cast_const(ntree_node_parent(cast_const(node, NNode)), NodeSt(type)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_node_get(NodeSt(type) *node, const uint32_t pos); \
    static NodeSt(type) *treest_##type##_node_get(NodeSt(type) *node, const uint32_t pos) \
    { \
        return cast(ntree_node_get(cast(node, NNode), pos), NodeSt(type)); \
    } \
\
    static __TYPECHECK const NodeSt(type) *treest_##type##_node_get_const(const NodeSt(type) *node, const uint32_t pos); \
    static const NodeSt(type) *treest_##type##_node_get_const(const NodeSt(type) *node, const uint32_t pos) \
    { \
        return cast_const(ntree_node_get(cast_const(node, NNode), pos), NodeSt(type)); \
    } \
\
    static __TYPECHECK NodeSt(type) *treest_##type##_node_insert(NodeSt(type) *node, const uint32_t pos); \
    static NodeSt(type) *treest_##type##_node_insert(NodeSt(type) *node, const uint32_t pos) \
    { \
        return cast(ntree_node_insert(cast(node, NNode), pos), NodeSt(type)); \
    } \
\
    static __TYPECHECK void treest_##type##_node_delete(NodeSt(type) *node, const uint32_t pos, void(func_remove)(type *)); \
    static void treest_##type##_node_delete(NodeSt(type) *node, const uint32_t pos, void(func_remove)(type *)) \
    { \
        ntree_node_delete(cast(node, NNode), pos, (FPtr_remove)func_remove); \
    } \
\
    static __TYPECHECK type *treest_##type##_node_data(NodeSt(type) *node); \
    static type *treest_##type##_node_data(NodeSt(type) *node) \
    { \
        return cast(ntree_node_data(cast(node, NNode)), type); \
    } \
\
    static __TYPECHECK const type *treest_##type##_node_data_const(const NodeSt(type) *node); \
    static const type *treest_##type##_node_data_const(const NodeSt(type) *node) \
    { \
        return cast_const(ntree_node_data(cast(node, NNode)), type); \
    } \
\
    typedef struct _treestend##type##_t treestend##type
