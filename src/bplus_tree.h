//
// Created by sam on 14/06/18.
//

#ifndef SDB_BPLUS_TREE_H
#define SDB_BPLUS_TREE_H

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

struct BPlusNode_t;
typedef struct BPlusNode_t BPlusNode;

struct BPlusKV_t {
    uint64_t key;
    void *value;
    BPlusNode *rightPointer;
};
typedef struct BPlusKV_t BPlusKV;

BPlusKV *new_bplus_kv(uint64_t key, void *value, BPlusNode *leftPointer);

void free_bplus_kv(BPlusKV *kv);

struct BPlusNode_t {
    uint64_t id;
    bool isInternal;
    size_t order;
    BPlusNode *parent;
    BPlusKV **keys;
    size_t keyCount;
    BPlusNode *leftPointer;
    BPlusNode *leftLeaf;
    BPlusNode *rightLeaf;
};

BPlusNode *new_bplus_node(uint64_t id, bool isInternal, size_t order);

void free_bplus_node(BPlusNode *node);

bool bplus_node_insert_kv(BPlusNode *node, BPlusKV *kv);

void print_bplus_node(BPlusNode *node, size_t indent);

struct BPlusTree_t {
    size_t order;
    size_t minFill;
    BPlusNode *root;
    uint64_t nextID;
};
typedef struct BPlusTree_t BPlusTree;

BPlusTree *new_bplus_tree(size_t order);

void free_bplus_tree(BPlusTree *tree);

void print_bplus_tree(BPlusTree *tree);

BPlusNode * bplus_tree_find_leaf(BPlusTree *tree, uint64_t key);

bool bplus_tree_insert(BPlusTree *tree, uint64_t key, void *value);


#endif //SDB_BPLUS_TREE_H
