/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: ntree.c
 *
 */

/* N-Ary tree structure */

#include "ntree.h"
#include "array.h"
#include "heap.h"
#include "stream.h"
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

struct _ntree_t
{
    uint16_t esize;
    bool_t traversing;
    NNode *root;
    NNode *current;
};

struct _nnode_t
{
    NTree *tree;
    NNode *parent;
    Array *children;
};

/*---------------------------------------------------------------------------*/

#define i_NODE_DATA(node) \
    ((void)(cast(node, NNode) == node), \
     (cast(node, byte_t) + sizeof(NNode)))

/*---------------------------------------------------------------------------*/

NTree *ntree_create(const uint16_t esize, const char_t *type)
{
    NTree *tree = cast(heap_malloc(sizeof32(NTree), type), NTree);
    tree->esize = esize;
    tree->traversing = FALSE;
    tree->root = NULL;
    tree->current = NULL;
    return tree;
}

/*---------------------------------------------------------------------------*/

static NNode *i_create_node(NTree *tree, NNode *parent)
{
    NNode *node = cast(heap_malloc(sizeof32(NNode) + tree->esize, "NNode"), NNode);
    node->tree = tree;
    node->parent = parent;
    node->children = NULL;
    return node;
}

/*---------------------------------------------------------------------------*/

static NNode *i_copy_node(NTree *ntree, const NNode *node, NNode *parent, FPtr_scopy func_copy)
{
    uint32_t i, n;
    NNode *nnode = i_create_node(ntree, parent);
    if (func_copy != NULL)
        func_copy(i_NODE_DATA(nnode), i_NODE_DATA(cast(node, NNode)));
    else
        bmem_copy(i_NODE_DATA(nnode), i_NODE_DATA(cast(node, NNode)), ntree->esize);
    if (node->children != NULL)
    {
        n = array_size(node->children);
        nnode->children = array_create(sizeofptr, "NNode");
        for (i = 0; i < n; ++i)
        {
            const NNode *child = *dcast(array_get(node->children, i), NNode);
            NNode *nchild = i_copy_node(ntree, child, nnode, func_copy);
            *dcast(array_insert(nnode->children, i, 1), NNode) = nchild;
        }
    }
    return nnode;
}

/*---------------------------------------------------------------------------*/

static NNode *i_copy_node_ptr(NTree *ntree, const NNode *node, NNode *parent, FPtr_copy func_copy)
{
    uint32_t i, n;
    NNode *nnode = i_create_node(ntree, parent);
    if (func_copy != NULL)
    {
        void *src_ptr = *dcast(i_NODE_DATA(cast(node, NNode)), void);
        *dcast(i_NODE_DATA(nnode), void) = func_copy(src_ptr);
    }
    else
    {
        bmem_copy(i_NODE_DATA(nnode), i_NODE_DATA(cast(node, NNode)), ntree->esize);
    }
    if (node->children != NULL)
    {
        n = array_size(node->children);
        nnode->children = array_create(sizeofptr, "NNode");
        for (i = 0; i < n; ++i)
        {
            const NNode *child = *dcast(array_get(node->children, i), NNode);
            NNode *nchild = i_copy_node_ptr(ntree, child, nnode, func_copy);
            *dcast(array_insert(nnode->children, i, 1), NNode) = nchild;
        }
    }
    return nnode;
}

/*---------------------------------------------------------------------------*/

NTree *ntree_copy(const NTree *tree, FPtr_scopy func_copy, const char_t *type)
{
    NTree *ntree = NULL;
    cassert_no_null(tree);
    ntree = ntree_create(tree->esize, type);
    if (tree->root != NULL)
        ntree->root = i_copy_node(ntree, tree->root, NULL, func_copy);
    return ntree;
}

/*---------------------------------------------------------------------------*/

NTree *ntree_copy_ptr(const NTree *tree, FPtr_copy func_copy, const char_t *type)
{
    NTree *ntree = NULL;
    cassert_no_null(tree);
    ntree = ntree_create(tree->esize, type);
    if (tree->root != NULL)
        ntree->root = i_copy_node_ptr(ntree, tree->root, NULL, func_copy);
    return ntree;
}

/*---------------------------------------------------------------------------*/

static NNode *i_read_node(Stream *stream, NTree *tree, NNode *parent, FPtr_read_init func_read_init, FPtr_read_init_ex func_read_init_ex, void *data)
{
    uint32_t i, n;
    NNode *node;
    n = stm_read_u32(stream);
    node = i_create_node(tree, parent);
    if (func_read_init_ex != NULL)
        func_read_init_ex(stream, i_NODE_DATA(node), data);
    else
        func_read_init(stream, i_NODE_DATA(node));
    if (n > 0)
    {
        node->children = array_create(sizeofptr, "NNode");
        for (i = 0; i < n; ++i)
        {
            NNode *child = i_read_node(stream, tree, node, func_read_init, func_read_init_ex, data);
            *dcast(array_insert(node->children, i, 1), NNode) = child;
        }
    }
    return node;
}

/*---------------------------------------------------------------------------*/

static NNode *i_read_node_ptr(Stream *stream, NTree *tree, NNode *parent, FPtr_read func_read, FPtr_read_ex func_read_ex, void *data)
{
    uint32_t i, n;
    bool_t nonull;
    void *ptr;
    NNode *node;
    n = stm_read_u32(stream);
    node = i_create_node(tree, parent);
    nonull = stm_read_bool(stream);
    if (nonull == TRUE)
    {
        if (func_read_ex != NULL)
            ptr = func_read_ex(stream, data);
        else
            ptr = func_read(stream);
    }
    else
    {
        ptr = NULL;
    }
    *dcast(i_NODE_DATA(node), void) = ptr;
    if (n > 0)
    {
        node->children = array_create(sizeofptr, "NNode");
        for (i = 0; i < n; ++i)
        {
            NNode *child = i_read_node_ptr(stream, tree, node, func_read, func_read_ex, data);
            *dcast(array_insert(node->children, i, 1), NNode) = child;
        }
    }
    return node;
}

/*---------------------------------------------------------------------------*/

NTree *ntree_read(Stream *stream, const uint16_t esize, FPtr_read_init func_read_init, const char_t *type)
{
    bool_t has_root;
    NTree *tree = ntree_create(esize, type);
    cassert_no_nullf(func_read_init);
    has_root = stm_read_bool(stream);
    if (has_root == TRUE)
        tree->root = i_read_node(stream, tree, NULL, func_read_init, NULL, NULL);
    return tree;
}

/*---------------------------------------------------------------------------*/

NTree *ntree_read_ex(Stream *stream, const uint16_t esize, FPtr_read_init_ex func_read_init, void *data, const char_t *type)
{
    bool_t has_root;
    NTree *tree = ntree_create(esize, type);
    cassert_no_nullf(func_read_init);
    has_root = stm_read_bool(stream);
    if (has_root == TRUE)
        tree->root = i_read_node(stream, tree, NULL, NULL, func_read_init, data);
    return tree;
}

/*---------------------------------------------------------------------------*/

NTree *ntree_read_ptr(Stream *stream, FPtr_read func_read, const char_t *type)
{
    bool_t has_root;
    NTree *tree = ntree_create(sizeofptr, type);
    cassert_no_nullf(func_read);
    has_root = stm_read_bool(stream);
    if (has_root == TRUE)
        tree->root = i_read_node_ptr(stream, tree, NULL, func_read, NULL, NULL);
    return tree;
}

/*---------------------------------------------------------------------------*/

NTree *ntree_read_ptr_ex(Stream *stream, FPtr_read_ex func_read, void *data, const char_t *type)
{
    bool_t has_root;
    NTree *tree = ntree_create(sizeofptr, type);
    cassert_no_nullf(func_read);
    has_root = stm_read_bool(stream);
    if (has_root == TRUE)
        tree->root = i_read_node_ptr(stream, tree, NULL, NULL, func_read, data);
    return tree;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_node(NNode **node, FPtr_remove func_remove)
{
    uint16_t esize;
    uint32_t i, n;
    cassert_no_null(node);
    cassert_no_null(*node);
    cassert_no_null((*node)->tree);
    esize = (*node)->tree->esize;
    if ((*node)->children != NULL)
    {
        n = array_size((*node)->children);
        for (i = 0; i < n; ++i)
        {
            NNode **child = dcast(array_get((*node)->children, i), NNode);
            i_destroy_node(child, func_remove);
        }
        array_destroy(&(*node)->children, NULL, "NNode");
    }
    if (func_remove != NULL)
        func_remove(i_NODE_DATA(*node));
    heap_free(dcast(node, byte_t), sizeof32(NNode) + esize, "NNode");
}

/*---------------------------------------------------------------------------*/

static void i_destroy_node_ptr(NNode **node, FPtr_destroy func_destroy)
{
    uint16_t esize;
    uint32_t i, n;
    cassert_no_null(node);
    cassert_no_null(*node);
    cassert_no_null((*node)->tree);
    esize = (*node)->tree->esize;
    if ((*node)->children != NULL)
    {
        n = array_size((*node)->children);
        for (i = 0; i < n; ++i)
        {
            NNode **child = dcast(array_get((*node)->children, i), NNode);
            i_destroy_node_ptr(child, func_destroy);
        }
        array_destroy(&(*node)->children, NULL, "NNode");
    }
    if (func_destroy != NULL)
        func_destroy(dcast(i_NODE_DATA(*node), void));
    heap_free(dcast(node, byte_t), sizeof32(NNode) + esize, "NNode");
}

/*---------------------------------------------------------------------------*/

static void i_clear(NTree *tree, FPtr_remove func_remove)
{
    cassert_no_null(tree);
    cassert(tree->traversing == FALSE);
    if (tree->root != NULL)
        i_destroy_node(&tree->root, func_remove);
    tree->root = NULL;
    tree->current = NULL;
}

/*---------------------------------------------------------------------------*/

static void i_clear_ptr(NTree *tree, FPtr_destroy func_destroy)
{
    cassert_no_null(tree);
    cassert(tree->traversing == FALSE);
    if (tree->root != NULL)
        i_destroy_node_ptr(&tree->root, func_destroy);
    tree->root = NULL;
    tree->current = NULL;
}

/*---------------------------------------------------------------------------*/

void ntree_destroy(NTree **tree, FPtr_remove func_remove, const char_t *type)
{
    cassert_no_null(tree);
    cassert_no_null(*tree);
    i_clear(*tree, func_remove);
    heap_free(dcast(tree, byte_t), sizeof32(NTree), type);
}

/*---------------------------------------------------------------------------*/

void ntree_destopt(NTree **tree, FPtr_remove func_remove, const char_t *type)
{
    cassert_no_null(tree);
    if (*tree != NULL)
        ntree_destroy(tree, func_remove, type);
}

/*---------------------------------------------------------------------------*/

void ntree_destroy_ptr(NTree **tree, FPtr_destroy func_destroy, const char_t *type)
{
    cassert_no_null(tree);
    cassert_no_null(*tree);
    i_clear_ptr(*tree, func_destroy);
    heap_free(dcast(tree, byte_t), sizeof32(NTree), type);
}

/*---------------------------------------------------------------------------*/

void ntree_destopt_ptr(NTree **tree, FPtr_destroy func_destroy, const char_t *type)
{
    cassert_no_null(tree);
    if (*tree != NULL)
        ntree_destroy_ptr(tree, func_destroy, type);
}

/*---------------------------------------------------------------------------*/

void ntree_clear(NTree *tree, FPtr_remove func_remove)
{
    i_clear(tree, func_remove);
}

/*---------------------------------------------------------------------------*/

void ntree_clear_ptr(NTree *tree, FPtr_destroy func_destroy)
{
    i_clear_ptr(tree, func_destroy);
}

/*---------------------------------------------------------------------------*/

static void i_write_node(Stream *stream, const NNode *node, FPtr_write func_write, FPtr_write_ex func_write_ex, const void *data)
{
    uint32_t i, n;
    if (node->children != NULL)
        n = array_size(node->children);
    else
        n = 0;
    stm_write_u32(stream, n);
    if (func_write_ex != NULL)
        func_write_ex(stream, i_NODE_DATA(cast(node, NNode)), data);
    else
        func_write(stream, i_NODE_DATA(cast(node, NNode)));
    for (i = 0; i < n; ++i)
    {
        const NNode *child = *dcast(array_get(node->children, i), NNode);
        i_write_node(stream, child, func_write, func_write_ex, data);
    }
}

/*---------------------------------------------------------------------------*/

static void i_write_node_ptr(Stream *stream, const NNode *node, FPtr_write func_write, FPtr_write_ex func_write_ex, const void *data)
{
    uint32_t i, n;
    const void *ptr;
    if (node->children != NULL)
        n = array_size(node->children);
    else
        n = 0;
    stm_write_u32(stream, n);
    ptr = *dcast(i_NODE_DATA(cast(node, NNode)), void);
    if (ptr != NULL)
    {
        stm_write_bool(stream, TRUE);
        if (func_write_ex != NULL)
            func_write_ex(stream, ptr, data);
        else
            func_write(stream, ptr);
    }
    else
    {
        stm_write_bool(stream, FALSE);
    }
    for (i = 0; i < n; ++i)
    {
        const NNode *child = *dcast(array_get(node->children, i), NNode);
        i_write_node_ptr(stream, child, func_write, func_write_ex, data);
    }
}

/*---------------------------------------------------------------------------*/

void ntree_write(Stream *stream, const NTree *tree, FPtr_write func_write)
{
    cassert_no_null(tree);
    cassert_no_nullf(func_write);
    stm_write_bool(stream, tree->root != NULL);
    if (tree->root != NULL)
        i_write_node(stream, tree->root, func_write, NULL, NULL);
}

/*---------------------------------------------------------------------------*/

void ntree_write_ex(Stream *stream, const NTree *tree, FPtr_write_ex func_write, const void *data)
{
    cassert_no_null(tree);
    cassert_no_nullf(func_write);
    stm_write_bool(stream, tree->root != NULL);
    if (tree->root != NULL)
        i_write_node(stream, tree->root, NULL, func_write, data);
}

/*---------------------------------------------------------------------------*/

void ntree_write_ptr(Stream *stream, const NTree *tree, FPtr_write func_write)
{
    cassert_no_null(tree);
    cassert_no_nullf(func_write);
    stm_write_bool(stream, tree->root != NULL);
    if (tree->root != NULL)
        i_write_node_ptr(stream, tree->root, func_write, NULL, NULL);
}

/*---------------------------------------------------------------------------*/

void ntree_write_ptr_ex(Stream *stream, const NTree *tree, FPtr_write_ex func_write, const void *data)
{
    cassert_no_null(tree);
    cassert_no_nullf(func_write);
    stm_write_bool(stream, tree->root != NULL);
    if (tree->root != NULL)
        i_write_node_ptr(stream, tree->root, NULL, func_write, data);
}

/*---------------------------------------------------------------------------*/

uint32_t ntree_esize(const NTree *tree)
{
    cassert_no_null(tree);
    return tree->esize;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_node_mem(const NNode *node, const uint16_t esize)
{
    uint32_t i, n;
    uint32_t mem = sizeof32(NNode) + esize;
    if (node->children != NULL)
    {
        mem += array_mem(node->children);
        n = array_size(node->children);
        for (i = 0; i < n; ++i)
        {
            const NNode *child = *dcast(array_get(node->children, i), NNode);
            mem += i_node_mem(child, esize);
        }
    }
    return mem;
}

/*---------------------------------------------------------------------------*/

uint32_t ntree_mem(const NTree *tree)
{
    uint32_t mem = sizeof32(NTree);
    cassert_no_null(tree);
    if (tree->root != NULL)
        mem += i_node_mem(tree->root, tree->esize);
    return mem;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_root_get(const NTree *tree)
{
    cassert_no_null(tree);
    return tree->root;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_root_new(NTree *tree)
{
    cassert_no_null(tree);
    cassert(tree->root == NULL);
    if (tree->root == NULL)
        tree->root = i_create_node(tree, NULL);
    return tree->root;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_dfs_first(NTree *tree)
{
    cassert_no_null(tree);
    tree->current = tree->root;
    if (tree->root != NULL)
        tree->traversing = TRUE;
    return tree->current;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_dfs_last(NTree *tree)
{
    cassert_no_null(tree);
    tree->current = tree->root;
    if (tree->current != NULL)
    {
        tree->traversing = TRUE;
        while (tree->current->children != NULL && array_size(tree->current->children) > 0)
            tree->current = *dcast(array_get_last(tree->current->children), NNode);
    }
    else
    {
        tree->traversing = FALSE;
    }
    return tree->current;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_next(NTree *tree)
{
    NNode *current = NULL;
    uint32_t pos = 0;
    cassert_no_null(tree);
    cassert(tree->traversing == TRUE);
    current = tree->current;
    cassert_no_null(current);
    if (current->children != NULL && array_size(current->children) > 0)
    {
        tree->current = *dcast(array_get(current->children, 0), NNode);
        return tree->current;
    }
    while (current->parent != NULL)
    {
        pos = array_find_ptr(current->parent->children, current);
        cassert(pos < array_size(current->parent->children));
        if (pos + 1 < array_size(current->parent->children))
        {
            tree->current = *dcast(array_get(current->parent->children, pos + 1), NNode);
            return tree->current;
        }
        current = current->parent;
    }
    tree->traversing = FALSE;
    tree->current = NULL;
    return NULL;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_prev(NTree *tree)
{
    NNode *current = NULL;
    NNode *prev = NULL;
    uint32_t pos = 0;
    cassert_no_null(tree);
    cassert(tree->traversing == TRUE);
    current = tree->current;
    cassert_no_null(current);
    if (current->parent == NULL)
    {
        tree->traversing = FALSE;
        tree->current = NULL;
        return NULL;
    }
    pos = array_find_ptr(current->parent->children, current);
    cassert(pos < array_size(current->parent->children));
    if (pos > 0)
    {
        prev = *dcast(array_get(current->parent->children, pos - 1), NNode);
        while (prev->children != NULL && array_size(prev->children) > 0)
            prev = *dcast(array_get_last(prev->children), NNode);
        tree->current = prev;
    }
    else
    {
        tree->current = current->parent;
    }
    return tree->current;
}

/*---------------------------------------------------------------------------*/

void ntree_dfs_stop(NTree *tree)
{
    cassert_no_null(tree);
    cassert(tree->traversing == TRUE);
    tree->traversing = FALSE;
    tree->current = NULL;
}

/*---------------------------------------------------------------------------*/

uint32_t ntree_node_size(const NNode *node)
{
    cassert_no_null(node);
    if (node->children != NULL)
        return array_size(node->children);
    return 0;
}

/*---------------------------------------------------------------------------*/

uint32_t ntree_node_depth(const NNode *node)
{
    uint32_t depth = 0;
    cassert_no_null(node);
    while (node->parent != NULL)
    {
        depth += 1;
        node = node->parent;
    }
    return depth;
}

/*---------------------------------------------------------------------------*/

uint32_t ntree_node_index(const NNode *node)
{
    NNode **children;
    uint32_t i;
    uint32_t n;
    cassert_no_null(node);
    cassert_no_null(node->parent);
    cassert_no_null(node->parent->children);
    n = array_size(node->parent->children);
    children = dcast(array_all(node->parent->children), NNode);
    for (i = 0; i < n; ++i)
    {
        if (children[i] == node)
            return i;
    }
    cassert(FALSE);
    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_node_parent(const NNode *node)
{
    cassert_no_null(node);
    return node->parent;
}

/*---------------------------------------------------------------------------*/

NNode *ntree_node_get(const NNode *node, const uint32_t pos)
{
    cassert_no_null(node);
    cassert_no_null(node->children);
    return *dcast(array_get(node->children, pos), NNode);
}

/*---------------------------------------------------------------------------*/

NNode *ntree_node_insert(NNode *node, const uint32_t pos)
{
    NNode *child = NULL;
    cassert_no_null(node);
    cassert_no_null(node->tree);
    cassert(node->tree->traversing == FALSE);
    if (node->children == NULL)
        node->children = array_create(sizeofptr, "NNode");
    child = i_create_node(node->tree, node);
    *dcast(array_insert(node->children, pos, 1), NNode) = child;
    return child;
}

/*---------------------------------------------------------------------------*/

byte_t *ntree_node_data(NNode *node)
{
    cassert_no_null(node);
    return i_NODE_DATA(node);
}

/*---------------------------------------------------------------------------*/

byte_t *ntree_node_data0(NNode *node)
{
    byte_t *data = NULL;
    cassert_no_null(node);
    cassert_no_null(node->tree);
    data = i_NODE_DATA(node);
    bmem_zero_n(data, node->tree->esize, byte_t);
    return data;
}

/*---------------------------------------------------------------------------*/

void ntree_node_delete(NNode *node, const uint32_t pos, FPtr_remove func_remove)
{
    NNode **child = NULL;
    cassert_no_null(node);
    cassert_no_null(node->tree);
    cassert(node->tree->traversing == FALSE);
    cassert_no_null(node->children);
    child = dcast(array_get(node->children, pos), NNode);
    i_destroy_node(child, func_remove);
    array_delete(node->children, pos, 1, NULL);
}

/*---------------------------------------------------------------------------*/

void ntree_node_delete_ptr(NNode *node, const uint32_t pos, FPtr_destroy func_destroy)
{
    NNode **child = NULL;
    cassert_no_null(node);
    cassert_no_null(node->tree);
    cassert(node->tree->traversing == FALSE);
    cassert_no_null(node->children);
    child = dcast(array_get(node->children, pos), NNode);
    i_destroy_node_ptr(child, func_destroy);
    array_delete(node->children, pos, 1, NULL);
}
