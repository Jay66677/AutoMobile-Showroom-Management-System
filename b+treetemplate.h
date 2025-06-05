#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<math.h>

#define MAX 4  // Max keys per node

typedef struct BTreeNode BTreeNode;
typedef struct BPlusTree BPlusTree;

// Function pointers for general key handling
typedef int (*CompareFunc)(const void*, const void*);
typedef void (*PrintFunc)(const void*);
typedef void* (*CloneFunc)(const void*);
typedef void (*FreeFunc)(void*);

// Doubly linked list for leaves
typedef struct LeafLink {
    struct BTreeNode* next;
    struct BTreeNode* prev;
} LeafLink;

// Key wrapper to store  pointers
typedef struct {
    void* key;
} KeyWrapper;

struct BTreeNode {
    int is_leaf;
    int num_keys;
    KeyWrapper keys[MAX];
    union {
        BTreeNode* children[MAX + 1]; // For internal nodes
        LeafLink leaf_link;           // For leaf nodes
    };
};

// B+ Tree structure 
struct BPlusTree {
    BTreeNode* root;
    CompareFunc compare;
    PrintFunc print;
    CloneFunc clone;
    FreeFunc free_func;
};

// Tree operations (generic)
BPlusTree* createBPlusTree(CompareFunc cmp, PrintFunc print, CloneFunc clone, FreeFunc free_key);
void bplusInsert(BPlusTree* tree, void* key);
void* bplusSearch(BPlusTree* tree, void* key);
void printBPlusTree(BPlusTree* tree);  //can I remove this
void freeBPlusTree(BPlusTree* tree);
int bplusDelete(BPlusTree* tree, void* key);

// Function pointer type for processing each key in range
typedef void (*ProcessKeyFunc)(void* key, void* user_data);

// Range search function declarations
void bplusRangeSearch(BPlusTree* tree, void* lower, void* upper, ProcessKeyFunc process, void* user_data);
void** getAllKeysInRange(BPlusTree* tree, void* lower, void* upper, int* count);

#endif
