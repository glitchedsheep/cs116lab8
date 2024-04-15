/*
 *     bit2.c
 *     by Jason Singer (jsinge02) Rolando Ortega (rorteg02), 2/11/2024
 *     iii
 *
 *     Implementation of a two dimensional Bit array. 
 */

#include <bit2.h>
#include <bit.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h> 

int index(int row, int column, int width);


struct Bit2_T {
        Bit_T bitArray;
        int height;
        int width;
};

/********************index*********************************************
*
* Function that calculates an index in the Bit2 

* Parameters: int row: a row in the Bit2
*             int column: a column in the Bit2
*             int width: the width of the Bit2
*
* Return: an int of an index in the Bit2
*
* Expects: N/A
*      
* Notes: N/A
*      
*********************************************************************/
int index(int row, int column, int width){
        return (row * width) + column;
}

/*********************Bit2_new*********************************************
*
* Function that initializes an empty Bit2 with the specified         
* dimensions
*
* Parameters: int DIM1: the width of the Bit2
*             int DIM2: the height of the Bit2
*
* Return: an int of the given array’s width
*
* Expects: That the dimensions are greater than 0.
*      
* Notes: under the hood, our Bit2_T implementation will be a one 
* dimensional array
*      
*********************************************************************/
Bit2_T Bit2_new(int DIM1, int DIM2)
{
        /* check for valid dimensions */
        assert(DIM1 > 0 && DIM2 > 0);

        /* make new Bit2 array */
        struct Bit2_T *new_array = malloc(sizeof(struct Bit2_T));
        assert(new_array != NULL);
        new_array->bitArray = Bit_new(DIM1 * DIM2);
        new_array->height = DIM2;
        new_array->width = DIM1;

        return new_array;
}

/*************Bit2_height******************************************
*
* Function that provides the height of the Bit2, which is equal to   
* the number of rows. 
*
* Parameters: Bit2_T a: a Bit2 that has been initialized
*
* Return: an int that stores the number of rows in the given Bit2
*
* Expects: at most one argument, a Bit2_T, which should not be a     
* NULL pointer. 
*      
* Notes: under the hood, our Bit2_T implementation will be a one 
* dimensional array.
*      
*********************************************************************/
int Bit2_height(Bit2_T b)
{
        /* assert b is not NULL */
        assert(b != NULL);
        return b->height;
}

/*************Bit2_width******************************************
*
* Function that provides the width of the Bit2, which is equal to   
* the number of columns. 
*
* Parameters: Bit2_T a: a Bit2 that has been initialized
*
* Return: an int that stores the number of columns in the given Bit2
*
* Expects: at most one argument, a Bit2_T, which should not be a     
* NULL pointer. 
*      
* Notes: under the hood, our Bit2_T implementation will be a one 
* dimensional array.
*      
*********************************************************************/
int Bit2_width(Bit2_T b)
{
        /* assert b is not NULL */
        assert(b != NULL);
        return b->width;
}

/*********************Bit2_get*********************************************
*
* Function that provides the bit at the given column and row in the given Bit2
*
* Parameters: Bit2_T b: a Bit2 that the function is accessing a bit from
*             int col: a column in the Bit2
*             int row: a row in the Bit2
*
* Return: an int containing the bit at the given column and row
*
* Expects: The Bit2_T that is provided should not be null, and the provided row
*          and column should be within the bounds of the given Bit2.
* 
* Notes: Under the hood, our Bit2 is a one dimensional Bit array. This function 
* throws checked runtime errors if the Bit2 provided is NULL or the provided 
* indices are out of bounds for the given Bit2. 
*      
*********************************************************************/
int Bit2_get(Bit2_T b, int col, int row)
{
        /* assert b is not NULL and indices are in bounds */
        assert(b != NULL);
        assert(row >= 0 && row < b->height);
        assert(col >= 0 && col < b->width);

        int bit = 0;
        bit = Bit_get(b->bitArray, index(row, col, b->width));
        return bit;
}

/*********************Bit2_put*********************************************
*
* Function that inserts a given bit into the Bit2 at a specifiec array, 
* and returns the bit that was previously at the given index

* Parameters: int col: the column index of the Bit2
*             int row: the row index of the Bit2
*
* Return: an integer representation of the bit at the given index before 
*         the new bit was inserted
*
* Expects: That the dimensions are greater than 0, and that the inserted bit
*          integer is either 0 or 1, and the Bit2 is not null.
*      
* Notes: under the hood, our Bit2_T implementation will be a one 
* dimensional array. Throws checked runtime errors if the provided Bit2 is NULL,
* the given indices are not within bounds, or the given bit is not 0 or 1 (the
* last expection is covered by the Bit interface).
*/
int Bit2_put(Bit2_T b, int col, int row, int bit)
{
        /* assert b is not NULL */
        assert(b != NULL);
        assert(row >= 0 && row < b->height);
        assert(col >= 0 && col < b->width);
        /*Bit_put asserts that bit is either 0 or 1*/
        return Bit_put(b->bitArray, index(row, col, b->width), bit);
}

/*************Bit2_free*******************************************
*
* Function that frees the memory of an initialized Bit2_T
*
* Parameters: Bit2_T *b: address of a Bit2_T that has been initialized
*
* Return: void
*
* Expects: Bit2_T is not NULL
*      
* Notes: does not free the memory of the individual elements inside  
* the array
*      
*******************************************************************/

void Bit2_free(Bit2_T *b)
{
        /* assert b is not NULL */
        assert(b != NULL);
        Bit_free(&(*b)->bitArray);
        free(*b);
}

/*******************Bit2_map_row_major***********************************
*
* Traverses the elements in the Bit2 by row and calls the apply         
* function for each element. 
*
* Parameters: Bit2_T b: a Bit2 that has been initialized 
*          void apply: a function pointer for a function that will 
*          be called for each element that the map function         
*          accesses. The apply function should take at least a row  
*          i, a column j and the Bit2_T. 
*          void *c: a closure statement for the map function 
*
* Return: nothing
*
* Expects that the Bit2_T is not a NULL pointer. 
*      
* Notes: under the hood, our Bit2_T implementation will be a one 
* dimensional array. Raises checked runtime error if Bit2 is not NULL.
*      
*********************************************************************/
void Bit2_map_row_major(Bit2_T b, void apply(int i, int j, Bit2_T b, 
                            int bit, void *p1), void *cl)
{
        /* assert b is not NULL */
        assert(b != NULL);
        for (int rows = 0; rows < b->height; rows++){
                for (int cols = 0; cols < b->width; cols++){
                        int bit1 = Bit2_get(b, cols, rows);
                        apply(cols, rows, b, bit1, cl);
                }
        }
}

/*************Bit2_map_col_major***********************************
*
* Traverses the elements in the Bit2 by column and calls the apply     
* function for each element. 
*
* Parameters: Bit2_T b: a Bit2 that has been initialized 
*          void apply: a function pointer for a function that will    
*          be called for each element that the map function             
*          accesses. The apply function should take at least a row        
*          i, a column and the Bit2_T. 
*          void *c: a closure statement for the map function 
*
* Return: nothing
*
* Expects that the Bit2_T is not a NULL pointer. 
*      
* Notes: under the hood, our Bit2_T implementation will be a one 
* dimensional array. Raises checked runtime error if the Bit2 is NULL. 
*      
*********************************************************************/
void Bit2_map_col_major(Bit2_T b, void apply(int i, int j, Bit2_T a, 
                            int b, void *p1), void *cl)
{
        /* assert b is not NULL */
        assert(b != NULL);
        for (int cols = 0; cols < b->width; cols++){
                for (int rows = 0; rows < b->height; rows++){
                        int bit1 = Bit2_get(b, cols, rows);
                        apply(cols, rows, b, bit1, cl);
                }
        }
}
