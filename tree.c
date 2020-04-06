#include "tree.h"

/**********  Helper functions for removing from an AVL tree **********/
TNode* removeNextInorder( TNode** pRoot );

/**********  Helper functions for balancing an AVL tree **********/
void updateHeights(TNode* root);
void rebalanceTree(Tree* t, TNode* x);
void rightRotate(TNode* root);
void leftRotate(TNode* root);
int getBalance(TNode* x);
TNode* getTallerSubTree(TNode* root);
bool isSameSignBalance(TNode* x, TNode* z);
int subTreeHeight(TNode* root);

/* createTree
 * input: none
 * output: a pointer to a Tree (this is malloc-ed so must be freed eventually!)
 *
 * Creates a new empty Tree and returns a pointer to it.
 */
Tree *createTree( )
{
    Tree* t = (Tree*)malloc( sizeof(Tree) );
    t->root = (TNode*)malloc( sizeof(TNode) );

    t->root->leaf = true;
    t->root->height = 0;
    t->root->pParent = t->root->pLeft = t->root->pRight = NULL;

    return t;
}

Tree *createTreeFromTNode( TNode* root )
{
    Tree* t = (Tree*)malloc( sizeof(Tree) );
    t->root = root;

    return t;
}

/* attachLeafNodes
 * input: a pointer to a TNode
 * output: none
 *
 * Mallocs and stores leaf nodes below the passed in TNode ins
 */
void attachLeafNodes( TNode *ins )
{
    /* Mark this node as not a leaf */
    ins->leaf = false;

    /* Add empty leaf nodes below this node */
    ins->pLeft = (TNode *)malloc( sizeof(TNode) );
    ins->pRight = (TNode *)malloc( sizeof(TNode) );
    if( ins->pLeft==NULL || ins->pRight==NULL ){
        fprintf( stderr, "malloc failed\n" );
        exit(-1);
    }
    ins->pLeft->leaf = ins->pRight->leaf = true;
    ins->pLeft->pParent = ins->pRight->pParent = ins;
    ins->pLeft->pLeft = ins->pRight->pLeft = ins->pLeft->pRight = ins->pRight->pRight = NULL;
    ins->pLeft->height = ins->pRight->height = 0;
}

/* attachChildNodes
 * input: three pointers to a TNodes
 * output: none
 *
 * Sets root's left and right children to the specified nodes
 */
void attachChildNodes( TNode* root, TNode* left, TNode* right ){
    root->pLeft = left;
    root->pRight = right;
    root->pLeft->pParent = root->pRight->pParent = root;
}

/* freeTree and freeTreeContents
 * input: a pointer to a Tree
 * output: none
 *
 * frees the given Tree and all of Data elements
 */
void freeTree( Tree *t )
{
    freeTreeContents(t->root, t->type);
    free(t);
}

void freeTreeContents( TNode *root, treeType type )
{
    if(root==NULL)
        return;

    freeTreeContents(root->pLeft, type);
    freeTreeContents(root->pRight, type);
    if(root->leaf==false){
        if(type==AVL && root->data!=NULL)
            freeData(root->data);
        if(type==HUFFMAN && root->str!=NULL)
            free(root->str);
    }
    free(root);
}


/**********  Functions for searching an AVL tree **********/

/* searchTree and searchTreeRec
 * input: a pointer to a Tree, a Data* tData
 * output: a pointer to the TNode that contains tData or, if no such node exists, a pointer to
 *
 * Finds and returns a pointer to the TNode that contains tData or, if no such node exists,
 * it returns a pointer to location where the TNode should be inserted at
 */
TNode* searchTree( Tree *t, Data* tData )
{
    return searchTreeRec( t->root, tData );
}

TNode* searchTreeRec( TNode *root, Data* tData )
{
    if( root->leaf == true || compareData( tData, root->data ) == 0 )
        return root;
    else if( compareData( tData, root->data ) < 0 )
        return searchTreeRec( root->pLeft, tData );
    else /* compareData( tData, root->data ) > 0 */
        return searchTreeRec( root->pRight, tData );
}


/**********  Functions for inserting/removing from an AVL tree **********/

/* insertAtTNode
 * input: a pointer to a TNode, a Data*
 * output: none
 *
 * Stores the passed Data* into the given leaf TNode, Does not rebalance tree
 */
void insertAtTNode( TNode *ins, Data* tData )
{
    if( !ins->leaf ){
        fprintf( stderr, "inserting into non-leaf node\n" );
        exit(-1);
    }

    attachLeafNodes( ins );
    updateHeights( ins );

    /* Put data in the node returned by search */
    ins->data = tData;
    ins->str = NULL;
}

/* insertTree
 * input: a pointer to a Tree, a Data*
 * output: none
 *
 * Stores the passed Data* into the Tree following BST order, Does not rebalance tree
 */
void insertTree( Tree *t, Data* tData )
{
    TNode* ins = searchTree( t, tData );
    insertAtTNode( ins, tData );
}

/* insertTreeBalanced
 * input: a pointer to a Tree, a Data*
 * output: none
 *
 * Stores the passed Data* into the Tree following BST order and rebalances the tree
 */
void insertTreeBalanced( Tree *t, Data* tData )
{
    TNode* ins = searchTree( t, tData );
    insertAtTNode( ins, tData );
    rebalanceTree( t, ins );
}

/* removeTree
 * input: a pointer to a Tree
 * output: a Data*
 *
 * Remove and returns the Data* with the specified key or NULL if its not in the tree
 */
Data* removeTree( Tree *t, char* key )
{
    Data temp;
    Data* ret;
    TNode *del, *update;
    TNode **parentDelPtr;

    temp.key = key;
    del = searchTree( t, &temp );
    ret = del->data;

    if( del->leaf == true )
        return NULL;

    /* Get previous node's pointer to del */
    if( del->pParent==NULL )
        parentDelPtr = &t->root;    /* del is the root */
    else
        parentDelPtr = del->pParent->pLeft==del ? &del->pParent->pLeft : &del->pParent->pRight; /* check if del if left or right child of its parent */

    /* del has no left child */
    if( del->pLeft->leaf==true ){
        free( del->pLeft );
        *parentDelPtr = del->pRight;
        update = del->pRight->pParent = del->pParent;
        free( del );
    }

    /* del has no right child */
    else if( del->pRight->leaf==true ){
        free( del->pRight );
        *parentDelPtr = del->pLeft;
        update = del->pLeft->pParent = del->pParent;
        free( del );
    }

    /* del has two children */
    else{
        TNode *next = removeNextInorder( &del->pRight );
        update = next->pParent;
        del->data = next->data;
        free(next);
    }

    /* Update the heights and rebalance around the node update */
    updateHeights(update);
    rebalanceTree(t, update);
    return ret;
}

TNode* removeNextInorder( TNode** pRoot ){
    TNode* temp = *pRoot;

    if( temp->pLeft->leaf == true ){
        *pRoot = temp->pRight;
        temp->pRight->pParent = temp->pParent;
        free( temp->pLeft );
    }
    else
        temp = removeNextInorder( &temp->pLeft );

    return temp;
}

int subTreeHeight(TNode* root){
    return root->height;
}

/* updateHeights
 * input: a pointer to a TNode
 * output: none
 *
 * Recursively recomputes the height of the current node and then recurses on the nodes parent
 */
void updateHeights(TNode* root){
    if(root!=NULL){
        root->height = subTreeHeight(root->pLeft)>subTreeHeight(root->pRight) ? subTreeHeight(root->pLeft) : subTreeHeight(root->pRight);
        root->height = root->height + 1;
        updateHeights( root->pParent );
    }
}

/* rebalanceTree
 * input: a pointer to a tree and a pointer to TNode
 * output: none
 *
 * Should attempt to rebalance the tree starting at x and going up through the root (i.e., until it reaches null).
 * After this function runs, every node should be balanced (i.e. -2 < balance < 2).
 *
 * Hint: The t->root will need to be updated if you do a rotate on the current root.
 * 
 * Extra Helper functions I can create 
 * TNode* getTallerSubTree(TNode* root);
 * bool isSameSignBalance(TNode* x, TNode* z);
 */
void rebalanceTree(Tree* t, TNode* x){ 

    // while(x != NULL) {
    //     // TNode* z = (TNode*)malloc(sizeof(TNode));
    //     // int xLeft = subTreeHeight(x->pLeft);
    //     // int xRight = subTreeHeight(x->pRight);
    //     // int xBalance = xLeft - xRight;
    //     TNode* tempNode = (TNode*)malloc(sizeof(TNode));
    //     printf("entering rebalance\n");

        
    //     // don't forget to call updateHeight()
    //     if(getBalance(x) == 2) {
    //         if(subTreeHeight(x->pLeft->pLeft) > subTreeHeight(x->pLeft->pRight)) {
    //             printf("+2 rotating right\n");
    //             tempNode = x->pLeft;
    //             rightRotate(x);
    //             x->pParent = tempNode;
    //             updateHeights(x);
    //         }
    //         else {
    //             printf("+2 rotating left:right\n");
    //             // rotate left first
    //             tempNode = x->pLeft->pRight;
    //             leftRotate(x->pLeft);
    //             x->pLeft->pParent = tempNode;
    //             updateHeights(x->pLeft);

    //             // rotate right second
    //             tempNode = x->pLeft;
    //             rightRotate(x);
    //             x->pParent = tempNode;
    //             updateHeights(x);
    //         }
    //     }
    //     else if(getBalance(x) == -2) {
    //         if(subTreeHeight(x->pRight->pLeft) > subTreeHeight(x->pRight->pRight)) {
    //             printf("-2 rotating left\n");
    //             tempNode = x->pRight;
    //             leftRotate(x);
    //             x->pParent = tempNode;
    //             updateHeights(x);
    //         }
    //         else {
    //             printf("-2 rotating right:left\n");
    //             // rotate right first
    //             tempNode = x->pRight->pLeft;
    //             rightRotate(x->pRight);
    //             x->pRight->pParent = tempNode;
    //             updateHeights(x->pRight);

    //             // rotate left second
    //             tempNode = x->pRight;
    //             leftRotate(x);
    //             x->pParent = tempNode;
    //             updateHeights(x);
    //         }
    //     }

    //     // if(x->pParent == NULL) {
    //     //     // root needs to equal TNode??

    //     // }

    //     // //checking if balance is -2 or 2
    //     // if(xBalance == -2 || xBalance == 2) {
    //     //     // printf("balance b/w -2 or 2\n");
    //     //     // setting z to the greater subtree (left or right)
    //     //     int zBalance;

    //     //     if(xLeft > xRight)
    //     //         z = x->pLeft;
    //     //     else
    //     //         z = x->pRight;

    //     //     // checking the balance signs ( + or - ) of x and z
    //     //     zBalance = subTreeHeight(z->pLeft) - subTreeHeight(z->pRight);
    //     //     if ((xBalance ^ zBalance) < 0) {
    //     //         if(zBalance > 0)
    //     //             rightRotate(z);
    //     //         else
    //     //             leftRotate(z);
    //     //     }

    //     //     // rotating x depending on a blanace of 2 or -2
    //     //     if(xBalance == 2)
    //     //         rightRotate(x);
    //     //     else if(xBalance == -2)
    //     //         leftRotate(x);
            
    //     //     // updating height of rotated nodes
    //     //     TNode* tallerSubTree = getTallerSubTree(x);
    //     //     x->height = tallerSubTree->height + 1;

    //     //     // if x was the orig. root then update new root to the parent of x
    //     //     x->pParent = x;
    //     // }
    //     // x = x->pParent;
    //     // break;
    // } 
}

TNode* getTallerSubTree(TNode* x) {
    int leftHeight, rightHeight;

    leftHeight = subTreeHeight(x->pLeft);
    rightHeight = subTreeHeight(x->pRight);

    if(leftHeight > rightHeight)
        return x->pLeft;
    else
        return x->pRight;
}

/* rightRotate and leftRotate
 * input: a pointer to a TNode
 * output: none
 *
 * Performs specified rotation around a given node
 */
void rightRotate(TNode* oldRoot){
    TNode *newRoot = oldRoot->pLeft;

    if( oldRoot->pParent!=NULL ){
        if( oldRoot->pParent->pLeft==oldRoot )
            oldRoot->pParent->pLeft = oldRoot->pLeft;
        else
            oldRoot->pParent->pRight = oldRoot->pLeft;
    }
    newRoot->pParent = oldRoot->pParent;

    oldRoot->pLeft = newRoot->pRight;
    if( newRoot->pRight!=NULL )
        newRoot->pRight->pParent = oldRoot;

    oldRoot->pParent = newRoot;
    newRoot->pRight = oldRoot;

    updateHeights( oldRoot );
}

void leftRotate(TNode* oldRoot){
    TNode *newRoot = oldRoot->pRight;

    if( oldRoot->pParent!=NULL ){
        if( oldRoot->pParent->pRight==oldRoot )
            oldRoot->pParent->pRight = oldRoot->pRight;
        else
            oldRoot->pParent->pLeft = oldRoot->pRight;
    }
    newRoot->pParent = oldRoot->pParent;

    oldRoot->pRight = newRoot->pLeft;
    if( newRoot->pLeft!=NULL )
        newRoot->pLeft->pParent = oldRoot;

    oldRoot->pParent = newRoot;
    newRoot->pLeft = oldRoot;

    updateHeights( oldRoot );
}

/* getBalance
 * input: a pointer to a TNode
 * output: none
 *
 * Finds the balance of the given node
 */
int getBalance(TNode* root){
    if(root->leaf==true)
        return 0;
    return subTreeHeight(root->pLeft) - subTreeHeight(root->pRight);
}

/**********  Functions for getting Huffman Encoding **********/

/* printHuffmanEncoding
 * input: a pointer to TNode and a char
 * output: none
 *
 * This function prints the Huffman encoding for the char c.  This encoding is based on the given Huffman tree.
 */
void printHuffmanEncoding( TNode* root, char c ){
    char *encoding = (char*)malloc(sizeof(char) * 50);
    char tempstr[2];
    tempstr[0] = c;
    tempstr[1] = '\0';

    while(root != NULL) {
        if(strcmp(tempstr, root->str) == 0)
            break;

        if(strchr(root->pLeft->str, c) != 0) {
            strcat(encoding, "0");
            root = root->pLeft;
        }
        else {
            strcat(encoding, "1");
            root = root->pRight;
        }
    }
    printf("%s", encoding);
    free(encoding);
}

/**********  Functions for Segment Tree **********/

/* constructSegmentTree
 * input: an array of doubles, an int low, an int high
 * output: the root of a tree
 *
 * Recursively builds a balanced tree containing all of the data in array points from index low to index high.
 */
TNode* constructSegmentTree( double* points, int low, int high ){
    TNode* root = (TNode*)malloc( sizeof(TNode) );
    root->cnt = 0;
    root->low = points[low];
    root->high = points[high];
    root->leaf = false;
    root->pParent = NULL;

    /* Recursively split the array around the mid point of the high and low indices */
    int mid = (high - low)/2 + low;
    if( low==high ) /* only one node left in the sub-array */
        attachLeafNodes( root );
    else
        attachChildNodes( root, constructSegmentTree( points, low, mid ), constructSegmentTree( points, mid+1, high ) );

    return root;
}

/* insertSegment
 * input: the root of a tree, a double segmentStart, and a double segmentEnd
 * output: none
 *
 * Recursively inserts the line segment from segmentStart to segmentEnd into the tree
 */
void insertSegment( TNode* root, double segmentStart, double segmentEnd ){
    //TODO
    if(root->leaf == true)
        return;
    else if(segmentStart == root->low || segmentEnd == root->high)
        return;
    else if(root->high - root->low < segmentEnd) {
        root->cnt++;
        return;
    }
    else {
        insertSegment(root->pLeft, segmentStart, segmentEnd);
        insertSegment(root->pRight, segmentStart, segmentEnd);
    }
}

/* lineStabQuery
 * input: the root of a tree, a double queryPoint
 * output: none
 *
 * Recursively count the number of line segments which intersect the queryPoint.
 */
int lineStabQuery( TNode* root, double queryPoint ){
    //TODO
    if(root->leaf == true)
        return 0;
    else if(queryPoint == root->low || queryPoint == root->high) {
        return 0;
    }
    else {
        return lineStabQuery(root->pLeft, queryPoint) + lineStabQuery(root->pRight, queryPoint) + root->cnt;
    }
    // return -1;
}



/**********  Functions for debugging an AVL tree **********/

/* printTree
 * input: a pointer to a Tree
 * output: none
 *
 * Prints the contents of the tree below the root node
 */
void printTree( TNode* root ){
    int i;
    if(root->leaf!=true){
        printTree(root->pLeft);
        for( i=1; i<root->height; i++){
            printf("\t");
        }
        printf("%s\n",root->data->key);
        printTree(root->pRight);

    }
}

/* checkAVLTree
 * input: a pointer to a Tree
 * output: none
 *
 * Prints error messages if there are any problems with the AVL tree
 */
void checkAVLTree(TNode* root){
    if(root->leaf != true){
        if( getBalance(root)>1 ||  getBalance(root)<-1 )
            printf("ERROR - Node %s had balance %d\n",root->data->key,getBalance(root) );
        if( root->pLeft!=NULL && root->pLeft->pParent!=root )
            printf("ERROR - Invalid edge at %s-%s\n",root->data->key,root->pLeft->data->key );
        if( root->pRight!=NULL && root->pRight->pParent!=root )
            printf("ERROR - Invalid edge at %s-%s\n",root->data->key,root->pRight->data->key );

        checkAVLTree(root->pLeft);
        checkAVLTree(root->pRight);
    }
}
