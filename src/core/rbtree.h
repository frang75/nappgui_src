/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: rbtree.h
 * https://nappgui.com/en/core/rbtree.html
 *
 */

/* Red - Black trees */

#include "core.hxx"

__EXTERN_C

RBTree *rbtree_create_imp(FPtr_compare func_compare, const uint16_t esize, const uint16_t ksize);

void rbtree_destroy_imp(RBTree **tree, FPtr_remove func_remove, FPtr_destroy func_destroy_key);

void rbtree_destroy_ptr_imp(RBTree **tree, FPtr_destroy func_destroy, FPtr_destroy func_destroy_key);

uint32_t rbtree_size_imp(const RBTree *tree);

byte_t *rbtree_get_imp(const RBTree *tree, const void *key, const bool_t isptr);

byte_t *rbtree_insert_imp(RBTree *tree, const void *key, FPtr_copy func_key_copy);

bool_t rbtree_insert_ptr_imp(RBTree *tree, void *ptr);

bool_t rbtree_delete_imp(RBTree *tree, const void *key, FPtr_remove func_remove, FPtr_destroy func_destroy_key);

bool_t rbtree_delete_ptr_imp(RBTree *tree, const void *key, FPtr_destroy func_destroy, FPtr_destroy func_destroy_key);

byte_t *rbtree_first_imp(RBTree *tree);

byte_t *rbtree_last_imp(RBTree *tree);

byte_t *rbtree_next_imp(RBTree *tree);

byte_t *rbtree_prev_imp(RBTree *tree);

byte_t *rbtree_first_ptr_imp(RBTree *tree);

byte_t *rbtree_last_ptr_imp(RBTree *tree);

byte_t *rbtree_next_ptr_imp(RBTree *tree);

byte_t *rbtree_prev_ptr_imp(RBTree *tree);

const char_t *rbtree_get_key(const RBTree *tree);

bool_t rbtree_check(const RBTree *tree);

__END_C

