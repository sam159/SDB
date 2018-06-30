//
// Created by sam on 30/06/18.
//

BPlusNode *_bplus_node_get_left_sibling(BPlusNode *node) {
    BPlusNode *parent = node->parent;
    if (node->isInternal) {
        if (parent == NULL) {
            return NULL;
        }
        if (parent->leftLeaf == node) {
            return NULL;
        }
        for (size_t i = 0; i < parent->keyCount; i++) {
            if (parent->keys[i]->rightPointer == node) {
                if (i == 0) {
                    return parent->leftPointer;
                }
                if (i + 1 < parent->keyCount) {
                    return parent->keys[i + 1]->rightPointer;
                }
                return NULL;
            }
        }
        return NULL;
    } else {
        return node->leftLeaf;
    }
}

BPlusNode *_bplus_node_get_right_sibling(BPlusNode *node) {
    BPlusNode *parent = node->parent;
    if (node->isInternal) {
        if (parent == NULL) {
            return NULL;
        }
        if (parent->leftPointer == node) {
            return parent->keyCount > 0 ? parent->keys[0]->rightPointer : NULL;
        }
        for (size_t i = 0; i < parent->keyCount; i++) {
            if (parent->keys[i]->rightPointer == node) {
                if (i == 0) {
                    return parent->leftPointer;
                }
                return parent->keys[i - 1]->rightPointer;
            }
        }
        return NULL;
    } else {
        return node->rightLeaf;
    }
}
