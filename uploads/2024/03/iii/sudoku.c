/*
 *     sudoku.c
 *     by Rolando Ortega (rorteg02) Jason Singer (jsinge02), 2/11/2024
 *     iii
 *
 *     Checks whether a file containing a sudoku board provided by the client 
 *     is a valid sudoku solution. 
 */

#include "uarray2.h"
#include <except.h>
#include <assert.h>
#include <stdio.h>
#include <pnmrdr.h>
#include <set.h>
#include <atom.h>
#include <stdlib.h>
#include <stdbool.h>

/****************************************************************************
 *
 *                  EXCEPTIONS AND GLOBAL VARIABLES
 *  
*****************************************************************************/
Except_T Wrong_Args = { "Too many arguments" };
Set_T oneThroughNine;

/****************************************************************************
 * 
 *                        FUNCTION DECLARATIONS
 * 
*****************************************************************************/
UArray2_T read_file(FILE *fp);
void check_sudoku(UArray2_T sudokuBoard);
void map_box_major(UArray2_T sudokuBoard, void check_squares(int col, int row, 
                    UArray2_T a, void *number, void *cl), int *timesCalled);
void check_squares(int col, int row, UArray2_T a, void *number, void *cl);

/********************main*********************************************
*
* Takes the name of a file given either on the command line or from standard 
* input and opens it. In then calls functions to read the sudoku board in the
* file into a UArray2 and runs a function to check the validity of the board
*
* Parameters: int argc: the number of arguments from the command line
*             char *argv[]: the arguments from the command line
*
* Return: Returns EXIT_SUCCESS if the programm runs without any functions 
*         returning EXIT_FAILURE. 
*
* Expects: Expects that only two arguments are given and that the file tht is 
            provided from either the command line or standard input is valid.
*      
* Notes: Raises checked runtime errors if more than two arguments are given or
*        the file can not be opened. Calls the read_file function to read in 
*        the contents of the file into an UArray2 and calls check_sudoku to 
*        check the board within the file 
*      
*****************************************************************************/
int main(int argc, char *argv[]){
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
        
        /* Read in the contents from the file and close*/
        UArray2_T sudokuBoard = read_file(fp);
        fclose(fp);
        
        /* Check the sudoku board in the UArray2*/
        check_sudoku(sudokuBoard);
        /* Free UArray2 and return EXIT_SUCCESS */
        UArray2_free(&sudokuBoard);
        return EXIT_SUCCESS;
}

/********************read_file*************************************************
*
* This function takes an opened sudoku file and reads its contents into a Pnmrdr
* reader. It then transfers the data of the sudoku board in the file from the 
* Pnmrdr reader into a new UArray2 data structure. 
*
* Parameters: File *fp: a pointer to an open file. 
*
* Return: a UArray2_T that contains the contents of the sudoku board in the 
*         given file. 
*
* Expects: Expects that the file provided is not NULL and that the sudoku board
*          in that file is properly formatted (i.e. is a 9x9 board with only
           integer characters). 
*      
* Notes: The function will exit with EXIT_FAILURE if the dimensions of the 
*        sudoku board are not 9x9 as expected. The Pnmrdr reader will also 
*        produce a checked runtime error if there is a character in the board 
*        that is not an integer, or of the file is not a pgm. 
*      
*****************************************************************************/
UArray2_T read_file(FILE *fp)
{
        struct Pnmrdr_T *pnm = Pnmrdr_new(fp);

        Pnmrdr_mapdata mapdata= Pnmrdr_data(pnm);
        /*raise CRE if not a pgm file*/
        assert(mapdata.type == 2);
        if (mapdata.width != 9 || mapdata.height != 9) {
                fclose(fp);
                exit(EXIT_FAILURE);
        }

        UArray2_T new = UArray2_new(mapdata.width, mapdata.height, sizeof(int));

        for (unsigned int row = 0; row < mapdata.height; row++) {
                for (unsigned int col = 0; col < mapdata.width; col++) {
                        *((int *)UArray2_at(new, col, row)) = Pnmrdr_get(pnm);
                }   
        }

        free(pnm);

        return new;
}

/*************check_sudoku**********************************************
*
* Function that calls three mapping functions to check the validity of the
* sudoku board, in row, column, and box order. 
*
* Parameters: UArray2_T sudokuBoard: a 9x9 UArray2 with values 1-9
*
* Return: void
*
* Expects The UArray2 has dimensions of 9x9 and only contains inputs 1-9
*
* Notes: The specific conditions the UArray2 must conform to have been checked
* in the read_file funciton to ensure valid input. This funciton also 
* initializes a Hansen Set of atoms. 
*      
********************************************************************/
void check_sudoku(UArray2_T sudokuBoard)
{
        int timesCalled = 0;
        oneThroughNine = Set_new(9, NULL, NULL);

        UArray2_map_row_major(sudokuBoard, check_squares, &timesCalled);

        UArray2_map_col_major(sudokuBoard, check_squares, &timesCalled);

        map_box_major(sudokuBoard, check_squares, &timesCalled);
        Set_free(&oneThroughNine);
}

/*************map_box_major**********************************************
*
* Function that calls an apply functionn on each index of a sudoku-formatted
* UArray2 in box order. We define box order as iterating through the nine
* 3x3 boxes that a 9x9 array can be broken up into
*
*             meters: UArray2_T : a function that is mapped onto each
*                                 index of the UArray2
*             int *timesCalled: a closure argument that tracks the number of 
*                               times check_squares is called                   
*
v Return:oid a void pointer to the element in the specified index
*
* The UArray2 has dimensions of 9x9 and only contains inputs 1-9*  
*        and the UArray2 is not NULL
*      
* Notes: check_squares is the only function passed as a paramenter in this 
* mapping function throughout the program, any function could be passed in
* as long as the UArray2 is 9x9
*      
********************************************************************/
void map_box_major(UArray2_T sudokuBoard, void check_squares(int col, int row, 
                    UArray2_T a, void *number, void *cl), int *timesCalled)
{
        int row = 0;
        int column = 0;
        /*outer loop iterates through boxe*/
        for (int boxes = 1; boxes < 10; boxes++){
                
                /*Inner nested loop iterates through 3x3 box*/
                for (int i = row; i < row + 3; i++){
                        for (int j = column; j < column + 3; j++){
                                check_squares(j, i, sudokuBoard, 
                                UArray2_at(sudokuBoard, j, i), timesCalled);
                        }
                }
                column += 3;
                /*Every three boxes, reset to col 0 and move down 3 rows*/
                if (boxes % 3 == 0){
                        row += 3;
                        column = 0;
                }
        }

}

/********************check_squares*********************************************
*
* This function checks each square in sudoku board using a set, and if nine 
* numbers have been checked from the board, it checks the set to see if the 
* row, column, or box in that set is valid (a valid set should have 9 numbers
* after nine checks). The set we are checking is initialized as a global 
* variable. 
*
* Parameters: int col: a column in the sudoku board 
*             int row: a row in the sudoku board 
*             UArray2_T a: a pointer to the UArray2 that contains the contents
                           of the sudoku board. 
*             void *number: a pointer to a number in the sudoku board we are 
                            checking.
*             void *c1: closure statement, the number of times the function has
                        been called. 
*
* Return: Function returns nothing, but does update the set and the value in 
          timesCalled. 
*
* Expects: Expects that the UArray2_T is a valid sudoku board and that the 
           number given is a valid number for the sudoku board. Also expects 
           that the set has been already initialized in a previous function. 
*      
* Notes: The function will exit with EXIT_FAILURE if either the given number is
         an invalid number for a sudoku board or if the set is not of length 9
         after 9 calls. This function is the apply function that is called on
         by the different mapping functions. 
*      
*****************************************************************************/
void check_squares(int col, int row, UArray2_T a, void *number, void *cl)
{
        (void) col;
        (void) row;
        int *timesCalled = cl;
        /* checks valid sudoku numbers (only integers 1 - 9) */
        if ((*(int *) number > 9) || (*(int *) number < 1)) {
                Set_free(&oneThroughNine);
                UArray2_free(&a);
                exit(EXIT_FAILURE);
        }
        const char *num = Atom_string((char *)number);
        Set_put(oneThroughNine, num);
        *timesCalled += 1;
        
        /* When timesCalled is 9, a row/col/box has been iterated through */
        if (*timesCalled == 9){
                /*If the set length is not 9, there was a repeat in the 
                row/box/col and the sudoku board isn't solved*/
                if (Set_length(oneThroughNine) != 9){
                        Set_free(&oneThroughNine);
                        UArray2_free(&a);
                        exit(EXIT_FAILURE);
                }
                /*If the row/col/box was valid, reset the set and 
                change timesCalled back to 0*/
                Set_free(&oneThroughNine);
                *timesCalled = 0;
                oneThroughNine = Set_new(9, NULL, NULL);
        }
}

