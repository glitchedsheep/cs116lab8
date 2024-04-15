/*
 *     uarray2.h
 *     by Rolando Ortega (rorteg02) Jason Singer (jsinge02), 2/11/2024
 *     iii
 *
 *     A two dimensional unboxed array has the ability to store data using the 
 *     index (column, row). Clients can create a new UArray2 that has the 
 *     ability to get elements within the 2-D array, get the array's height, 
 *     width, and element size, and traverse elements in the array by rows and
 *     column.  
 */

#define T UArray2_T
typedef struct T *T;
 
void *UArray2_at(UArray2_T a, int col, int row);
int UArray2_height(UArray2_T a);
int UArray2_width(UArray2_T a);
int UArray2_size(UArray2_T a);
UArray2_T UArray2_new(int DIM1, int DIM2, int ELEMENT_SIZE);
void UArray2_map_col_major(UArray2_T a, void apply(int i, int j, UArray2_T a, 
        void *p1, void *p2), void *cl);
void UArray2_map_row_major(UArray2_T a, void apply(int i, int j, UArray2_T a, 
        void *p1, void *p2), void *c);
void UArray2_free(UArray2_T *a);

#undef T
