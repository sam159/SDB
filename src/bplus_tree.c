//
// Created by sam on 14/06/18.
//

#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "bplus_tree.h"

#define APPEND_STR(str, size, _) { \
    if (strlen(_) + strlen(str) + 1 > (size)) { \
        (size) *= 2; \
        (str) = realloc(str, (size) * sizeof(char)); \
    } \
    strcat(str, _);\
}

BPlusKV *new_bplus_kv(uint64_t key, void *value, BPlusNode *leftPointer) {
    BPlusKV *kv = malloc(sizeof(BPlusKV));
    kv->key = key;
    kv->value = value;
    kv->rightPointer = leftPointer;
    return kv;
}

void free_bplus_kv(BPlusKV *kv) {
    if (kv->rightPointer != NULL) {
        free_bplus_node(kv->rightPointer);
    }
    if (kv->value != NULL) {
        free(kv->value);
    }
    free(kv);
}

BPlusNode *new_bplus_node(uint64_t id, bool isInternal, size_t order) {
    BPlusNode *node = malloc(sizeof(BPlusNode));
    node->id = id;
    node->isInternal = isInternal;
    node->order = order;
    node->parent = NULL;
    node->keys = calloc(order, sizeof(BPlusKV *));
    node->keyCount = 0;
    node->leftPointer = NULL;
    node->leftLeaf = NULL;
    node->rightLeaf = NULL;
    return node;
}

void free_bplus_node(BPlusNode *node) {
    for (size_t i = 0; i < node->keyCount; i++) {
        free_bplus_kv(node->keys[i]);
    }
    free(node->keys);
    if (node->leftPointer != NULL) {
        free_bplus_node(node->leftPointer);
    }
    free(node);
}

bool bplus_node_insert_kv(BPlusNode *node, BPlusKV *kv) {
    if (node->keyCount == node->order) {
        return false;
    }
    //Insert if empty
    if (node->keyCount == 0) {
        node->keys[0] = kv;
        node->keyCount = 1;
        return true;
    }
    //Check if can add to end
    if (kv->key > node->keys[node->keyCount - 1]->key) {
        node->keys[node->keyCount++] = kv;
        return true;
    }

    //Find an index where we should insert
    size_t i;
    for (i = 0; i < node->keyCount; i++) {
        if (kv->key < node->keys[i]->key) {
            break;
        }
    }
    //Make hole for new value
    for (size_t k = node->order - 1; k > i; k--) {
        node->keys[k] = node->keys[k - 1];
    }
    node->keys[i] = kv;
    node->keyCount++;
    return true;
}

void print_bplus_node(BPlusNode *node, size_t indent) {
    char *indentStr = calloc(indent + 1, sizeof(char));
    memset(indentStr, ' ', sizeof(char) * indent);

    if (node->parent == NULL) {
        printf("%sROOT #%ld (%ld/%ld)\n", indentStr, node->id, node->keyCount, node->order);
    } else if (node->isInternal) {
        printf("%sINTERNAL #%ld (%ld/%ld)\n", indentStr, node->id, node->keyCount, node->order);
    } else {
        printf("%sLEAF #%ld (%ld/%ld)\n", indentStr, node->id, node->keyCount, node->order);
    }
    if (node->parent != NULL) {
        printf("%s  Parent #%ld\n", indentStr, node->parent->id);
    }

    if (node->leftPointer != NULL) {
        print_bplus_node(node->leftPointer, indent + 4);
    }
    for (size_t i = 0; i < node->keyCount; i++) {
        printf("%s  Key: %ld\t", indentStr, node->keys[i]->key);
        if (!node->isInternal) {
            if (node->keys[i]->value != NULL) {
                printf("Value: %p", node->keys[i]->value);
            } else {
                printf("Value: NULL");
            }
        }
        printf("\n");
        if (node->keys[i]->rightPointer != NULL) {
            print_bplus_node(node->keys[i]->rightPointer, indent + 4);
        }
    }

    free(indentStr);
}

char *debug_bplus_node_str(BPlusNode *node, char *str, size_t *strSize) {
    APPEND_STR(str, *strSize, "[");
    if (node->leftPointer != NULL) {
        str = debug_bplus_node_str(node->leftPointer, str, strSize);
    }
    char buffer[32] = {0};
    for (size_t i = 0; i < node->keyCount; i++) {
        snprintf(buffer, 32, node->isInternal ? "{%ld}" : "{%ld*}", node->keys[i]->key);
        APPEND_STR(str, *strSize, buffer);
        if (node->keys[i]->rightPointer != NULL) {
            str = debug_bplus_node_str(node->keys[i]->rightPointer, str, strSize);
        }
    }

    APPEND_STR(str, *strSize, "]");
    return str;
}

BPlusTree *new_bplus_tree(size_t order) {
    BPlusTree *tree = malloc(sizeof(BPlusTree));
    tree->order = order;
    tree->minFill = order / 2;
    tree->nextID = 0;
    tree->root = new_bplus_node(tree->nextID++, false, order);
    return tree;
}

void free_bplus_tree(BPlusTree *tree) {
    if (tree->root != NULL) {
        free_bplus_node(tree->root);
    }
    free(tree);
}

void print_bplus_tree(BPlusTree *tree) {
    printf("B+ Tree\n");
    printf("- Order: %ld\n", tree->order);
    printf("- Min Fill: %ld\n", tree->minFill);

    if (tree->root != NULL) {
        print_bplus_node(tree->root, 2);
    } else {
        printf("  EMPTY TREE\n");
    }
}

char *debug_bplus_tree_str(BPlusTree *tree, char *str) {
    size_t strSize = 32;
    str = realloc(str, strSize * sizeof(char));
    memset(str, 0, strSize * sizeof(char));

    snprintf(str, 32, "B+<%ld>", tree->order);

    str = debug_bplus_node_str(tree->root, str, &strSize);

    return str;
}

BPlusNode *bplus_tree_find_leaf(BPlusTree *tree, uint64_t key) {
    BPlusNode *node = tree->root;

    //Descend to correct leaf node
    while (node != NULL) {
        //Found our way to a leaf node
        if (!node->isInternal) {
            break;
        }
        //We are assuming at least one key
        if (key < node->keys[0]->key) {
            //Follow left pointer if less than first key
            node = node->leftPointer;
            continue;
        }
        size_t i;
        for (i = 1; i < node->keyCount; i++) {
            if (key < node->keys[i]->key) {
                break;
            }
        }
        node = node->keys[i - 1]->rightPointer;
    }
    return node;
}

bool bplus_tree_insert(BPlusTree *tree, uint64_t newKey, void *newValue) {
    BPlusNode *node = bplus_tree_find_leaf(tree, newKey);
    assert(node != NULL);

    if (node->keyCount < node->order - 1) {
        //Can insert at this node
        BPlusKV *kv = new_bplus_kv(newKey, newValue, NULL);
        bool insertKV = bplus_node_insert_kv(node, kv);
        assert(insertKV == true);
    } else {
        //Note: as this is a leaf node, we are not updating/maintaining any pointers
        //Insert and split
        BPlusKV *kv = new_bplus_kv(newKey, newValue, NULL);
        bool insertKV = bplus_node_insert_kv(node, kv);
        assert(insertKV == true);

        //Decide on new midpoint
        size_t midpointIndex = node->order / 2;

        //Create new node and update leaf links
        BPlusNode *newNode = new_bplus_node(tree->nextID++, false, node->order);
        newNode->parent = node->parent;
        newNode->rightLeaf = node->rightLeaf;
        newNode->leftLeaf = node;
        node->rightLeaf = newNode;

        //Move keys >= midpoint to a new node
        size_t k = 0;
        for (size_t i = midpointIndex; i < node->order; i++) {
            newNode->keys[k++] = node->keys[i];
            node->keys[i] = NULL;
            newNode->keyCount++;
            node->keyCount--;
        }

        //Send midpoint key to parent
        BPlusKV *ascendingKV = new_bplus_kv(newNode->keys[0]->key, NULL, NULL);
        ascendingKV->rightPointer = newNode;

        do {
            if (node->parent == NULL) {
                //Split root, create a new node with the ascending kv and replace it
                BPlusNode *newRoot = new_bplus_node(tree->nextID++, true, tree->order);
                newRoot->leftPointer = node;
                bplus_node_insert_kv(newRoot, ascendingKV);
                tree->root = newRoot;
                newNode->parent = tree->root;
                node->parent = tree->root;
                break;
            } else {
                node = node->parent;
                bplus_node_insert_kv(node, ascendingKV);
                if (node->keyCount < node->order) {
                    break;
                } else {
                    //Parent needs splitting
                    midpointIndex = node->order / 2;
                    //Create new node
                    newNode = new_bplus_node(tree->nextID++, true, node->order);
                    newNode->parent = node->parent;
                    //Move midpoint up and move keys > midpoint to new node
                    k = 0;
                    for (size_t i = midpointIndex + 1; i < node->order; i++) {
                        node->keys[i]->rightPointer->parent = newNode;
                        newNode->keys[k++] = node->keys[i];
                        node->keys[i] = NULL;
                        newNode->keyCount++;
                        node->keyCount--;
                    }
                    ascendingKV = node->keys[midpointIndex];
                    node->keys[midpointIndex] = NULL;
                    node->keyCount--;
                    newNode->leftPointer = ascendingKV->rightPointer;
                    newNode->leftPointer->parent = newNode;
                    ascendingKV->rightPointer = newNode;
                }
            }
        } while (node != NULL);

    }

}

BPlusKV *bplus_tree_find(BPlusTree *tree, uint64_t key) {
    BPlusNode *leaf = bplus_tree_find_leaf(tree, key);
    if (leaf == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < leaf->keyCount; i++) {
        if (leaf->keys[i]->key == key) {
            return leaf->keys[i];
        }
    }
}

BPlusKV *bplus_tree_find_closest(BPlusTree *tree, uint64_t key, BPlusFindComp dir) {
    //Do regular find if we are only looking for key
    if (dir == FIND_EQ) {
        return bplus_tree_find(tree, key);
    }
    BPlusNode *leaf = bplus_tree_find_leaf(tree, key);
    if (leaf == NULL) {
        return NULL;
    }
    //Scan right until we find a value >= key
    ssize_t i;
    do {
        bool done = false;
        for (i = 0; i < leaf->keyCount; i++) {
            if (leaf->keys[i]->key == key && (dir & FIND_EQ) == FIND_EQ) {
                //Return the value if we are looking for value = key
                done = true;
                break;
            } else if (leaf->keys[i]->key > key) {
                //Return the value if we are looking for value > key
                if ((dir & FIND_GT) == FIND_GT) {
                    return leaf->keys[i];
                } else {
                    done = true;
                    break;
                }
            }
        }
        if (done) {
            break;
        }
        if (leaf->rightLeaf == NULL) {
            break;
        }
        leaf = leaf->rightLeaf;
    } while (leaf != NULL);

    //Didn't find an acceptable value >= key and we are not looking for value < key
    if ((dir & FIND_LT) == 0) {
        return NULL;
    }

    //Scan left until we find a value < key
    do {
        for (i = leaf->keyCount - 1; i >= 0; i--) {
            if (leaf->keys[i]->key < key) {
                return leaf->keys[i];
            }
        }
        leaf = leaf->leftLeaf;
    } while (leaf != NULL);

    return NULL;
}