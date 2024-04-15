/*
 *     uarray2b.c
 *     by Rolando Ortega (rorteg02) Jason Singer (jsinge02), 2/22/2024
 *     locality
 *
 *     Implementation of a two dimensional blocked array 
 */
#include "uarray2b.h"
#include "uarray2.h"
#include "uarray.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>

#define T UArray2b_T

int getIndex(int col, int row, int blocksize);
void apply_free_blocks(int i, int j, UArray2_T array, void *block, void* cl);

/*Structure storing important information accesed by the UArray2b interface
Stores the width, height, size, and blocksize of the array, as well as a 
UArray2 of UArrays representing the differnt blocks*/
struct T {
        int width, height;
        int size;
        int blocksize;
        UArray2_T blocks; 
};

/********************getIndex*********************************************
*
* Function that calculates an index in the UArray2, converting the 2D
* index into a corresponding index of a one dimensional array 

* Parameters: int row: a row in the UArray2
*             int column: a column in the UArray2
*             int blocksize: the length of a block the UArray2
*
* Return: an int of an index in the UArray2 of blocks
*      
*********************************************************************/
int getIndex(int col, int row, int blocksize) 
{
        return blocksize * (row % blocksize) + (col % blocksize);
}

/*************UArray2b_new*********************************************
*
* Function that initializes an empty UArray2b with the specified         
* dimensions and blocksize
* 
* Parameters: int width: the width of the UArray2b
*             int height: the height of the UArray2b
*             int size: the size of an element in the UArray2b
*             int blocksize: the length of a block in the UArray2b
*
* Return: the newly initilized UArray2bw
*
* Expects: there is space for size amount of memory can be allocated, and 
* blocksize > 1
*      
* Notes: each block can store blockize * blocksize blocks
*      
*********************************************************************/
extern T UArray2b_new(int width, int height, int size, int blocksize) 
{
        assert(blocksize > 1);

        struct UArray2b_T *new_arr = malloc(sizeof(struct UArray2b_T));
        assert(new_arr != NULL);
        new_arr->width = width;
        new_arr->height = height;
        new_arr->blocksize = blocksize;
        new_arr->size = size;
        new_arr->blocks = UArray2_new(ceil((double)width / (double)blocksize), 
            ceil((double)height / (double)blocksize), sizeof(UArray_T));
        
        /*Equation gets the number of total blocks in the rows and cols of 
        the blocks UArray2*/
        int numBlocksInCol = ceil((double)height / (double)blocksize);
        int numBlocksInRow = ceil((double)width / (double)blocksize);

        /*Initlizes the UArray_Ts on our blocks UArray2*/
        for (int i = 0; i < numBlocksInCol; i++) {
                for (int j = 0; j < numBlocksInRow; j++) {
                        UArray_T *thisBlock = UArray2_at(new_arr->blocks, j, i);
                        *thisBlock = UArray_new(blocksize * blocksize, size);
                }
        }
        return new_arr;
}

/*************UArray2b_new_64K_block*****************************************
*
* Function that initializes an empty UArray2b with the specified         
* dimensions and blocks that can store 64K space of memory
* 
* Parameters: int width: the width of the UArray2b
*             int height: the height of the UArray2b
*             int size: the size of an element in the UArray2b
*
* Return: the newly initilized UArray2b with an automatically set blocksize
*
* Expects: there is space for size amount of memory can be allocated, and 
* blocksize > 1
*      
* Notes: if the size of the elems > 64K, a CRE will be raised after the new
* function is called
*      
*********************************************************************/
extern T UArray2b_new_64K_block(int width, int height, int size)
{
        int blocksize;

        if (size > 64 * 1024){
                blocksize = 1;
        }
        else {
                blocksize = sqrt((64 * 1024) / size);
        }

        return UArray2b_new(width, height, size, blocksize);
}

/*************UArray2b_free*******************************************
*
* Function that frees the memory of an initialized UArray2
*
* Parameters: T *array2b: a UArray2b that has been initialized
*
* Return: void
*
* Expects: UArray2 a is not NULL
*      
* Notes: Frees the blocked array as well as the blocks themselves
*      
*******************************************************************/
extern void UArray2b_free(T *array2b) 
{
        assert(array2b != NULL);
        UArray2_map_row_major((*array2b)->blocks, apply_free_blocks, NULL);
        UArray2_free(&(*array2b)->blocks);
        free(*array2b);
}

/*************UArray2b_at*******************************************
*
* Function that gets an element at specified indicies in the given uarray2
*
* Parameters: UArray2b_T : array2p a UArray2 that has been initialized
*                 int col: col index of a UArray2
*                 int row: row index of a UArray2
*
* Return: void pointer of element at given index
*
* Expects: array2 is not NULL, indicies are within bounds of array
*      
*********************************************************************/
extern void *UArray2b_at(UArray2b_T array2b, int col, int row) 
{
        assert(array2b != NULL);
        assert(col >= 0 && col < array2b->width);
        assert(row >= 0 && row < array2b->height);

        int blocksize = array2b->blocksize;
        UArray_T *thisBlock = (UArray_T *)UArray2_at(array2b->blocks, 
                col / blocksize, row / blocksize);
        void *elem = UArray_at(*thisBlock, getIndex(col, row, blocksize));
        return elem;
}

/*************UArray2_width*******************************************
*
* Function that gets the width of the UArray2, which is equal to the 
* number of columns
*
* Parameters:T array2b: a UArray2b that has been initialized
*
* Return: an int of the given array’s width
*
* Expects: the UArray2 is not NULL
*      
*********************************************************************/
extern int UArray2b_width(T array2b) 
{
        assert(array2b != NULL);
        return array2b->width;
}

/*************UArray2_height*******************************************
*
* Function that gets the height of the UArray2, which is equal to the 
* number of rows
*
* Parameters:T array2b: a UArray2b that has been initialized
*
* Return: an int of the given array’s height
*
* Expects: the UArray2 is not NULL
*      
*********************************************************************/
extern int UArray2b_height(T array2b) 
{
        assert(array2b != NULL);
        return array2b->height;
}

/*************UArray2_size*******************************************
*
* Function that gets the size of the elements in the UArray2b
*
* Parameters:T array2b: a UArray2b that has been initialized
*
* Return: an int of the given array elem size
*
* Expects: the UArray2 is not NULL
*      
*********************************************************************/
extern int UArray2b_size(T array2b) 
{
        assert(array2b != NULL);
        return array2b->size;
}

/*************UArray2_blocksize*******************************************
*
* Function that gets the length of the blocks in the UArray2b
*
* Parameters:T array2b: a UArray2b that has been initialized
*
* Return: an int of the given array's blocksize
*
* Expects: the UArray2 is not NULL
*      
* Notes: blocksize that is returned is the length of a block. Blocks can hold
* blocksize * blocksize elements
*********************************************************************/
extern int UArray2b_blocksize(T array2b) 
{
        assert(array2b != NULL);
        return array2b->blocksize;
}

/*************UArray2b_map***********************************
*
* Traverses the elements in the UArray2b by blcok and calls the apply         
* function for each element. 
*
* Parameters: A2Methods_UArray2 uarray2: a UArray2 that has been initialized 
*          void apply: a function pointer for a function that will 
*          be called for each element that the map function         
*          accesses. The apply function should take at least a row  
*          index, a column index, the elem being accessed, and the UArray2_T. 
*          void *cl: a closure statement for the map function 
*
* Return: nothing, but apply function affects elements of array and closure
*
*********************************************************************/
extern void UArray2b_map(T array2b, void apply(int col, int row, T array2b, 
        void *elem, void *cl), void *cl)
{
        assert(array2b != NULL);
        int numBlocks = UArray2_width(array2b->blocks);

        int row = 0;
        int col = 0;
        
        /*Loop for each block, then inner nested loop iterates through the 
        blocks themselves*/
        for (int blocks = 1; blocks <= numBlocks * numBlocks; blocks++){
                for (int i = row; i < row + array2b->blocksize 
                        && i < array2b->height; i++){
                        for (int j = col; j < col + array2b->blocksize 
                                && j < array2b->width; j++){   
                                apply(j, i, array2b, UArray2b_at(array2b, j, i),
                                        cl);
                        }
                }
                col += array2b->blocksize;
                if (blocks % numBlocks == 0){
                        col = 0;
                        row += array2b->blocksize;
                }
        } 
}

/*Helper apply function that frees the individual blocks of the UArray2 when 
called by a mapping function*/
void apply_free_blocks(int i, int j, UArray2_T array, void *block, void* cl)
{
        (void) i;
        (void) j;
        (void) array;
        (void) cl;
        UArray_T thisBlock = *(UArray_T *)block;
        UArray_free(&thisBlock);
}