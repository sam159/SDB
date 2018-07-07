//
// Created by sam on 23/06/18.
//

#include <stdlib.h>
#include "bplus_tree_test.h"
#include "../src/bplus_tree.h"

START_TEST(insert_keys_4)
    {

        BPlusTree *tree = new_bplus_tree(4);

        char *buffer = NULL;

        ck_assert_str_eq(debug_bplus_tree_str(tree, buffer), "B+<4> [] ");

        bplus_tree_insert(tree, 10, NULL);
        bplus_tree_insert(tree, 20, NULL);

        ck_assert_str_eq(debug_bplus_tree_str(tree, buffer), "B+<4> [{10*}{20*}] ");

        bplus_tree_insert(tree, 30, NULL);
        bplus_tree_insert(tree, 40, NULL);

        ck_assert_str_eq(debug_bplus_tree_str(tree, buffer), "B+<4> [ [{10*}{20*}] {30} [{30*}{40*}] ] ");

        bplus_tree_insert(tree, 35, NULL);
        bplus_tree_insert(tree, 50, NULL);
        ck_assert_str_eq(debug_bplus_tree_str(tree, buffer), "B+<4> [ [{10*}{20*}] {30} [{30*}{35*}] {40} [{40*}{50*}] ] ");

        free_bplus_tree(tree);

        if (buffer != NULL) free(buffer);

    }END_TEST

START_TEST(find_keys_4)
    {
        BPlusTree *tree = new_bplus_tree(4);

        bplus_tree_insert(tree, 10, NULL);
        bplus_tree_insert(tree, 20, NULL);
        bplus_tree_insert(tree, 30, NULL);
        bplus_tree_insert(tree, 40, NULL);

        ck_assert(bplus_tree_find(tree, 10) != NULL);
        ck_assert(bplus_tree_find(tree, 40) != NULL);

        BPlusKV *kv;
        kv = bplus_tree_find_closest(tree, 15, FIND_LT | FIND_EQ);
        ck_assert_ptr_ne(kv, NULL);
        ck_assert_int_eq(kv->key, 10);

        kv = bplus_tree_find_closest(tree, 30, FIND_LT);
        ck_assert_ptr_ne(kv, NULL);
        ck_assert_int_eq(kv->key, 20);

        kv = bplus_tree_find_closest(tree, 30, FIND_GT);
        ck_assert_ptr_ne(kv, NULL);
        ck_assert_int_eq(kv->key, 40);

        free_bplus_tree(tree);
    }END_TEST

START_TEST(delete_key_leaf) {
        BPlusTree *tree = new_bplus_tree(4);
        for(uint64_t i=1;i<=16;i++) {
            ck_assert(bplus_tree_insert(tree, i, NULL) == true);
        }
        for(uint64_t i=1;i<=16;i++) {
            ck_assert(bplus_tree_delete(tree, i) == true);
        }
        free_bplus_tree(tree);
}END_TEST;

Suite *blus_tree_suite(void) {
    Suite *s = suite_create("B+ Tree");

    TCase *core = tcase_create("Insert");
    suite_add_tcase(s, core);

    tcase_add_test(core, insert_keys_4);
    tcase_add_test(core, find_keys_4);
    tcase_add_test(core, delete_key_leaf);

    return s;
}