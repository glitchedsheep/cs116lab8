/*
 *     unblackedges.c
 *     by Jason Singer (jsinge02) Rolando Ortega (rorteg02), 2/11/2024
 *     iii
 *
 *     Removes black edge pixels from a black and white image and prints out 
 *     fixed image to standard output. 
 */

#include "bit2.h"
#include <except.h>
#include <assert.h>
#include <stdio.h>
#include <pnmrdr.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stack.h>

/* Exception */
Except_T Wrong_Args = { "Too many arguments" };

/****************************************************************************
 * 
 *                        FUNCTION DECLARATIONS
 * 
*****************************************************************************/
Bit2_T read_file(FILE *fp);
void map_black_edges(Bit2_T bitmap);
void print(Bit2_T bitmap);
void unblack(Bit2_T bitmap, int col, int row);
void unblack_up(Bit2_T, int col, int row, Stack_T bitStack);
void unblack_down(Bit2_T, int col, int row, Stack_T bitStack);
void unblack_left(Bit2_T, int col, int row, Stack_T bitStack);
void unblack_right(Bit2_T, int col, int row, Stack_T bitStack);

struct gridEntry {
        int row;
        int col;
        int val;
};

/********************main*********************************************
*
* Takes the name of a file given either on the command line or from standard 
* input and opens it. It then calls functions to read the contents of the file
* into a Bit2, and then calls functions to remmove black edges from the file and
* printing the new file out. 
*
* Parameters: int argc: the number of arguments from the command line
*             char *argv[]: the arguments from the command line
*
* Return: Returns EXIT_SUCCESS if the programm runs to completion
*
* Expects: Expects that only two arguments are given and that the file tht is 
            provided from either the command line or standard input is valid.
*      
* Notes: Raises checked runtime errors if more than two arguments are given or
*        the file can not be opened. Calls the read_file function to read in 
*        the contents of the file into an Bit2, calls map_black_edges to begin
*        the process of removing black edges, and print to print out the fixed
*        contents out to standard output 
*****************************************************************************/
int main(int argc, char *argv[]) 
{
        /* Check if correct number of arguments were given */
        if (argc > 2) {
                RAISE(Wrong_Args);
        }

        /* Create a new file pointer and open the provided file */
        FILE *fp = NULL;

        if (argc == 2) {
                fp = fopen(argv[1], "r");
        }
        else if (argc == 1){
                fp = stdin;
        }

        assert(fp != NULL);

        /* Read in file to a Bit2 and close the file */
        Bit2_T bitmap = read_file(fp);
        fclose(fp);

        /* Check the black edges and print contents of fixed file */
        map_black_edges(bitmap);
        print(bitmap);
        
        /* Free Bit2 and exit program */
        Bit2_free(&bitmap);
        return EXIT_SUCCESS;
}

/********************read_file*************************************************
*
* This function takes an opened pbm file and reads its contents into a Pnmrdr
* reader. It then transfers the data in the file from the Pnmrdr reader into a 
* new Bit2 data structure. 
*
* Parameters: File *fp: a pointer to an open file. 
*
* Return: a Bit2_T that contains the contents in the file. 
*
* Expects: Expects that the file provided is not NULL and that the file is not 
           empty. 
*      
* Notes: The function raise a CRE if the provided pgm file has a width or 
*        height of 0 The Pnmrdr reader will also produce a checked 
*        runtime error if there is a character in the board that is not an 
*        integer, or of the file is not a pgm. 
*      
*****************************************************************************/
Bit2_T read_file(FILE *fp)
{
        struct Pnmrdr_T *pnm = Pnmrdr_new(fp);

        Pnmrdr_mapdata mapdata= Pnmrdr_data(pnm);
        /*raise CRE if not a pcg\m file*/
        assert(mapdata.type == 1);
        assert(!(mapdata.width == 0 || mapdata.height == 0));

        Bit2_T new = Bit2_new(mapdata.width, mapdata.height);

        for (unsigned int row = 0; row < mapdata.height; row++) {
                for (unsigned int col = 0; col < mapdata.width; col++) {
                        Bit2_put(new, col, row, Pnmrdr_get(pnm));
                }   
        }

        free(pnm);

        return new;
}

/********************map_black_edgges******************************************
*
* This function maps around the outer edges of the Bit2 from the original file, 
* calling unblack() to check provided indicies and black edges that are deeper 
* in the file. 
*
* Parameters: Bit2_T bitmap: a Bit2 containing data from the file. 
*
* Return: nothing. 
*
* Expects: expects the provided bitmap is not NULL
*      
* Notes: function calls unblack() on the indicies of each edge index
*      
*****************************************************************************/
void map_black_edges(Bit2_T bitmap)
{
        assert(bitmap != NULL);
        /*check horizontal edges */
        for (int i = 0; i < Bit2_width(bitmap); i++){
                unblack(bitmap, i, 0);
                unblack(bitmap, i,  Bit2_height(bitmap) - 1);
        }
        
        /*check vertical edges */
        for (int j = 0; j < Bit2_height(bitmap); j++){
                unblack(bitmap, 0, j);
                unblack(bitmap, Bit2_width(bitmap) - 1, j);
        }
}

/********************unblack*********************************************
*
* This function uses an interative depth-first search approach to flip all
* black edge entries in a Bit2_T into white (0) entries. This function is 
* called on the indicies of each entry in the four edges of the bitmap.
*
*
* Parameters: Bit2_t bitmap: a 2d bitmap
*             int col: the column the of the root index where the DFS starts
*             int row: the row the of the root index where the DFS starts
*          
*
* Return: Function returns nothing, but does update the bitmap to remove black
*          edges
*
* Expects: Expects that the bitmap is a valid pgm file. This has been asserted
*          in the read_file function. This function also expects to be called
*          on the edges of a bitmap in order to fully check all black edges
*      
* Notes: This function has four helper functions that push four adjacent
*        indicies to a black edge bit currently popped off the stack (may be 
*        < 4  if adjacent indicies are out of bounds). Allocates memory for 
*        each bit,   which are represented in the function as pointers to the
*        gridEntry  struct. Memory is freed when each bit has been popped off 
*        the stack  and adjacent black indicies have been pushed onto it, and 
*        the stack itself is freed when all indicies adjacent to black edge 
*        have been searched.
*      
*****************************************************************************/
void unblack(Bit2_T bitmap, int col, int row)
{       
        /*our iterative DFS utilizes a Hansen stack*/
        Stack_T bitStack = Stack_new();

        /* Create a gridEntry from the bit at the given column and row */
        struct gridEntry *root = malloc(sizeof(struct gridEntry));
        assert(root != NULL);
        root->row = row;
        root->col = col;
        root->val = Bit2_get(bitmap, col, row);

        Stack_push(bitStack, root);

        /* Look through each bit that is on the stack */
        while (!Stack_empty(bitStack)){
                struct gridEntry *bit = Stack_pop(bitStack);
                /*If the bit is black, push its four adjacent entries (if 
                possible) onto the stack. If the bit is white, simply free 
                the bit and check the next entry*/
                if (bit->val == 1){
                        int thisCol = bit->col;
                        int thisRow = bit->row;
                        Bit2_put(bitmap, bit->col, bit->row, 0);
                        unblack_right(bitmap, thisCol, thisRow, bitStack);
                        unblack_down(bitmap, thisCol, thisRow, bitStack);
                        unblack_left(bitmap, thisCol, thisRow, bitStack);
                        unblack_up(bitmap, thisCol, thisRow, bitStack);
                }
                free(bit);
        }
        /* Free the stack */
        Stack_free(&bitStack);
}

/********************print*********************************************
* This function iterates through the bitmap and prints each integer in row 
* major order, as well as a pgm header


* Parameters: Bit2_t bitmap: a 2d bitmap representing a pgm file and 
* is not NULL    
*
* Return: void
*
* Expects: the provided bitmap represents the contents of a pgm file and is not
*           NULL. 
*      
* Notes: Raises checked runtime error if bitmap is NULL
*****************************************************************************/
void print(Bit2_T bitmap)
{
        /* Check that bitmap is not NULL */
        assert(bitmap != NULL);
        printf("P1\n%d %d\n", Bit2_width(bitmap), Bit2_height(bitmap));
        for (int row = 0; row < Bit2_height(bitmap); row++){
                for (int col = 0; col < Bit2_width(bitmap); col++){
                        printf("%d ", Bit2_get(bitmap, col, row));
                }
                printf("\n");
        }
}

/********************unblack_right*********************************************
* This function adds the bit to the right of the bit at the given column and
* row.
* 
* Parameters: Bit2_t bitmap: a 2d bitmap representing a pgm file and 
*               is not NULL
*             int thisCol: a column for a bit in bitmap
*             int thisRow: a row for a bit in bitmap
*             Stack_T bitStack: a stack of bits being used for the DFS search  
*
* Return: void
*
* Expects: the provided bitmap represents the contents of a pgm file and is not
*           NULL.
*      
* Notes: Raises checked runtime error if bitmap is NULL 
*****************************************************************************/
void unblack_right(Bit2_T bitmap, int thisCol, int thisRow, Stack_T bitStack)
{
        /* Check that bitmap is not NULL */
        assert(bitmap != NULL);

        /* Check that going right is still in bounds */
        if ((thisCol + 1 < Bit2_width(bitmap))){
                struct gridEntry *nextBit = malloc(sizeof(struct gridEntry));
                assert(nextBit != NULL);
                nextBit->row = thisRow;
                nextBit->col = (thisCol + 1);
                nextBit->val = Bit2_get(bitmap, thisCol + 1, thisRow);
                Stack_push(bitStack, nextBit);
        }
}

/********************unblack_down*********************************************
* This function adds the bit below the bit at the given column and row to the 
* stack
* 
* Parameters: Bit2_t bitmap: a 2d bitmap representing a pgm file and 
*               is not NULL
*             int thisCol: a column for a bit in bitmap
*             int thisRow: a row for a bit in bitmap
*             Stack_T bitStack: a stack of bits being used for the DFS search   
*
* Return: void
*
* Expects: the provided bitmap represents the contents of a pgm file and is not
*           NULL.
*      
* Notes: Raises checked runtime error if bitmap is NULL 
*****************************************************************************/
void unblack_down(Bit2_T bitmap, int thisCol, int thisRow, Stack_T bitStack)
{
        /* Check that bitmap is not NULL */
        assert(bitmap != NULL);

        /* Check that going down is still in bounds */
        if ((thisRow + 1 < Bit2_height(bitmap))){
                struct gridEntry *nextBit = malloc(sizeof(struct gridEntry));
                assert(nextBit != NULL);
                nextBit->row = (thisRow + 1);
                nextBit->col = thisCol;
                nextBit->val = Bit2_get(bitmap, thisCol, thisRow + 1);
                Stack_push(bitStack, nextBit);
        }  
}

/********************unblack_left*********************************************
* This function adds the bit to the left of the bit at the given column and
* row to the stack.
* 
* Parameters: Bit2_t bitmap: a 2d bitmap representing a pgm file and 
*               is not NULL
*             int thisCol: a column for a bit in bitmap
*             int thisRow: a row for a bit in bitmap
*             Stack_T bitStack: a stack of bits being used for the DFS search   
*
* Return: void
*
* Expects: the provided bitmap represents the contents of a pgm file and is not
*           NULL.
*      
* Notes: Raises checked runtime error if bitmap is NULL 
*****************************************************************************/
void unblack_left(Bit2_T bitmap, int thisCol, int thisRow, Stack_T bitStack)
{
        /* Check that bitmap is not NULL */
        assert(bitmap != NULL);

        /* Check that going left is still in bounds */
        if (thisCol - 1 > 0){
                struct gridEntry *nextBit = malloc(sizeof(struct gridEntry));
                assert(nextBit != NULL);
                nextBit->row = thisRow; 
                nextBit->col = (thisCol - 1);
                nextBit->val = Bit2_get(bitmap, thisCol - 1, thisRow);
                Stack_push(bitStack, nextBit);
        }       
}

/********************unblack_up*********************************************
* This function adds the bit above the bit at the given column and row to the
* stack 
* 
* Parameters: Bit2_t bitmap: a 2d bitmap representing a pgm file and 
*               is not NULL
*             int thisCol: a column for a bit in bitmap
*             int thisRow: a row for a bit in bitmap
*             Stack_T bitStack: a stack of bits being used for the DFS search  
*
* Return: void
*
* Expects: the provided bitmap represents the contents of a pgm file and is not
*           NULL.
*      
* Notes: Raises checked runtime error if bitmap is NULL 
*****************************************************************************/
void unblack_up(Bit2_T bitmap, int thisCol, int thisRow, Stack_T bitStack)
{
        /* Check that bitmap is not NULL */
        assert(bitmap != NULL);

        /* Check that going up is still in bounds */
        if (thisRow - 1 > 0){
                struct gridEntry *nextBit = malloc(sizeof(struct gridEntry));
                assert(nextBit != NULL);
                nextBit->row = (thisRow - 1);
                nextBit->col = thisCol;
                nextBit->val = Bit2_get(bitmap, thisCol, thisRow - 1);
                Stack_push(bitStack, nextBit);
        }
}