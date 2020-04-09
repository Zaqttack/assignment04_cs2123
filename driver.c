#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "data.h"
#include "tree.h"
#include "priorityQueue.h"

#define MAX_VALUE 1000

/**********  Functions for testing Huffman Tree **********/
void testHuffmanEncoding( char *str );

/**********  Functions for testing AVL Tree **********/
void testAVLTree( );
void createName( int key, char arr[] );

/**********  Functions for testing Segment Tree **********/
void testSegmentTree( char *fileName );
int carTraversalTree( double moveSequence[], int numMoves );
void readArray( char* fileName, double** pmoveSequence, int* pprovidedSolution, int* pnumMoves );
int removeDuplicates( double* points, int oldSize );
int cmpDoubles (const void * a, const void * b);

int main( int argc, char *argv[] )
{
    /* test the Huffman-Encoding */
    printf("HUFFMAN TREE TEST:\n");
    testHuffmanEncoding( "aabacccadadadadda" );

    /* test the AVL tree */
    printf("AVL TREE TEST:\n");
    testAVLTree( );

    /* test the Segment tree */
    printf("SEGMENT TREE TEST:\n");
    testSegmentTree( "CTP-Simple01.txt" );

    return 0;
}


/**********  Functions for testing Huffman Encoding **********/

/* testHuffmanEncoding
 * input: a string
 * output: none
 *
 * Prints Huffman encoding for each char in the original string
 */
void testHuffmanEncoding( char *str ){
    int i, charCounts[26], length = strlen(str);
    bool flag = false;
    TNode* root;
    Tree* pt;
    PriorityQueue* ppq = createPQ();

    /* Compute frequency (i.e. # instances) of each lowercase character */
    for( i=0; i<26; i++ )
        charCounts[i]=0;

    for( i=0; i<length; i++ ){
        if( 'a' <= str[i] && str[i] <= 'z' ){
            charCounts[ str[i]-'a' ]++;
            flag = true;
        }
    }

    if( !flag ){
        printf("No lowercase characters in \"%s\"!\n", str);
        return;
    }

    /* enter all of the frequencies into the priority queue */
    for( i=0; i<26; i++ ){
        if( charCounts[i]>0 ){
            root = (TNode*)malloc( sizeof(TNode) );
            attachLeafNodes( root );

            root->str = (char*)malloc( (length+1)*sizeof(char) );
            root->priority = charCounts[i];
            root->str[0] = 'a'+i;
            root->str[1] = '\0';
            insertPQ( ppq, root );
        }
    }

    /* Build Huffman encoding tree */
    TNode *min1 = removePQ( ppq );
    while( !isEmptyPQ(ppq) ){
        TNode* min2 = removePQ( ppq );

        root = (TNode*)malloc( sizeof(TNode) );
        root->str = (char*)malloc( (length+1)*sizeof(char) );
        root->priority = min1->priority + min2->priority;
        root->str[0] = '\0';
        root->leaf = false;
        strcat( root->str, min1->str );
        strcat( root->str, min2->str );
        attachChildNodes( root, min1, min2 );
        insertPQ( ppq, root );

        min1 = removePQ( ppq );
    }
    root = min1;

    /* get the encoding for each char in the tree */
    for( i=0; i<26; i++ ){
        if( charCounts[i]>0 ){
            printf("The character '%c' is encoded as ", 'a'+i );
            printHuffmanEncoding( root, 'a'+i );
            printf("\n");
        }
    }
    printf("\n");

    pt = createTreeFromTNode( root );
    pt->type = HUFFMAN;
    freeTree( pt );
    freePQ( ppq );
}


/**********  Functions for testing AVL-Tree **********/

void testAVLTree( ){
    int i = 0;
    char testData[30];
    Data *temp;
    clock_t start, end;

    Tree* pt = createTree();
    pt->type = AVL;

    /* Time the insert function */
    start = clock();
    for( i=1; i<MAX_VALUE; i++){
        temp = (Data *)malloc( sizeof(Data) );
        temp->verification = i;
        temp->key = (char*)malloc( 31*sizeof(char) );
        createName( i, temp->key );
        insertTreeBalanced( pt, temp );
        // checkAVLTree( pt->root );
    }
    end = clock();
    printf( "Time to insert (in seconds): %lf\n" , (double)(end - start) / CLOCKS_PER_SEC );
    // printTree( pt->root );

    /* Time the remove function */
    start = clock();
    for( i=MAX_VALUE-1; i>0; i--){
        createName( i, testData );

        temp = removeTree( pt, testData );
        if( temp==NULL )
            printf( "NULL returned for: %s\n", testData );
        else if( temp->verification!=i ){
            printf( "Wrong value returned for: %s\n", testData );
            freeData( temp );
        }
        else{
            //printf( "Correctly removed: %s\n", testData );
            freeData( temp );
        }

        temp = removeTree( pt, testData );
        // checkAVLTree( pt->root );
        if( temp!=NULL ){
            printf( "Failed to remove: %s\n", testData );
            freeData( temp );
        }
        // checkAVLTree( pt->root );
    }
    end = clock();
    printf( "Time to remove (in seconds): %lf\n" , (double)(end - start) / CLOCKS_PER_SEC );
    // printTree( pt->root );

    /* Free all data in pt */
    freeTree( pt );
    printf("\n");
}

void createName( int freq, char *keyName ){
    int i;
    bool b = true;
    for( i=29; i>=0; i-- ){
        if( freq%10 == 0 && !b){
            keyName[i] = '-';
        }
        else {
            keyName[i] = '0' + freq%10;
            b = false;
        }
        freq = freq/10;
    }
    keyName[30] = '\0';
}


/**********  Functions for testing Segment Tree **********/

void testSegmentTree( char *fileName ){
    double *moveSequence;
    int providedSolution, computedSolution;
    int numMoves;

    readArray( fileName, &moveSequence, &providedSolution, &numMoves );
    computedSolution = carTraversalTree( moveSequence, numMoves );

    printf( "Your segment tree computed a solution of %d\n", computedSolution );
    if( providedSolution!=-1 && computedSolution==providedSolution ){
        printf( "Your algorithm worked correctly (i.e. same as provided solution)\n" );
    }
    else if( providedSolution!=-1){
        printf( "Your algorithm did not match the provided solution of %d.\n", providedSolution );
    }
}

void readArray( char *fileName, double** pmoveSequence, int* pprovidedSolution, int* pnumMoves ){
    double* moveSequence;

    *pnumMoves = 0;
    *pprovidedSolution = -1;

    int i;

    if( fileName != NULL ){
        FILE *in_file = fopen(fileName, "r");

        if(in_file == NULL)
        {
            printf("File %s not found.\n", fileName);
            exit(-1);
        }

        /* read the number of nodes from file and allocate space to store them */
        if( fscanf( in_file, "%d%d", pnumMoves, pprovidedSolution ) != 2 )
        {
            printf( "Invalid file format.  First line should be number of moves followed by the correct solution (or -1 if none is provided)\n");
            exit(-1);
        }
        if( *pnumMoves < 0 )
        {
            printf( "The number moves must be non-negative.\n");
            exit(-1);
        }

        (*pmoveSequence) = (double*) malloc( (*pnumMoves)*sizeof( double ) );
        moveSequence = (*pmoveSequence);
        /* read in names of nodes */
        for( i=0 ; i<(*pnumMoves); i++)
        {
            if( fscanf( in_file, "%lf", &moveSequence[i] ) == 0 )
            {
                printf( "Failed to read %dth double in the move sequence", i );
                exit(-1);
            }
        }
        fclose( in_file );
    }
}

int carTraversalTree( double moveSequence[], int numMoves ){
    double current = 0, next = 0;
    int i, max;
    double* segmentStartArray = (double*) malloc( numMoves*sizeof( double ) );
    double* segmentEndArray = (double*) malloc( numMoves*sizeof( double ) );
    double* points = (double*) malloc( (numMoves+1)*sizeof( double ) );
    Tree* pt;
    TNode* root;

    points[0] = 0;
    for( i=0 ; i<numMoves; i++)
    {
        current = next;
        next = next + moveSequence[i];

        if( current < next ){
            segmentStartArray[i] = current;
            segmentEndArray[i] = next;
        }
        else{
            segmentStartArray[i] = next;
            segmentEndArray[i] = current;
        }
        points[i+1] = next;
    }

    /* Sort the points and remove all duplicates */
    qsort( points, numMoves+1, sizeof(double), cmpDoubles );
    int numUnique = removeDuplicates( points, numMoves+1 );

    /* build the segment tree */
    root = constructSegmentTree( points, 0, numUnique-1 );
    for( i=0 ; i<numMoves; i++)
        insertSegment( root, segmentStartArray[i], segmentEndArray[i] );

    /* query the segment tree */
    max = -1;
    for( i=0 ; i<numUnique; i++)
    {
        int temp = lineStabQuery( root, points[i] );
        if( temp>max ){
            max = temp;
        }
    }

    //Free all of the nodes in our tree
    pt = createTreeFromTNode( root );
    pt->type = SEGMENT;
    freeTree( pt );
    free( points );
    free( segmentStartArray );
    free( segmentEndArray );
    free( moveSequence );

    return max;
}

int removeDuplicates( double* points, int oldSize ){
    int i, j = 0;

    for( i=1 ; i<oldSize; i++)
    {
        if(points[j]==points[i])
            continue;
        j++;
        points[j]=points[i];
    }

    return j+1;
}

int cmpDoubles (const void * a, const void * b) {
    if( *(double*)a - *(double*)b == 0 )
        return 0;
    else if ( *(double*)a - *(double*)b > 0 )
        return 1;
    else
        return -1;
}
