/*
 *     uarray2.c
 *     by Rolando Ortega (rorteg02) Jason Singer (jsinge02), 2/11/2024
 *     iii
 *
 *     Implementation of a two dimension unboxed array. 
 */

#include <uarray2.h>
#include <uarray.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h> 

int index(int row, int column, int width);

struct UArray2_T {
        UArray_T array;
        int height;
        int width;
        int size;
};

/********************index*********************************************
*
* Function that calculates an index in the UArray2, converting the 2D
* index into a corresponding index of a one dimensional array 

* Parameters: int row: a row in the UArray2
*             int column: a column in the UArray2
*             int width: the width of the UArray2
*
* Return: an int of an index in the UArray2
*
* Expects: N/A
*      
* Notes: N/A
*      
*********************************************************************/
int index(int row, int column, int width){
        /* Calculate index and return */
        return (row * width) + column;
}

/*************UArray2_new*********************************************
*
* Function that initializes an empty UArray2 with the specified         
* dimensions
* 
* Parameters: int DIM1: the width of the UArray2
*             int DIM2: the height of the UArray2
*             int ELEMENT_SIZE: the size of an element in the UArray2
*
* Return: an int of the given array’s width
*
* Expects: there is space for ELEMENT_SIZE amount of memory can be 
* allocated and that the dimensions are greater than 0.
*      
* Notes: under the hood, our UArray2_T implementation will be a one 
* dimensional array
*      
*********************************************************************/
UArray2_T UArray2_new(int DIM1, int DIM2, int ELEMENT_SIZE)
{
        /* Check that the dimensions are valid */
        assert(DIM1 > 0 && DIM2 > 0);
        
        /* Create a one dimensional UArray */
        struct UArray2_T *new_array = malloc(sizeof(struct UArray2_T));
        assert(new_array != NULL);

        /* Set parameters */
        new_array->array = UArray_new(DIM1 * DIM2, ELEMENT_SIZE);
        new_array->height = DIM2;
        new_array->width = DIM1;
        new_array->size = ELEMENT_SIZE;

        return new_array;
}

/*************UArray2_height******************************************
*
* Function that provides the height of the UArray2, which is equal to   
* the number of rows. 
*
* Parameters: UArray2_T a: a UArray2 that has been initialized
*
* Return: an int that stores the number of rows in the given UArray2
*
* Expects: at most one argument, a UArray2_T, which should not be a     
* NULL pointer. 
*      
* Notes: under the hood, our UArray2_T implementation will be a one 
* dimensional array.
*      
*********************************************************************/
int UArray2_height(UArray2_T a)
{
        /* Check that UArray2 is not NULL */
        assert(a != NULL);
        return a->height;
}

/*************UArray2_width*******************************************
*
* Function that gets the width of the UArray2, which is equal to the 
* number of columns
*
* Parameters: UArray2_T a: a UArray2 that has been initialized
*
* Return: an int of the given array’s width
*
* Expects: the UArray2 is not NULL
*      
* Notes: under the hood, our UArray2_T implementation will be a one 
* dimensional array.
*      
*********************************************************************/
int UArray2_width(UArray2_T a)
{
        /* Check that UArray2 is not NULL */
        assert(a != NULL);
        return a->width;
}

/*************UArray2_size********************************************
*
* Function that returns the size of the UArray2, which is equal to the    
* memory size of each element. 
*
* Parameters: UArray2_T a: a UArray2 that has been initialized 
*
* Return: an int that stores the amount of memory for each element in   
* the given UArray2
*
* Expects: UArray2 that is not a NULL pointer. 
*      
* Notes: under the hood, our UArray2_T implementation will be a one 
* dimensional array.
*      
*********************************************************************/
int UArray2_size(UArray2_T a)
{
        /* Check that UArray2 a is not NULL */
        assert(a != NULL);
        return a->size;
}
 
/*************UArray2_at**********************************************
*
* Function that takes in a UArray2, and the row and column index, and 
* returns the value in the given column and row index.
*
* Parameters: UArray2_T a: a UArray2 that has been initialized
*          int row: index of the row that is to be accessed
*          int column: index of the column that is to be accessed
*
* Return: a void pointer to the element in the specified index
*
* Expects: The dimensions given are inside the bounds of the UArray2,   
* and the UArray2 is not NULL
*      
* Notes: under the hood, our UArray2_T implementation will be a one 
* dimensional array. Throws checked runtime error if indices are out   
* of bounds or if the given UArray2 is null.  
*      
********************************************************************/
void *UArray2_at(UArray2_T a, int col, int row)
{
        /* Check that arguments are correct */
        assert(a != NULL);
        assert(row >= 0 && row < a->height);
        assert(col >= 0 && col < a->width);
        
        int ind = index(row, col, a->width);
        
        return UArray_at(a->array, ind);
}

/*************UArray2_free*******************************************
*
* Function that frees the memory of an initialized UArray2
*
* Parameters: UArray2_T a: a UArray2 that has been initialized
*
* Return: void
*
* Expects: UArray2 a is not NULL
*      
* Notes: does not free the memory of the individual elements inside   
* the array
*      
*******************************************************************/
void UArray2_free(UArray2_T *a)
{
        /* Check that UArray2 a is not NULL */
        assert(a != NULL);
        UArray_free(&(*a)->array);
        free(*a);
}

/*************UArray2_map_row_major***********************************
*
* Traverses the elements in the UArray2 by row and calls the apply         
* function for each element. 
*
* Parameters: UArray2_T a: a UArray2 that has been initialized 
*          void apply: a function pointer for a function that will 
*          be called for each element that the map function         
*          accesses. The apply function should take at least a row  
*          i, a column j and the UArray2_T. 
*          void *c: a closure statement for the map function 
*
* Return: nothing
*
* Expects that the UArray2_T is not a NULL pointer. 
*      
* Notes: under the hood, our UArray2_T implementation will be a one 
* dimensional array. Raises checked runtime error if UArray2 is not NULL.
*      
*********************************************************************/
void UArray2_map_row_major(UArray2_T a, void apply(int i, int j, UArray2_T a, 
                            void *p1, void *p2), void *cl)
{
        /* Check that UArray2 a is not NULL */
        assert(a != NULL);
        void *p1;

        for (int rows = 0; rows < a->height; rows++){
                for (int cols = 0; cols < a->width; cols++){
                        p1 = UArray2_at(a, cols, rows);
                        apply(cols, rows, a, p1, cl);
                }
        }
}

/*************UArray2_map_col_major***********************************
*
* Traverses the elements in the UArray2 by column and calls the apply     
* function for each element. 
*
* Parameters: UArray2_T a: a UArray2 that has been initialized 
*          void apply: a function pointer for a function that will    
*          be called for each element that the map function             
*          accesses. The apply function should take at least a row        
*          i, a column and the UArray2_T. 
*          void *c: a closure statement for the map function 
*
* Return: nothing
*
* Expects that the UArray2_T is not a NULL pointer. 
*      
* Notes: under the hood, our UArray2_T implementation will be a one 
* dimensional array. Raises checked runtime error if the UArray2 is NULL. 
*      
*********************************************************************/

void UArray2_map_col_major(UArray2_T a, void apply(int i, int j, UArray2_T a, 
                            void *p1, void *p2), void *cl)
{
        /* Check that UArray 2 a is not NULL */
        assert(a != NULL);
        void *p1;

        for (int cols = 0; cols < a->width; cols++){
                for (int rows = 0; rows < a->height; rows++){
                        p1 = UArray2_at(a, cols, rows);
                        apply(cols, rows, a, p1, cl);
                }
        }
}