#ifndef _tree_h
#define _tree_h
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "data.h"

typedef struct Data Data;

typedef enum treeType{ HUFFMAN, AVL, SEGMENT } treeType;

typedef struct TNode
{
    /* Data for every TNode */
    bool leaf;              /* leaf is true if this node is an empty leaf node  */
    struct TNode* pLeft;    /* left child (NULL for leaf) */
    struct TNode* pRight;   /* right child (NULL for leaf) */
    struct TNode* pParent;  /* parent TNode (NULL for root) */

    /* AVL data */
    int height;             /* max number of nodes on path from this node down to a leaf of the tree */
    Data* data;             /* Pointer to the data stored in the node, leaves contain no valid data */

    /* Huffman data */
    int priority;
    char *str;

    /* Segment tree data */
    double low, high;
    int cnt;
}  TNode;

typedef struct Tree
{
    TNode* root;
    treeType type;
}  Tree;

/**********  Functions for creating/freeing a tree **********/
Tree *createTree( );
Tree *createTreeFromTNode( TNode* root );
void freeTree( Tree* t );
void freeTreeContents( TNode *root, treeType type );

/**********  Functions for creating/linking TNodes tree **********/
void attachLeafNodes( TNode *ins );
void attachChildNodes( TNode* root, TNode* left, TNode* right );

/**********  Functions for searching an AVL tree **********/
TNode* searchTree( Tree *t, Data* tData );
TNode* searchTreeRec( TNode *root, Data* tData );

/**********  Functions for inserting/removing from an AVL tree **********/
void insertAtTNode( TNode *ins, Data* tData );
void insertTree( Tree* t, Data* tData );
void insertTreeBalanced( Tree* t, Data* tData );
Data* removeTree( Tree* t, char* key );

/**********  Functions for getting Huffman Encoding **********/
void printHuffmanEncoding( TNode* root, char c );

/**********  Functions for Segment Tree **********/
TNode* constructSegmentTree( double* points, int low, int high);
void insertSegment( TNode* root, double segmentStart, double segmentEnd );
int lineStabQuery( TNode* root, double queryPoint );

/**********  Functions for debugging an AVL tree **********/
void printTree( TNode* root );
void checkAVLTree( TNode* root );

#endif
