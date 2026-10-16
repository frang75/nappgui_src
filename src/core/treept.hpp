/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: treept.hpp
 * https://www.nappgui.com/en/core/treept.html
 *
 */

/* Trees (pointer) */

#ifndef __TREEPT_HPP__
#define __TREEPT_HPP__

#include <sewer/bstd.h>
#include <sewer/nowarn.hxx>
#include <typeinfo>
#include <sewer/warn.hxx>

template < class type >
struct TreePt
{
    struct Node
    {
        static uint32_t size(const Node *node);

        static uint32_t depth(const Node *node);

        static uint32_t index(const Node *node);

        static Node *parent(Node *node);

        static const Node *parent(const Node *node);

        static Node *get(Node *node, const uint32_t pos);

        static const Node *get(const Node *node, const uint32_t pos);

        static Node *insert(Node *node, const uint32_t pos, type *ptr);

        static void ddelete(Node *node, const uint32_t pos, void (*func_destroy)(type **));

        static type *data(Node *node);

        static const type *data(const Node *node);

#if defined __ASSERTS__
        NTree *tree_ref;
        Node *paren;
        Array *children;
        type *elem;
#endif
    };

    static TreePt< type > *create(void);

    static TreePt< type > *copy(const TreePt< type > *tree, type *(*func_copy)(const type *));

    static TreePt< type > *read(Stream *stm, type *(*func_read)(Stream *));

    static void destroy(TreePt< type > **tree, void (*func_destroy)(type **));

    static void destopt(TreePt< type > **tree, void (*func_destroy)(type **));

    static void clear(TreePt< type > *tree, void (*func_destroy)(type **));

    static void write(Stream *stm, const TreePt< type > *tree, void (*func_write)(Stream *, const type *));

    static uint32_t esize(const TreePt< type > *tree);

    static uint32_t mem(const TreePt< type > *tree);

    static Node *root_get(TreePt< type > *tree);

    static const Node *root_get(const TreePt< type > *tree);

    static Node *root_new(TreePt< type > *tree, type *ptr);

    static Node *dfs_first(TreePt< type > *tree);

    static const Node *dfs_first(const TreePt< type > *tree);

    static Node *dfs_last(TreePt< type > *tree);

    static const Node *dfs_last(const TreePt< type > *tree);

    static Node *next(TreePt< type > *tree);

    static const Node *next(const TreePt< type > *tree);

    static Node *prev(TreePt< type > *tree);

    static const Node *prev(const TreePt< type > *tree);

    static void dfs_stop(TreePt< type > *tree);

#if defined __ASSERTS__
    // Only for debugger inspector (non used)
    uint16_t elem_sizeof;
    bool_t traversing;
    Node *root_node;
    Node *current_node;
#endif
};

template < typename type, typename dtype >
struct TreeP2
{
    static TreePt< type > *read_ex(Stream *stm, type *(*func_read)(Stream *, const dtype *), const dtype *data);

    static void write_ex(Stream *stm, const TreePt< type > *tree, void (*func_write)(Stream *, const type *, const dtype *), const dtype *data);
};

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreePt< type >::Node::size(const Node *node)
{
    return ntree_node_size(cast_const(node, NNode));
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreePt< type >::Node::depth(const Node *node)
{
    return ntree_node_depth(cast_const(node, NNode));
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreePt< type >::Node::index(const Node *node)
{
    return ntree_node_index(cast_const(node, NNode));
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::Node::parent(Node *node)
{
    return cast(ntree_node_parent(cast_const(node, NNode)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreePt< type >::Node *TreePt< type >::Node::parent(const Node *node)
{
    return cast_const(ntree_node_parent(cast_const(node, NNode)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::Node::get(Node *node, const uint32_t pos)
{
    return cast(ntree_node_get(cast_const(node, NNode), pos), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreePt< type >::Node *TreePt< type >::Node::get(const Node *node, const uint32_t pos)
{
    return cast_const(ntree_node_get(cast_const(node, NNode), pos), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::Node::insert(Node *node, const uint32_t pos, type *ptr)
{
    NNode *nnode = ntree_node_insert(cast(node, NNode), pos);
    *dcast(ntree_node_data(nnode), type) = ptr;
    return cast(nnode, Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreePt< type >::Node::ddelete(Node *node, const uint32_t pos, void (*func_destroy)(type **))
{
    ntree_node_delete_ptr(cast(node, NNode), pos, (FPtr_destroy)func_destroy);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *TreePt< type >::Node::data(Node *node)
{
    return *dcast(ntree_node_data(cast(node, NNode)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *TreePt< type >::Node::data(const Node *node)
{
    return *dcast_const(ntree_node_data(cast(node, NNode)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
TreePt< type > *TreePt< type >::create(void)
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreePt<%s>", typeid(type).name());
    return cast(ntree_create((uint16_t)sizeof(void *), dtype), TreePt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
TreePt< type > *TreePt< type >::copy(const TreePt< type > *tree, type *(*func_copy)(const type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreePt<%s>", typeid(type).name());
    return cast(ntree_copy_ptr(cast_const(tree, NTree), (FPtr_copy)func_copy, dtype), TreePt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
TreePt< type > *TreePt< type >::read(Stream *stm, type *(*func_read)(Stream *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreePt<%s>", typeid(type).name());
    return cast(ntree_read_ptr(stm, (FPtr_read)func_read, dtype), TreePt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreePt< type >::destroy(TreePt< type > **tree, void (*func_destroy)(type **))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreePt<%s>", typeid(type).name());
    ntree_destroy_ptr(dcast(tree, NTree), (FPtr_destroy)func_destroy, dtype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreePt< type >::destopt(TreePt< type > **tree, void (*func_destroy)(type **))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreePt<%s>", typeid(type).name());
    ntree_destopt_ptr(dcast(tree, NTree), (FPtr_destroy)func_destroy, dtype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreePt< type >::clear(TreePt< type > *tree, void (*func_destroy)(type **))
{
    ntree_clear_ptr(cast(tree, NTree), (FPtr_destroy)func_destroy);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreePt< type >::write(Stream *stm, const TreePt< type > *tree, void (*func_write)(Stream *, const type *))
{
    ntree_write_ptr(stm, cast_const(tree, NTree), (FPtr_write)func_write);
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreePt< type >::esize(const TreePt< type > *tree)
{
    return ntree_esize(cast_const(tree, NTree));
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreePt< type >::mem(const TreePt< type > *tree)
{
    return ntree_mem(cast_const(tree, NTree));
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::root_get(TreePt< type > *tree)
{
    return cast(ntree_root_get(cast_const(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreePt< type >::Node *TreePt< type >::root_get(const TreePt< type > *tree)
{
    return cast_const(ntree_root_get(cast_const(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::root_new(TreePt< type > *tree, type *ptr)
{
    NNode *nnode = ntree_root_new(cast(tree, NTree));
    *dcast(ntree_node_data(nnode), type) = ptr;
    return cast(nnode, Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::dfs_first(TreePt< type > *tree)
{
    return cast(ntree_dfs_first(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreePt< type >::Node *TreePt< type >::dfs_first(const TreePt< type > *tree)
{
    return cast_const(ntree_dfs_first(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::dfs_last(TreePt< type > *tree)
{
    return cast(ntree_dfs_last(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreePt< type >::Node *TreePt< type >::dfs_last(const TreePt< type > *tree)
{
    return cast_const(ntree_dfs_last(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::next(TreePt< type > *tree)
{
    return cast(ntree_next(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreePt< type >::Node *TreePt< type >::next(const TreePt< type > *tree)
{
    return cast_const(ntree_next(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreePt< type >::Node *TreePt< type >::prev(TreePt< type > *tree)
{
    return cast(ntree_prev(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreePt< type >::Node *TreePt< type >::prev(const TreePt< type > *tree)
{
    return cast_const(ntree_prev(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreePt< type >::dfs_stop(TreePt< type > *tree)
{
    ntree_dfs_stop(cast(tree, NTree));
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
TreePt< type > *TreeP2< type, dtype >::read_ex(Stream *stm, type *(*func_read)(Stream *, const dtype *), const dtype *data)
{
    char_t ntype[64];
    bstd_sprintf(ntype, sizeof(ntype), "TreePt<%s>", typeid(type).name());
    return cast(ntree_read_ptr_ex(stm, (FPtr_read_ex)func_read, cast(data, void), ntype), TreePt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
void TreeP2< type, dtype >::write_ex(Stream *stm, const TreePt< type > *tree, void (*func_write)(Stream *, const type *, const dtype *), const dtype *data)
{
    ntree_write_ptr_ex(stm, cast_const(tree, NTree), (FPtr_write_ex)func_write, cast_const(data, void));
}

#endif
