/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: treest.hpp
 * https://www.nappgui.com/en/core/treest.html
 *
 */

/* Trees */

#ifndef __TREEST_HPP__
#define __TREEST_HPP__

#include <sewer/bstd.h>
#include <sewer/nowarn.hxx>
#include <typeinfo>
#include <sewer/warn.hxx>

template < class type >
struct TreeSt
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

        static Node *insert(Node *node, const uint32_t pos);

        static void ddelete(Node *node, const uint32_t pos, void (*func_remove)(type *));

        static type *data(Node *node);

        static const type *data(const Node *node);

#if defined __ASSERTS__
        NTree *tree_ref;
        Node *paren;
        Array *children;
        type elem;
#endif
    };

    static TreeSt< type > *create(void);

    static TreeSt< type > *copy(const TreeSt< type > *tree, void (*func_copy)(type *, const type *));

    static TreeSt< type > *read(Stream *stm, void (*func_read)(Stream *, type *));

    static void destroy(TreeSt< type > **tree, void (*func_remove)(type *));

    static void destopt(TreeSt< type > **tree, void (*func_remove)(type *));

    static void clear(TreeSt< type > *tree, void (*func_remove)(type *));

    static void write(Stream *stm, const TreeSt< type > *tree, void (*func_write)(Stream *, const type *));

    static uint32_t esize(const TreeSt< type > *tree);

    static uint32_t mem(const TreeSt< type > *tree);

    static Node *root_get(TreeSt< type > *tree);

    static const Node *root_get(const TreeSt< type > *tree);

    static Node *root_new(TreeSt< type > *tree);

    static Node *dfs_first(TreeSt< type > *tree);

    static const Node *dfs_first(const TreeSt< type > *tree);

    static Node *dfs_last(TreeSt< type > *tree);

    static const Node *dfs_last(const TreeSt< type > *tree);

    static Node *next(TreeSt< type > *tree);

    static const Node *next(const TreeSt< type > *tree);

    static Node *prev(TreeSt< type > *tree);

    static const Node *prev(const TreeSt< type > *tree);

    static void dfs_stop(TreeSt< type > *tree);

#if defined __ASSERTS__
    // Only for debugger inspector (non used)
    uint16_t elem_sizeof;
    bool_t traversing;
    Node *root_node;
    Node *current_node;
#endif
};

template < typename type, typename dtype >
struct TreeS2
{
    static TreeSt< type > *read_ex(Stream *stm, void (*func_read)(Stream *, type *, const dtype *), const dtype *data);

    static void write_ex(Stream *stm, const TreeSt< type > *tree, void (*func_write)(Stream *, const type *, const dtype *), const dtype *data);
};

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreeSt< type >::Node::size(const Node *node)
{
    return ntree_node_size(cast_const(node, NNode));
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreeSt< type >::Node::depth(const Node *node)
{
    return ntree_node_depth(cast_const(node, NNode));
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreeSt< type >::Node::index(const Node *node)
{
    return ntree_node_index(cast_const(node, NNode));
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::Node::parent(Node *node)
{
    return cast(ntree_node_parent(cast_const(node, NNode)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreeSt< type >::Node *TreeSt< type >::Node::parent(const Node *node)
{
    return cast_const(ntree_node_parent(cast_const(node, NNode)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::Node::get(Node *node, const uint32_t pos)
{
    return cast(ntree_node_get(cast_const(node, NNode), pos), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreeSt< type >::Node *TreeSt< type >::Node::get(const Node *node, const uint32_t pos)
{
    return cast_const(ntree_node_get(cast_const(node, NNode), pos), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::Node::insert(Node *node, const uint32_t pos)
{
    return cast(ntree_node_insert(cast(node, NNode), pos), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreeSt< type >::Node::ddelete(Node *node, const uint32_t pos, void (*func_remove)(type *))
{
    ntree_node_delete(cast(node, NNode), pos, (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *TreeSt< type >::Node::data(Node *node)
{
    return cast(ntree_node_data(cast(node, NNode)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *TreeSt< type >::Node::data(const Node *node)
{
    return cast_const(ntree_node_data(cast(node, NNode)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
TreeSt< type > *TreeSt< type >::create(void)
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreeSt<%s>", typeid(type).name());
    return cast(ntree_create((uint16_t)sizeof(type), dtype), TreeSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
TreeSt< type > *TreeSt< type >::copy(const TreeSt< type > *tree, void (*func_copy)(type *, const type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreeSt<%s>", typeid(type).name());
    return cast(ntree_copy(cast_const(tree, NTree), (FPtr_scopy)func_copy, dtype), TreeSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
TreeSt< type > *TreeSt< type >::read(Stream *stm, void (*func_read)(Stream *, type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreeSt<%s>", typeid(type).name());
    return cast(ntree_read(stm, (uint16_t)sizeof(type), (FPtr_read_init)func_read, dtype), TreeSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreeSt< type >::destroy(TreeSt< type > **tree, void (*func_remove)(type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreeSt<%s>", typeid(type).name());
    ntree_destroy(dcast(tree, NTree), (FPtr_remove)func_remove, dtype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreeSt< type >::destopt(TreeSt< type > **tree, void (*func_remove)(type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "TreeSt<%s>", typeid(type).name());
    ntree_destopt(dcast(tree, NTree), (FPtr_remove)func_remove, dtype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreeSt< type >::clear(TreeSt< type > *tree, void (*func_remove)(type *))
{
    ntree_clear(cast(tree, NTree), (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreeSt< type >::write(Stream *stm, const TreeSt< type > *tree, void (*func_write)(Stream *, const type *))
{
    ntree_write(stm, cast_const(tree, NTree), (FPtr_write)func_write);
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreeSt< type >::esize(const TreeSt< type > *tree)
{
    return ntree_esize(cast_const(tree, NTree));
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t TreeSt< type >::mem(const TreeSt< type > *tree)
{
    return ntree_mem(cast_const(tree, NTree));
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::root_get(TreeSt< type > *tree)
{
    return cast(ntree_root_get(cast_const(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreeSt< type >::Node *TreeSt< type >::root_get(const TreeSt< type > *tree)
{
    return cast_const(ntree_root_get(cast_const(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::root_new(TreeSt< type > *tree)
{
    return cast(ntree_root_new(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::dfs_first(TreeSt< type > *tree)
{
    return cast(ntree_dfs_first(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreeSt< type >::Node *TreeSt< type >::dfs_first(const TreeSt< type > *tree)
{
    return cast_const(ntree_dfs_first(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::dfs_last(TreeSt< type > *tree)
{
    return cast(ntree_dfs_last(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreeSt< type >::Node *TreeSt< type >::dfs_last(const TreeSt< type > *tree)
{
    return cast_const(ntree_dfs_last(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::next(TreeSt< type > *tree)
{
    return cast(ntree_next(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreeSt< type >::Node *TreeSt< type >::next(const TreeSt< type > *tree)
{
    return cast_const(ntree_next(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
typename TreeSt< type >::Node *TreeSt< type >::prev(TreeSt< type > *tree)
{
    return cast(ntree_prev(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const typename TreeSt< type >::Node *TreeSt< type >::prev(const TreeSt< type > *tree)
{
    return cast_const(ntree_prev(cast(tree, NTree)), Node);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void TreeSt< type >::dfs_stop(TreeSt< type > *tree)
{
    ntree_dfs_stop(cast(tree, NTree));
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
TreeSt< type > *TreeS2< type, dtype >::read_ex(Stream *stm, void (*func_read)(Stream *, type *, const dtype *), const dtype *data)
{
    char_t ntype[64];
    bstd_sprintf(ntype, sizeof(ntype), "TreeSt<%s>", typeid(type).name());
    return cast(ntree_read_ex(stm, (uint16_t)sizeof(type), (FPtr_read_init_ex)func_read, cast(data, void), ntype), TreeSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
void TreeS2< type, dtype >::write_ex(Stream *stm, const TreeSt< type > *tree, void (*func_write)(Stream *, const type *, const dtype *), const dtype *data)
{
    ntree_write_ex(stm, cast_const(tree, NTree), (FPtr_write_ex)func_write, cast_const(data, void));
}

#endif
