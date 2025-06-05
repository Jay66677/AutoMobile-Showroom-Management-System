#include "b+treetemplate.h"

//helper functions designed for inserting node into B-tree

// Create a new node leaf or internal
BTreeNode* createNode(int is_leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) return NULL;
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    if (is_leaf) {
        node->leaf_link.next = NULL;
        node->leaf_link.prev = NULL;
    }
    return node;
}



// Find insert position in node
int findInsertPos(BTreeNode* node, void* key, CompareFunc compare) {
    int i = 0;
    while (i < node->num_keys && compare(node->keys[i].key, key) <= 0) i++;
    return i;
}

// Split leaf node
void splitLeaf(BTreeNode* leaf, BTreeNode** new_leaf, void** promoted_key, BPlusTree* tree) {
    int mid = MAX / 2;
    *new_leaf = createNode(1);
    for (int i = mid, j = 0; i < MAX; i++, j++) {
        (*new_leaf)->keys[j].key = tree->clone(leaf->keys[i].key);
        (*new_leaf)->num_keys++;
    }
    leaf->num_keys = mid;

    (*new_leaf)->leaf_link.next = leaf->leaf_link.next;
    (*new_leaf)->leaf_link.prev = leaf;
    if (leaf->leaf_link.next)
        leaf->leaf_link.next->leaf_link.prev = *new_leaf;
    leaf->leaf_link.next = *new_leaf;

    *promoted_key = tree->clone((*new_leaf)->keys[0].key);
}

// Split internal node
void splitInternal(BTreeNode* node, BTreeNode** new_node, void** promoted_key, BPlusTree* tree) {
    int mid = MAX / 2;
    *new_node = createNode(0);

    *promoted_key = tree->clone(node->keys[mid].key);

    for (int i = mid + 1, j = 0; i < MAX; i++, j++) {
        (*new_node)->keys[j].key = tree->clone(node->keys[i].key);
        (*new_node)->children[j] = node->children[i];
        (*new_node)->num_keys++;
    }
    (*new_node)->children[(*new_node)->num_keys] = node->children[MAX];
    node->num_keys = mid;
}




// Create B+ tree
BPlusTree* createBPlusTree(CompareFunc cmp, PrintFunc print, CloneFunc clone, FreeFunc free_key){
    BPlusTree* tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    if (!tree) return NULL;
    tree->root = NULL;
    tree->compare = cmp;
    tree->clone = clone;
    tree->free_func = free_key;
    tree->print = print;
    return tree;
}

// Print keys in leaf level
void printBPlusTree(BPlusTree* tree) {
    if (!tree || !tree->root) return;

    BTreeNode* node = tree->root;
    while (!node->is_leaf)
        node = node->children[0];

    while (node) {
        for (int i = 0; i < node->num_keys; i++) {
            tree->print(node->keys[i].key);
            printf(" ");
        }
        node = node->leaf_link.next;
    }
    printf("\n");
}

// Recursive insert logic
BTreeNode* insertRecursive(BTreeNode* node, void* key, void** promoted_key, BPlusTree* tree, int* grew) {
    int pos = findInsertPos(node, key, tree->compare);

    if (node->is_leaf) {
        for (int i = node->num_keys; i > pos; i--) {
            node->keys[i].key = node->keys[i - 1].key;
        }
        node->keys[pos].key = tree->clone(key);
        node->num_keys++;

        if (node->num_keys < MAX) {
            *grew = 0;
            return NULL;
        }

        BTreeNode* new_leaf = NULL;
        splitLeaf(node, &new_leaf, promoted_key, tree);
        *grew = 1;
        return new_leaf;
    }

    void* child_promoted = NULL;
    int child_grew = 0;
    BTreeNode* child = insertRecursive(node->children[pos], key, &child_promoted, tree, &child_grew);

    if (!child_grew) {
        *grew = 0;
        return NULL;
    }

    for (int i = node->num_keys; i > pos; i--) {
        node->keys[i].key = node->keys[i - 1].key;
        node->children[i + 1] = node->children[i];
    }
    node->keys[pos].key = child_promoted;
    node->children[pos + 1] = child;
    node->num_keys++;

    if (node->num_keys < MAX) {
        *grew = 0;
        return NULL;
    }

    BTreeNode* new_internal = NULL;
    splitInternal(node, &new_internal, promoted_key, tree);
    *grew = 1;
    return new_internal;
}

// general to insert into B+ Tree
void bplusInsert(BPlusTree* tree, void* key) {
    if (tree->root == NULL) {
        tree->root = createNode(1);
        tree->root->keys[0].key = tree->clone(key);
        tree->root->num_keys = 1;
        return;
    }

    void* promoted_key = NULL;
    int grew = 0;

    BTreeNode* new_node = insertRecursive(tree->root, key, &promoted_key, tree, &grew);

    if (grew) {
        BTreeNode* new_root = createNode(0);
        new_root->keys[0].key = promoted_key;
        new_root->children[0] = tree->root;
        new_root->children[1] = new_node;
        new_root->num_keys = 1;
        tree->root = new_root;
    }
}



// Search for a key in the B+ Tree
void* bplusSearch(BPlusTree* tree, void* key) {
    BTreeNode* current = tree->root;
    if (!current) return NULL;

    while (!current->is_leaf) {
        int pos = findInsertPos(current, key, tree->compare);
        current = current->children[pos];
    }

    for (int i = 0; i < current->num_keys; i++) {
        if (tree->compare(current->keys[i].key, key) == 0) {
            return current->keys[i].key;
        }
    }
    return NULL;
}



// Free a B+ Tree node recursively
void freeNode(BTreeNode* node, BPlusTree* tree) {
    if (!node) return;

    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            freeNode(node->children[i], tree);
        }
    }

    for (int i = 0; i < node->num_keys; i++) {
        tree->free_func(node->keys[i].key);
    }

    free(node);
}


// Free the entire B+ Tree
void freeBPlusTree(BPlusTree* tree) {
    if (!tree) return;
    freeNode(tree->root, tree);
    free(tree);
}


//deletion part from here

// Find the predecessor key (rightmost key in the left subtree)
void* findPredecessor(BTreeNode* node, int idx, BPlusTree* tree) {
    BTreeNode* current = node->children[idx];
    while (!current->is_leaf) {
        current = current->children[current->num_keys];
    }
    return tree->clone(current->keys[current->num_keys - 1].key);
}

// Find the successor key (leftmost key in the right subtree)
void* findSuccessor(BTreeNode* node, int idx, BPlusTree* tree) {
    BTreeNode* current = node->children[idx + 1];
    while (!current->is_leaf) {
        current = current->children[0];
    }
    return tree->clone(current->keys[0].key);
}

// Merge two nodes (used when a node has too few keys)
void mergeNodes(BTreeNode* left, BTreeNode* right, int parent_idx, BTreeNode* parent, BPlusTree* tree) {
    // For internal nodes, get the key from the parent
    if (!left->is_leaf) {
        left->keys[left->num_keys].key = parent->keys[parent_idx].key;
        left->num_keys++;
    }

    // Copy keys and children from right to left
    for (int i = 0; i < right->num_keys; i++) {
        left->keys[left->num_keys].key = tree->clone(right->keys[i].key);
        if (!left->is_leaf) {
            left->children[left->num_keys] = right->children[i];
        }
        left->num_keys++;
    }

    // If internal node, copy the last child too
    if (!left->is_leaf) {
        left->children[left->num_keys] = right->children[right->num_keys];
    } else {
        // If leaf node, update the linked list
        left->leaf_link.next = right->leaf_link.next;
        if (right->leaf_link.next) {
            right->leaf_link.next->leaf_link.prev = left;
        }
    }

    // Remove the parent key and update child pointers
    for (int i = parent_idx; i < parent->num_keys - 1; i++) {
        parent->keys[i].key = parent->keys[i + 1].key;
        parent->children[i + 1] = parent->children[i + 2];
    }
    parent->num_keys--;

    // Free the right node
    for (int i = 0; i < right->num_keys; i++) {
        tree->free_func(right->keys[i].key);
    }
    free(right);
}

// Redistribute keys among siblings (used to avoid merging when possible)
void redistributeKeys(BTreeNode* left, BTreeNode* right, int parent_idx, BTreeNode* parent, int direction, BPlusTree* tree) {
    // direction: 0 = move from right to left, 1 = move from left to right

    if (direction == 0) {
        // Move a key from right to left
        if (!left->is_leaf) {
            // For internal nodes, move through parent
            left->keys[left->num_keys].key = parent->keys[parent_idx].key;
            left->children[left->num_keys + 1] = right->children[0];
            parent->keys[parent_idx].key = tree->clone(right->keys[0].key);
            
            // Shift keys and children in right node
            for (int i = 0; i < right->num_keys - 1; i++) {
                right->keys[i].key = right->keys[i + 1].key;
                right->children[i] = right->children[i + 1];
            }
            right->children[right->num_keys - 1] = right->children[right->num_keys];
        } else {
            // For leaf nodes, copy directly
            left->keys[left->num_keys].key = tree->clone(right->keys[0].key);
            
            // Update parent key
            parent->keys[parent_idx].key = tree->clone(right->keys[1].key);
            
            // Shift keys in right node
            for (int i = 0; i < right->num_keys - 1; i++) {
                right->keys[i].key = right->keys[i + 1].key;
            }
        }
        
        left->num_keys++;
        right->num_keys--;
    } else {
        // Move a key from left to right
        // Shift keys and children in right node
        for (int i = right->num_keys; i > 0; i--) {
            right->keys[i].key = right->keys[i - 1].key;
            if (!left->is_leaf) {
                right->children[i + 1] = right->children[i];
            }
        }
        if (!left->is_leaf) {
            right->children[1] = right->children[0];
        }
        
        if (!left->is_leaf) {
            // For internal nodes, move through parent
            right->keys[0].key = parent->keys[parent_idx].key;
            right->children[0] = left->children[left->num_keys];
            parent->keys[parent_idx].key = tree->clone(left->keys[left->num_keys - 1].key);
        } else {
            // For leaf nodes, copy directly
            right->keys[0].key = tree->clone(left->keys[left->num_keys - 1].key);
            
            // Update parent key (only needed for leaf nodes)
            parent->keys[parent_idx].key = tree->clone(right->keys[0].key);
        }
        
        right->num_keys++;
        left->num_keys--;
    }
}

// Check if a node needs rebalancing (too few keys)
int needsRebalancing(BTreeNode* node) {
    return node->num_keys < (MAX / 2);
}

// Rebalance tree by merging or redistributing keys
void rebalanceTree(BTreeNode* node, BTreeNode* parent, int child_idx, BPlusTree* tree) {
    // If child_idx is -1, node is the root, handle differently
    if (child_idx == -1) {
        if (node->num_keys == 0 && !node->is_leaf) {
            BTreeNode* new_root = node->children[0];
            tree->root = new_root;
            free(node);
        }
        return;
    }
    
    // Try to borrow from left sibling
    if (child_idx > 0) {
        BTreeNode* left_sibling = parent->children[child_idx - 1];
        if (left_sibling->num_keys > (MAX / 2)) {
            redistributeKeys(left_sibling, node, child_idx - 1, parent, 0, tree);
            return;
        }
    }
    
    // Try to borrow from right sibling
    if (child_idx < parent->num_keys) {
        BTreeNode* right_sibling = parent->children[child_idx + 1];
        if (right_sibling->num_keys > (MAX / 2)) {
            redistributeKeys(node, right_sibling, child_idx, parent, 1, tree);
            return;
        }
    }
    
    // If borrowing isn't possible, merge with a sibling
    if (child_idx > 0) {
        // Merge with left sibling
        mergeNodes(parent->children[child_idx - 1], node, child_idx - 1, parent, tree);
    } else {
        // Merge with right sibling
        mergeNodes(node, parent->children[child_idx + 1], child_idx, parent, tree);
    }
}

// Helper function to remove a key from a leaf node
int removeFromLeaf(BTreeNode* leaf, void* key, BPlusTree* tree) {
    int idx = 0;
    while (idx < leaf->num_keys && tree->compare(leaf->keys[idx].key, key) != 0) {
        idx++;
    }
    
    if (idx == leaf->num_keys) {
        // Key not found
        return 0;
    }
    
    // Free the key
    tree->free_func(leaf->keys[idx].key);
    
    // Shift keys to fill the gap
    for (int i = idx; i < leaf->num_keys - 1; i++) {
        leaf->keys[i].key = leaf->keys[i + 1].key;
    }
    
    leaf->num_keys--;
    return 1;
}

// Helper function for removing a key from an internal node
void removeFromInternal(BTreeNode* node, int idx, BPlusTree* tree) {
    // Get a replacement key (either predecessor or successor)
    void* replacement;
    if (node->children[idx]->num_keys >= node->children[idx + 1]->num_keys) {
        replacement = findPredecessor(node, idx, tree);
    } else {
        replacement = findSuccessor(node, idx, tree);
    }
    
    // Free the current key and replace it
    tree->free_func(node->keys[idx].key);
    node->keys[idx].key = replacement;
}

// Recursive delete function
int deleteRecursive(BTreeNode* node, void* key, BTreeNode* parent, int child_idx, BPlusTree* tree) {
    int key_idx, result;
    
    // Find the position where the key might be
    key_idx = 0;
    while (key_idx < node->num_keys && tree->compare(node->keys[key_idx].key, key) < 0) {
        key_idx++;
    }
    
    if (node->is_leaf) {
        // If this is a leaf node, directly remove the key
        result = removeFromLeaf(node, key, tree);
        
        // Rebalance if needed
        if (result && needsRebalancing(node)) {
            rebalanceTree(node, parent, child_idx, tree);
        }
        
        return result;
    } else {
        // If this is an internal node
        if (key_idx < node->num_keys && tree->compare(node->keys[key_idx].key, key) == 0) {
            // The key is in this internal node
            // Replace it with predecessor or successor and delete from there
            BTreeNode* left_child = node->children[key_idx];
            BTreeNode* right_child = node->children[key_idx + 1];
            
            if (left_child->num_keys >= (MAX / 2) + 1) {
                // If left child has enough keys, replace with predecessor
                void* pred = findPredecessor(node, key_idx, tree);
                tree->free_func(node->keys[key_idx].key);
                node->keys[key_idx].key = pred;
                
                // Now delete the predecessor key from the left subtree
                return deleteRecursive(left_child, pred, node, key_idx, tree);
            } else if (right_child->num_keys >= (MAX / 2) + 1) {
                // If right child has enough keys, replace with successor
                void* succ = findSuccessor(node, key_idx, tree);
                tree->free_func(node->keys[key_idx].key);
                node->keys[key_idx].key = succ;
                
                // Now delete the successor key from the right subtree
                return deleteRecursive(right_child, succ, node, key_idx + 1, tree);
            } else {
                // Neither child has enough keys, merge them
                mergeNodes(left_child, right_child, key_idx, node, tree);
                
                // Now delete the key from the merged node
                if (parent && needsRebalancing(node)) {
                    rebalanceTree(node, parent, child_idx, tree);
                }
                
                return deleteRecursive(left_child, key, node, key_idx, tree);
            }
        } else {
            // Recurse into appropriate child
            int next_idx = (key_idx < node->num_keys && 
                          tree->compare(key, node->keys[key_idx].key) >= 0) 
                          ? key_idx + 1 : key_idx;
            
            result = deleteRecursive(node->children[next_idx], key, node, next_idx, tree);
            
            // Check if the child needs rebalancing
            if (needsRebalancing(node->children[next_idx])) {
                rebalanceTree(node->children[next_idx], node, next_idx, tree);
            }
            
            return result;
        }
    }
}

// general call to delete a key from the B+ tree
int bplusDelete(BPlusTree* tree, void* key) {
    if (!tree || !tree->root) {
        return 0; // Tree is empty
    }
    
    int result = deleteRecursive(tree->root, key, NULL, -1, tree);
    
    // If root only has one child, make that child the new root
    if (!tree->root->is_leaf && tree->root->num_keys == 0) {
        BTreeNode* old_root = tree->root;
        tree->root = tree->root->children[0];
        free(old_root);
    }
    
    return result;
}


//range search 


// Range search function that processes all keys in range using a callback
void bplusRangeSearch(BPlusTree* tree, void* lower, void* upper, ProcessKeyFunc process, void* user_data) {
    if (!tree || !tree->root) {
        return;
    }
    
    // Find the leaf node that might contain the lower bound
    BTreeNode* current = tree->root;
    while (!current->is_leaf) {
        int pos = findInsertPos(current, lower, tree->compare);
        current = current->children[pos];
    }
    
    // Traverse leaf nodes, processing keys in range
    while (current) {
        for (int i = 0; i < current->num_keys; i++) {
            // Skip keys less than lower bound
            if (tree->compare(current->keys[i].key, lower) < 0) continue;
            
            // Stop if we reached the upper bound
            if (tree->compare(current->keys[i].key, upper) > 0) {
                return;
            }
            
            // Process this key
            process(current->keys[i].key, user_data);
        }
        
        // Move to next leaf
        current = current->leaf_link.next;
    }
}

