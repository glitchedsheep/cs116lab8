/*
 *     bit2.h
 *     by Rolando Ortega (rorteg02) Jason Singer (jsinge02), 2/11/2024
 *     iii
 *
 *     A two dimensional Bit has the ability to store data using the index 
 *     (column, row). Clients can create a new UArray2 that has the ability to 
 *     get elements within the 2-D array, get the array's height, width, and 
 *     element size, and traverse elements in the array by rows and column.  
 */

#define T Bit2_T
typedef struct T *T;

int Bit2_get(Bit2_T b, int col, int row);
int Bit2_put(Bit2_T b, int col, int row, int bit);
int Bit2_height(Bit2_T a);
int Bit2_width(Bit2_T a);
Bit2_T Bit2_new(int DIM1, int DIM2);
void Bit2_map_col_major(Bit2_T a, void apply(int i, int j, Bit2_T a, int b, 
    void *p1), void *c);
void Bit2_map_row_major(Bit2_T a, void apply(int i, int j, Bit2_T a, int b, 
    void *p1), void *c);
void Bit2_free(Bit2_T *a);
int getIndex(int row, int column);

#undef T
