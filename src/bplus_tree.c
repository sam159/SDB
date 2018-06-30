//
// Created by sam on 14/06/18.
//

#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "bplus_tree.h"

#include "bplus_tree.helpers.c"

#define APPEND_STR(str, size, _) { \
    if (strlen(_) + strlen(str) + 1 > (size)) { \
        (size) *= 2; \
        (str) = realloc(str, (size) * sizeof(char)); \
    } \
    strcat(str, _);\
}

BPlusKV *new_bplus_kv(uint64_t key, void *value, BPlusNode *rightPointer) {
    BPlusKV *kv = malloc(sizeof(BPlusKV));
    kv->key = key;
    kv->value = value;
    kv->rightPointer = rightPointer;
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

bool bplus_node_remove_kv(BPlusNode *node, BPlusKV *kv) {
    bool removed = false;
    //Find key
    for (size_t i = 0; i < node->keyCount; i++) {
        if (node->keys[i] == kv) {
            node->keys[i] = NULL;
            node->keyCount--;
            //Move keys down
            for (; i < node->keyCount; i++) {
                node->keys[i] = node->keys[i + 1];
            }
            removed = true;
            break;
        }
    }
    return removed;
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
    assert(node != NULL && node->isInternal == false);

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

bool bplus_tree_delete(BPlusTree *tree, uint64_t key) {
    BPlusNode *leaf = bplus_tree_find_leaf(tree, key);
    assert(leaf != NULL && leaf->isInternal == false);

    //Find and remove kv
    bool removed = false;
    for (size_t i = 0; i < leaf->keyCount; i++) {
        if (leaf->keys[i]->key == key) {
            BPlusKV *kv = leaf->keys[i];
            removed = bplus_node_remove_kv(leaf, leaf->keys[i]);
            assert(removed);
            free_bplus_kv(kv);
            break;
        }
    }

    if (!removed) {
        return false;
    }

    if (leaf->keyCount >= tree->minFill || leaf->parent == NULL) {
        return true;
    }

    //Try and borrow a key from a neighbour

    //Check right leaf
    if (leaf->rightLeaf != NULL
        && leaf->rightLeaf->parent == leaf->parent
        && leaf->rightLeaf->keyCount > tree->minFill) {
        //Move first kv on leaf to this leaf
        BPlusKV *kv = leaf->rightLeaf->keys[0];
        assert(bplus_node_remove_kv(leaf->rightLeaf, kv));
        assert(bplus_node_insert_kv(leaf, kv));
        //Update the right leafs parent pointer
        for (size_t i = 0; i < leaf->parent->keyCount; i++) {
            if (leaf->parent->keys[i]->rightPointer == leaf->rightLeaf) {
                leaf->parent->keys[i]->key = leaf->rightLeaf->keys[0]->key;
                break;
            }
        }
        return true;
    }

    //Check left leaf
    if (leaf->leftLeaf != NULL
        && leaf->leftLeaf->parent == leaf->parent
        && leaf->leftLeaf->keyCount > tree->minFill) {
        //Move right most key from left leaf
        BPlusKV *kv = leaf->leftLeaf->keys[leaf->keyCount - 1];
        assert(bplus_node_remove_kv(leaf->leftLeaf, kv));
        assert(bplus_node_insert_kv(leaf, kv));
        //Update the key pointing to this leaf
        for (size_t i = 0; i < leaf->parent->keyCount; i++) {
            if (leaf->parent->keys[i]->rightPointer == leaf) {
                leaf->parent->keys[i]->key = leaf->keys[0]->key;
                break;
            }
        }
    }

    //No key to borrow, merge with a sibling
    //Look right and left for leafs with same parent and where merging would result in keys < order

    bool merged = false;
    //Try right leaf
    if (leaf->rightLeaf != NULL
        && leaf->parent == leaf->rightLeaf->parent
        && leaf->keyCount + leaf->rightLeaf->keyCount < tree->order) {
        //Move all keys from right leaf to this leaf
        while (leaf->rightLeaf->keyCount > 0) {
            assert(bplus_node_insert_kv(leaf, leaf->rightLeaf->keys[0]));
            assert(bplus_node_remove_kv(leaf->rightLeaf, leaf->rightLeaf->keys[0]));
        }
        //Remove right leaf and parent pointer
        for (size_t i = 0; i < leaf->parent->keyCount; i++) {
            BPlusKV *kv = leaf->parent->keys[i];
            if (kv->rightPointer == leaf->rightLeaf) {
                //Remove key
                bplus_node_remove_kv(leaf->parent, kv);
                //Update leaf pointers
                leaf->rightLeaf = leaf->rightLeaf;
                if (leaf->rightLeaf != NULL) {
                    leaf->rightLeaf->leftLeaf = leaf;
                }
                //Free key and leaf
                free_bplus_kv(kv);
                break;
            }
        }
        merged = true;
    }

    //Try left leaf
    if (leaf->leftLeaf != NULL
        && leaf->parent == leaf->leftLeaf->parent
        && leaf->keyCount + leaf->leftLeaf->keyCount < tree->order) {
        //Move all keys into left leaf
        while (leaf->keyCount > 0) {
            assert(bplus_node_insert_kv(leaf->leftLeaf, leaf->keys[0]));
            assert(bplus_node_remove_kv(leaf, leaf->keys[0]));
        }
        //Remove this leaf and parent pointer (if any)
        for (size_t i = 0; i < leaf->parent->keyCount; i++) {
            BPlusKV *kv = leaf->parent->keys[i];
            if (kv->rightPointer == leaf) {
                //Remove key
                bplus_node_remove_kv(leaf->parent, kv);
                //Update leaf pointers
                leaf->leftLeaf->rightLeaf = leaf->rightLeaf;
                if (leaf->leftLeaf->rightLeaf != NULL) {
                    leaf->leftLeaf->rightLeaf->leftLeaf = leaf->leftLeaf;
                }
                leaf = leaf->leftLeaf;
                //Free key and leaf
                free_bplus_kv(kv);
                break;
            }
        }
        merged = true;
    }

    //Must have merged left or right leaf by this point
    assert(merged == true);

    //Start merging parents where key count <= 1
    BPlusNode *node = leaf->parent;

    while (node != NULL) {

        //Dont need to do anything if node has enough keys or is the root
        if (node->keyCount > 1 || node->parent == NULL) {
            break;
        }
        BPlusNode *left = _bplus_node_get_left_sibling(node);
        BPlusNode *right = _bplus_node_get_right_sibling(node);

        //Try and borrow a key from a sibling

        if (right != NULL && right->keyCount > 1) {
            //Take the left pointer and create a new key for it
            BPlusKV *kv = new_bplus_kv(right->leftPointer->keys[0]->key, NULL, right->leftPointer);
            right->leftPointer->parent = node;
            bplus_node_insert_kv(node, kv);
            //Consume first key to populate left pointer
            right->leftPointer = right->keys[0]->rightPointer;
            kv = right->keys[0];
            bplus_node_remove_kv(right, kv);
            kv->rightPointer = NULL;
            free_bplus_kv(kv);
            //Update pointer key to right
            for (size_t i = 0; i < node->parent->keyCount; i++) {
                if (node->parent->keys[i]->rightPointer == right) {
                    node->parent->keys[i]->key = right->keys[0]->key;
                }
            }
            break;
        }

        if (left != NULL && left->keyCount > 1) {
            //Push our left pointer to a new key
            BPlusKV *newKV = new_bplus_kv(node->leftPointer->keys[0]->key, NULL, node->leftPointer);
            bplus_node_insert_kv(node, newKV);
            //Take the right key from the left sibling
            BPlusKV *kv = left->keys[left->keyCount - 1];
            bplus_node_remove_kv(left, kv);
            node->leftPointer = kv->rightPointer;
            node->leftPointer->parent = node;
            kv->rightPointer = NULL;
            free_bplus_kv(kv);
            //Update our parent pointer
            for (size_t i = 0; i < node->parent->keyCount; i++) {
                if (node->parent->keys[i]->rightPointer == node) {
                    node->parent->keys[i]->key = node->keys[0]->key;
                }
            }
            break;
        }

        //Couldn't borrow key, merge with a sibling

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