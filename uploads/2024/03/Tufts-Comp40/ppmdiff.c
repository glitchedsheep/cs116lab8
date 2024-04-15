
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"

double computeE(int height, int width, Pnm_ppm image1, Pnm_ppm image2);

/* 
 * Steps:
 *   1. Read in arguments from command line, one file may be on stdin
 *   2. Read into ppm
 *   3. Check that width and height differ by 1, returning an error 
 *      otherwise
 *   4. Compute summation using nested for loops
 *   3. Print result to standard output with 4 digits after decimal point
 */

int main(int argc, char *argv[])
{
    assert(argc == 3);
    FILE *file_p1, *file_p2;
    bool stdinProvided = false;

    /* read in first file */
    if (*argv[1] == '-') {
        file_p1 = stdin;
        stdinProvided = true;
    } else {
        file_p1 = fopen(argv[1], "r");
    }

    /* read in second file */
    if (*argv[2] == '-' && stdinProvided) {
        fprintf(stderr, "Only one file may be provided through stdin\n");
        exit(1);
    } else if (*argv[2] == '-'){
        file_p2 = stdin;
    } else {
        file_p2 = fopen(argv[2], "r");
    }

    assert(file_p1 != NULL && file_p2 != NULL);
    
    /* default to UArray2 methods */
    A2Methods_T methods = uarray2_methods_plain; 
    assert(methods != NULL);
    
    /* read images into ppm, get their widths and heights */
    Pnm_ppm image1 = Pnm_ppmread(file_p1, methods);
    Pnm_ppm image2 = Pnm_ppmread(file_p2, methods);
    int width1 = image1->width;
    int width2 = image2->width;
    int height1 = image1->height;
    int height2 = image2->height;
    
    /* ensure that their heights and width differ by no more than 1 */
    if (abs(width1 - width2) > 1 || abs(height1 - height2) > 1) {
        printf("1.0\n");
        fprintf(stderr, "height and width differ by more than 1 pixel\n");
        exit(1);
    }
    
    /* using the smaller of the two widths and heights, compute E */
    int width = (width1 < width2) ? width1 : width2;
    int height = (height1 < height2) ? height1 : height2;
    double e = computeE(height, width, image1, image2);
    
    printf("%0.4f\n", e);
    
    return 0;
}


double computeE(int height, int width, Pnm_ppm image1, Pnm_ppm image2)
{
    assert(image1 != NULL && image2 != NULL);
    assert(image1->methods != NULL && image2->methods != NULL);
    assert(height >= 0 && width >= 0);
    
    double totalE = 0;
    
    for(int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            Pnm_rgb pixel1 = (Pnm_rgb)image1->methods->at(image1->pixels, i, j);
            Pnm_rgb pixel2 = (Pnm_rgb)image2->methods->at(image2->pixels, i, j);
            
            double redDiff   = ((float)pixel1->red)/image1->denominator - 
                               ((float)pixel2->red)/image2->denominator;
            double greenDiff = ((float)pixel1->green)/image1->denominator - 
                               ((float)pixel2->green)/image2->denominator;
            double blueDiff  = ((float)pixel1->blue)/image1->denominator - 
                               ((float)pixel2->blue)/image2->denominator;
  
            totalE += pow(redDiff, 2) + pow(greenDiff, 2) + pow(blueDiff, 2);
        }
    }
    
    totalE = sqrt((totalE/(double)(3 * width * height)));
    
    return totalE;
}
