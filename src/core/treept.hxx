/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: treept.hxx
 * https://www.nappgui.com/en/core/treept.html
 *
 */

/* Tree macros for type checking at compile time */

#if defined __ASSERTS__
#define TreePtDebug(type) \
    struct Node##Pt##type \
    { \
        NTree *tree; \
        struct Node##Pt##type *parent; \
        Array *children; \
        type *data; \
    }; \
\
    struct Tree##Pt##type \
    { \
        uint16_t esize; \
        bool_t traversing; \
        struct Node##Pt##type *root; \
        struct Node##Pt##type *current; \
    }
#else
#define TreePtDebug(type) \
    typedef struct _treeptnonused##type##_t treeptnonused##type
#endif

#define TreePtFuncs(type) \
    TreePt(type); \
    NodePt(type); \
\
    static __TYPECHECK TreePt(type) *treept_##type##_create(void); \
    static TreePt(type) *treept_##type##_create(void) \
    { \
        return cast(ntree_create((uint16_t)sizeof(void *), cast_const(TREEPT #type, char_t)), TreePt(type)); \
    } \
\
    static __TYPECHECK TreePt(type) *treept_##type##_copy(const TreePt(type) *tree, type *(func_copy)(const type *)); \
    static TreePt(type) *treept_##type##_copy(const TreePt(type) *tree, type *(func_copy)(const type *)) \
    { \
        return cast(ntree_copy_ptr(cast_const(tree, NTree), (FPtr_copy)func_copy, cast_const(TREEPT #type, char_t)), TreePt(type)); \
    } \
\
    static __TYPECHECK TreePt(type) *treept_##type##_read(Stream *stream, type *(func_read)(Stream *)); \
    static TreePt(type) *treept_##type##_read(Stream *stream, type *(func_read)(Stream *)) \
    { \
        return cast(ntree_read_ptr(stream, (FPtr_read)func_read, cast_const(TREEPT #type, char_t)), TreePt(type)); \
    } \
\
    static __TYPECHECK TreePt(type) *treept_##type##_read_ex(Stream *stream, FPtr_read_ex func_read, void *data); \
    static TreePt(type) *treept_##type##_read_ex(Stream *stream, FPtr_read_ex func_read, void *data) \
    { \
        return cast(ntree_read_ptr_ex(stream, func_read, data, cast_const(TREEPT #type, char_t)), TreePt(type)); \
    } \
\
    static __TYPECHECK void treept_##type##_destroy(struct Tree##Pt##type **tree, void(func_destroy)(type **)); \
    static void treept_##type##_destroy(struct Tree##Pt##type **tree, void(func_destroy)(type **)) \
    { \
        ntree_destroy_ptr(dcast(tree, NTree), (FPtr_destroy)func_destroy, cast_const(TREEPT #type, char_t)); \
    } \
\
    static __TYPECHECK void treept_##type##_destopt(struct Tree##Pt##type **tree, void(func_destroy)(type **)); \
    static void treept_##type##_destopt(struct Tree##Pt##type **tree, void(func_destroy)(type **)) \
    { \
        ntree_destopt_ptr(dcast(tree, NTree), (FPtr_destroy)func_destroy, cast_const(TREEPT #type, char_t)); \
    } \
\
    static __TYPECHECK void treept_##type##_clear(TreePt(type) *tree, void(func_destroy)(type **)); \
    static void treept_##type##_clear(TreePt(type) *tree, void(func_destroy)(type **)) \
    { \
        ntree_clear_ptr(cast(tree, NTree), (FPtr_destroy)func_destroy); \
    } \
\
    static __TYPECHECK void treept_##type##_write(Stream *stream, const TreePt(type) *tree, void(func_write)(Stream *, const type *)); \
    static void treept_##type##_write(Stream *stream, const TreePt(type) *tree, void(func_write)(Stream *, const type *)) \
    { \
        ntree_write_ptr(stream, cast_const(tree, NTree), (FPtr_write)func_write); \
    } \
\
    static __TYPECHECK void treept_##type##_write_ex(Stream *stream, const TreePt(type) *tree, FPtr_write_ex func_write, const void *data); \
    static void treept_##type##_write_ex(Stream *stream, const TreePt(type) *tree, FPtr_write_ex func_write, const void *data) \
    { \
        ntree_write_ptr_ex(stream, cast_const(tree, NTree), func_write, data); \
    } \
\
    static __TYPECHECK uint32_t treept_##type##_esize(const TreePt(type) *tree); \
    static uint32_t treept_##type##_esize(const TreePt(type) *tree) \
    { \
        return ntree_esize(cast_const(tree, NTree)); \
    } \
\
    static __TYPECHECK uint32_t treept_##type##_mem(const TreePt(type) *tree); \
    static uint32_t treept_##type##_mem(const TreePt(type) *tree) \
    { \
        return ntree_mem(cast_const(tree, NTree)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_root_get(TreePt(type) *tree); \
    static NodePt(type) *treept_##type##_root_get(TreePt(type) *tree) \
    { \
        return cast(ntree_root_get(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK const NodePt(type) *treept_##type##_root_get_const(const TreePt(type) *tree); \
    static const NodePt(type) *treept_##type##_root_get_const(const TreePt(type) *tree) \
    { \
        return cast_const(ntree_root_get(cast_const(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_root_new(TreePt(type) *tree, type *ptr); \
    static NodePt(type) *treept_##type##_root_new(TreePt(type) *tree, type *ptr) \
    { \
        NNode *nnode = ntree_root_new(cast(tree, NTree)); \
        *dcast(ntree_node_data(nnode), type) = ptr; \
        return cast(nnode, NodePt(type)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_dfs_first(TreePt(type) *tree); \
    static NodePt(type) *treept_##type##_dfs_first(TreePt(type) *tree) \
    { \
        return cast(ntree_dfs_first(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK const NodePt(type) *treept_##type##_dfs_first_const(const TreePt(type) *tree); \
    static const NodePt(type) *treept_##type##_dfs_first_const(const TreePt(type) *tree) \
    { \
        return cast_const(ntree_dfs_first(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_dfs_last(TreePt(type) *tree); \
    static NodePt(type) *treept_##type##_dfs_last(TreePt(type) *tree) \
    { \
        return cast(ntree_dfs_last(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK const NodePt(type) *treept_##type##_dfs_last_const(const TreePt(type) *tree); \
    static const NodePt(type) *treept_##type##_dfs_last_const(const TreePt(type) *tree) \
    { \
        return cast_const(ntree_dfs_last(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_next(TreePt(type) *tree); \
    static NodePt(type) *treept_##type##_next(TreePt(type) *tree) \
    { \
        return cast(ntree_next(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK const NodePt(type) *treept_##type##_next_const(const TreePt(type) *tree); \
    static const NodePt(type) *treept_##type##_next_const(const TreePt(type) *tree) \
    { \
        return cast_const(ntree_next(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_prev(TreePt(type) *tree); \
    static NodePt(type) *treept_##type##_prev(TreePt(type) *tree) \
    { \
        return cast(ntree_prev(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK const NodePt(type) *treept_##type##_prev_const(const TreePt(type) *tree); \
    static const NodePt(type) *treept_##type##_prev_const(const TreePt(type) *tree) \
    { \
        return cast_const(ntree_prev(cast(tree, NTree)), NodePt(type)); \
    } \
\
    static __TYPECHECK void treept_##type##_dfs_stop(TreePt(type) *tree); \
    static void treept_##type##_dfs_stop(TreePt(type) *tree) \
    { \
        ntree_dfs_stop(cast(tree, NTree)); \
    } \
\
    static __TYPECHECK uint32_t treept_##type##_node_size(const NodePt(type) *node); \
    static uint32_t treept_##type##_node_size(const NodePt(type) *node) \
    { \
        return ntree_node_size(cast_const(node, NNode)); \
    } \
\
    static __TYPECHECK uint32_t treept_##type##_node_depth(const NodePt(type) *node); \
    static uint32_t treept_##type##_node_depth(const NodePt(type) *node) \
    { \
        return ntree_node_depth(cast_const(node, NNode)); \
    } \
\
    static __TYPECHECK uint32_t treept_##type##_node_index(const NodePt(type) *node); \
    static uint32_t treept_##type##_node_index(const NodePt(type) *node) \
    { \
        return ntree_node_index(cast_const(node, NNode)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_node_parent(NodePt(type) *node); \
    static NodePt(type) *treept_##type##_node_parent(NodePt(type) *node) \
    { \
        return cast(ntree_node_parent(cast(node, NNode)), NodePt(type)); \
    } \
\
    static __TYPECHECK const NodePt(type) *treept_##type##_node_parent_const(const NodePt(type) *node); \
    static const NodePt(type) *treept_##type##_node_parent_const(const NodePt(type) *node) \
    { \
        return cast_const(ntree_node_parent(cast_const(node, NNode)), NodePt(type)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_node_get(NodePt(type) *node, const uint32_t pos); \
    static NodePt(type) *treept_##type##_node_get(NodePt(type) *node, const uint32_t pos) \
    { \
        return cast(ntree_node_get(cast(node, NNode), pos), NodePt(type)); \
    } \
\
    static __TYPECHECK const NodePt(type) *treept_##type##_node_get_const(const NodePt(type) *node, const uint32_t pos); \
    static const NodePt(type) *treept_##type##_node_get_const(const NodePt(type) *node, const uint32_t pos) \
    { \
        return cast_const(ntree_node_get(cast_const(node, NNode), pos), NodePt(type)); \
    } \
\
    static __TYPECHECK NodePt(type) *treept_##type##_node_insert(NodePt(type) *node, const uint32_t pos, type *ptr); \
    static NodePt(type) *treept_##type##_node_insert(NodePt(type) *node, const uint32_t pos, type *ptr) \
    { \
        NNode *nnode = ntree_node_insert(cast(node, NNode), pos); \
        *dcast(ntree_node_data(nnode), type) = ptr; \
        return cast(nnode, NodePt(type)); \
    } \
\
    static __TYPECHECK void treept_##type##_node_delete(NodePt(type) *node, const uint32_t pos, void(func_destroy)(type **)); \
    static void treept_##type##_node_delete(NodePt(type) *node, const uint32_t pos, void(func_destroy)(type **)) \
    { \
        ntree_node_delete_ptr(cast(node, NNode), pos, (FPtr_destroy)func_destroy); \
    } \
\
    static __TYPECHECK type *treept_##type##_node_data(NodePt(type) *node); \
    static type *treept_##type##_node_data(NodePt(type) *node) \
    { \
        return *dcast(ntree_node_data(cast(node, NNode)), type); \
    } \
\
    static __TYPECHECK const type *treept_##type##_node_data_const(const NodePt(type) *node); \
    static const type *treept_##type##_node_data_const(const NodePt(type) *node) \
    { \
        return *dcast_const(ntree_node_data(cast(node, NNode)), type); \
    } \
\
    typedef struct _treeptend##type##_t treeptend##type
