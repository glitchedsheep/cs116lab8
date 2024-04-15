/*
 *     ppmtrans.c
 *     by Rolando Ortega (rorteg02) Jason Singer (jsinge02), 2/11/2024
 *     locality
 *
 *     ppmtrans transforms images provided by the client. Clients can either 
 *     rotate 0, 90, 180, or 270 degrees, flip the image vertically, or 
 *     transpose the image. Transformations are timed and, if the client wishes
 *     to see the timed results for a transformation, can provide an output
 *     file to append these results to. The client can specify how these 
 *     transformation are done, i.e. by row, column, or block major. This 
 *     program relies on the a2methods and 2plain methods suites as well as 
 *     pnm.h interface to handle file reading and writing. Runtime errors are
 *     raised for improper inputs. 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"

void start_transform(FILE *picFile, int rotation, char *time_file, 
        A2Methods_mapfun* map, A2Methods_T methods, char *otherTrans, 
        char *direction);
void rotate_image_setup(Pnm_ppm image, int rotation, char *time_file, 
        A2Methods_mapfun* map);
A2Methods_UArray2 rotation_options(int rotation, Pnm_ppm image, 
        A2Methods_mapfun* map, CPUTime_T timer, double *timeTaken);
void rotate_180(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source);
void rotate_90(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source);
void rotate_270(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source);
void other_transformations(Pnm_ppm image, char* transformation, 
        char *time_file, A2Methods_mapfun* map);
/* void flip_horizontal(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source); */
void flip_vertical(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source);
void transpose(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source);
void time_handle(char *timeFile, double timeTaken, Pnm_ppm image);

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)

static void usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] "
                        "[-time time_file] "
                        "[filename]\n",
                        progname);
        exit(1);
}

/*************rotate_180***********************************
*
* Apply function that rotates image 180 degrees, copying rotated pixels
* into a destination array

* Parameters: int sourceCol: column index of the source array
*             int sourceRow: row index of the source array
*             A2Methods_UArray2 array: destination array
*             void *elem: element of the destination array
*             void *source: void * to pnm_pppm of source image
*
* Expects: destination array is same dimensions of source array, source is
* not NULL
*
* Return: nothing, but function affects elements of destination array
*********************************************************************/
void rotate_180(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void *source)
{
        (void) array;
        /* Check source is valid */
        assert(source != NULL);
        Pnm_ppm image = source;
        
        /* Transform coordinates */
        const struct A2Methods_T *methods = image->methods;
        int height = methods->height(image->pixels);
        int width = methods->width(image->pixels);
        *(Pnm_rgb)elem = *(Pnm_rgb)methods->at(image->pixels, 
                width - sourceCol - 1, height - sourceRow - 1);
}

/*************rotate_90**********************************
*
* Apply function that rotates image 90 degrees, copying rotated pixels
* into a destination array

* Parameters: int sourceCol: column index of the source array
*             int sourceRow: row index of the source array
*             A2Methods_UArray2 array: destination array
*             void *elem: element of the destination array
*             void *source: void * to pnm_pppm of source image
*
* Expects: destination array is opposite dimensions of source array, source is
* not NULL
*
* Return: nothing, but function affects elements of destination array
*********************************************************************/
void rotate_90(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source)
{
        (void) array;
        /* Check source is not NULL */
        assert(source != NULL);
        Pnm_ppm image = source;

        /* Update coordinates */
        const struct A2Methods_T *methods = image->methods;
        int height = methods->height(image->pixels);
        *(Pnm_rgb)elem = *(Pnm_rgb)methods->at(image->pixels, 
                sourceRow, height - sourceCol - 1);
}

/*************rotate_270**********************************
*
* Apply function that rotates image 270 degrees, copying rotated pixels
* into a destination array

* Parameters: int sourceCol: column index of the source array
*             int sourceRow: row index of the source array
*             A2Methods_UArray2 array: destination array
*             void *elem: element of the destination array
*             void *source: void * to pnm_pppm of source image
*
* Expects: destination array is opposite dimensions of source array, source is
* not NULL
*
* Return: nothing, but function affects elements of destination array
*********************************************************************/
void rotate_270(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source)
{
        (void) array;
        /* Check source is not NULL */
        assert(source != NULL);
        Pnm_ppm image = source;

        /* Update coordinates */
        const struct A2Methods_T *methods = image->methods;
        int newWidth = methods->width(image->pixels);
        *(Pnm_rgb)elem = *(Pnm_rgb)methods->at(image->pixels, 
                newWidth - sourceRow - 1, sourceCol);
}

/*************flip_vertical**********************************
*
* Apply function that flips image vertically, copying rotated pixels
* into a destination array

* Parameters: int sourceCol: column index of the source array
*             int sourceRow: row index of the source array
*             A2Methods_UArray2 array: destination array
*             void *elem: element of the destination array
*             void *source: void * to pnm_pppm of source image
*
* Expects: destination array is same dimensions of source array, source is
* not NULL
*
* Return: nothing, but function affects elements of destination array
*********************************************************************/
void flip_vertical(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source)
{
        (void) array;
        /* Check source is not NULL */
        assert(source != NULL);
        Pnm_ppm image = source;

        /* Update coordinates */
        const struct A2Methods_T *methods = image->methods;
        int height = methods->height(image->pixels);
        *(Pnm_rgb)elem = *(Pnm_rgb)methods->at(image->pixels, 
                sourceCol, height - sourceRow - 1);   
}

/***********************************************
* Apply function that flips image horizontally, copying rotated pixels
* into a destination array

* Parameters: int sourceCol: column index of the source array
*             int sourceRow: row index of the source array
*             A2Methods_UArray2 array: destination array
*             void *elem: element of the destination array
*             void *source: void * to pnm_pppm of source image
*
* Expects: destination array is same dimensions of source array, source is
* not NULL
*
* Return: nothing, but function affects elements of destination array
* Notes: WE COULD NOT GET THIS FUNCTION TO WORK CORRECTLY, AND IS NOT A PART
OF OUR FINAL SUBMISSION. 
**************************************************************************
void flip_horizontal(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source)
{
        (void) array;
        Pnm_ppm image = source;
        const struct A2Methods_T *methods = image->methods;
        int width = methods->width(image->pixels);
        *(Pnm_rgb *)elem = *(Pnm_rgb *)methods->at(image->pixels, 
                width - sourceCol - 1, sourceRow);   
}
*/

/*************transpose**********************************
*
* Apply function that transposes the imag, copying rotated pixels
* into a destination array

* Parameters: int sourceCol: column index of the source array
*             int sourceRow: row index of the source array
*             A2Methods_UArray2 array: destination array
*             void *elem: element of the destination array
*             void *source: void * to pnm_pppm of source image
*
* Expects: destination array is opposite of source array, source is
* not NULL
*
* Return: nothing, but function affects elements of destination array
*********************************************************************/
void transpose(int sourceCol, int sourceRow, A2Methods_UArray2 array, 
        void *elem, void* source)
{
        (void) array;
        /* Check that source is not NULL */
        assert(source != NULL);
        Pnm_ppm image = source;

        /* Update coordinates */
        const struct A2Methods_T *methods = image->methods;
        *(Pnm_rgb)elem = *(Pnm_rgb)methods->at(image->pixels, 
                sourceRow, sourceCol);   
}


/*************rotate_image_setup**********************************
*
* Helper function that sets the destination array, timer for rotations, 
* and calls rotatation_options for further rotation handeling

* Parameters: Pnm_ppm image: Pnm_ppm struct pointer storing the methods and 
*                             source image in an array
*             int rotation: the number of degrees to rotate by
*             char *time_file: the name of a file to output time data to 
*             A2Methods_mapfun* map: the mapping function to be used for a 
*                   transformation
*             
*
* Expects: image is not NULL
*
* Return: nothing, but function will have changed the image to store the 
* rotated image
*********************************************************************/
void rotate_image_setup(Pnm_ppm image, int rotation, char *time_file, 
        A2Methods_mapfun* map)
{
        /* Check image is valid */
        assert(image != NULL); 
        CPUTime_T timer = CPUTime_New();

        double timeTaken;

        /* Rotate and update image */
        A2Methods_UArray2 destination = rotation_options(rotation, image, map, 
                timer, &timeTaken);
        image->pixels = destination;
        time_handle(time_file, timeTaken, image);
    
        CPUTime_Free(&timer);
}

/*****************rotation_options*****************************************
*
* Function that carries out timing of rotation and handles which rotation to
* call, depending on command line input. Rotation of 0 degrees is default
* 
* Parameters:   int rotation: the degrees of the rotation represented as an int
*               Pnm_ppm image: the data for the given image
*               A2Methods_mapfun* map: the mapping function to be used for a
*                                      transformation
*               CPUTime_T timer: timer object that prefroms timing of rotation
*               double *timeTaken: double pointer to variable storing the time
*                                 a rotation takes
*               A2Methods_mapfun* map: the mapping function to be used for a 
*                                transformation
*           
* Return: An A2Methods_UArray2 of the rotated image. Also updates the timeTaken
*         double to be used in time handling
*
* Expects: none of the arguments, should be NULL which have been asserted in
*          main and rotate_image_setup. 
*      
* Notes: Relies on the provided map function to carry out the transformation,
*        with the transformation functions being apply functions. Frees
*        memory for the timer and the original image.
*      
******************************************************************************/
A2Methods_UArray2 rotation_options(int rotation, Pnm_ppm image, 
        A2Methods_mapfun* map, CPUTime_T timer, double *timeTaken)
{
        /* Get image data */
        const struct A2Methods_T *methods = image->methods;
        int destHeight = methods->height(image->pixels);
        int destWidth = methods->width(image->pixels);
        int destSize = methods->size(image->pixels);
        A2Methods_UArray2 destination;
        
        /* Carry out transformation*/
        CPUTime_Start(timer);
        if (rotation == 0){
                *timeTaken = CPUTime_Stop(timer);
                return image->pixels;
        }  
        if (rotation == 180){
                destination = methods->new(destWidth, destHeight, destSize);
                map(destination, rotate_180, image);
        } else {
                destination = methods->new(destHeight, destWidth, destSize);
                if (rotation == 90) {
                        map(destination, rotate_90, image);
                } else {
                        map(destination, rotate_270, image);
                }
                image->height = image->methods->height(destination);
                image->width = image->methods->width(destination);
        }
        /* Get time data, free source image and return the new one */
        *timeTaken = CPUTime_Stop(timer);
        image->methods->free(&(image->pixels));
        return destination;
}

/*****************other_transformations*****************************************
*
* Function that carries out either the flip vertical or transpose 
* transformations. 
* 
* Parameters: Pnm_ppm image: the data for the given image
*             char *transformation: the type of transformation
*
* Return: Nothing, but transforms the image according to the given command.
*
* Expects: none of the arguments, other than the time_file, should be NULL, 
*          which is checked in the main and start_transform functions. 
*      
* Notes: Relies on the provided map function to carry out the transformation, 
*        with the transformation functions being apply functions. Frees 
*        memory for the timer and the original image. 
*      
******************************************************************************/
void other_transformations(Pnm_ppm image, char *transformation, 
        char *time_file, A2Methods_mapfun* map)
{
        /* Get timer and image data */
        CPUTime_T timer = CPUTime_New();
        const struct A2Methods_T *methods = image->methods;
        int destHeight = methods->height(image->pixels);
        int destWidth = methods->width(image->pixels);
        int destSize = methods->size(image->pixels);
        A2Methods_UArray2 destination;

        /* Carry out the given transformation */
        if (strcmp(transformation, "transpose") == 0){
                destination = methods->new(destHeight, destWidth, destSize);
                CPUTime_Start(timer);
                map(destination, transpose, image);
        } else {
                /*Since we did not implement horizontal flipping, program will 
                only enter this conditional if -flip vertical is given. Check 
                For horizontal would be here if implemented*/
                destination = methods->new(destWidth, destHeight, destSize);
                CPUTime_Start(timer);
                map(destination, flip_vertical, image);
        }
        /* Stop timer and update the image */
        double timeTaken = CPUTime_Stop(timer);
        image->height = image->methods->height(destination);
                        image->width = image->methods->width(destination);
        image->methods->free(&(image->pixels));
        image->pixels = destination;
        
        time_handle(time_file, timeTaken, image);
        CPUTime_Free(&timer);
}

/*****************start_transform*****************************************
*
* Function that gets the image data from the provided file and initiates the 
* image transformation.
* 
* Parameters: FILE *picFile: the file containing the image 
*             int rotation: the number of degrees to rotate by, if the 
*                   rotation command is given. 
*             char *time_file: the name of a file to output time data to 
*             A2Methods_mapfun* map: the mapping function to be used for a 
*                   transformation
*             A2Methods_T methods: the methods suite for UArray2s 
*             char *otherTrans: command for transformations other than rotation
*             char *direction: if flip is given, the type of flip 
*
* Return: Nothing, but prints the new image to standard output 
*
* Expects: that the image file, mapping function, and methods suite are not 
*          NULL, which is checked in the main function.
*      
* Notes: Relies on the pnm.h interface to read the source image as well as 
*        write and free the new image to standard output. Raises checked 
*        runtime error if the Pnm_ppm for the image was not created 
*        successfully. 
*      
*********************************************************************/
void start_transform(FILE *picFile, int rotation, char *time_file, 
        A2Methods_mapfun* map, A2Methods_T methods, char *otherTrans, 
        char *direction)
{
        /* Read in the image data from file */
        Pnm_ppm image = Pnm_ppmread(picFile, methods);
        assert(image != NULL);

        /* Transform the image according to the command given */
        if (otherTrans == NULL) {
                rotate_image_setup(image, rotation, time_file, map);
        }
        
        else if (strcmp(otherTrans, "-flip") == 0){
                other_transformations(image, direction, time_file, map);
        }

        /* Write new image to standard output and free */
        Pnm_ppmwrite(stdout, image);
        Pnm_ppmfree(&image);
}

/**********************time_handle*****************************************
*
* Prints the time data from the transformation to a time output file, if one 
* was provided by the client. 
* 
* Parameters: char *timeFile: the name of a time output file
*             double timeTaken: the amount of time a transformation took
*             Pnm_ppm image: the data of a provided image
*
* Return: Nothing, but prints time data to the output file if provided
*
* Expects: that a time file is valid if one was given.
*      
* Notes: if the time file provided is not valid, a checked runtime error is 
*        raised. Called on by the rotate_image_setup and flip_image functions.
*      
*********************************************************************/
void time_handle(char *timeFile, double timeTaken, Pnm_ppm image)
{
        /* Continue if a time file was provided */
        if (timeFile != NULL) {
                /* Open the file */
                FILE *time = fopen(timeFile, "w");
                assert(time != NULL);

                /* Print total time and time to pixel to file */
                fprintf(time, "Total time for transformation: %f nanoseconds ", 
                        timeTaken);

                double timePerPixel = timeTaken / 
                        ((double)(image->methods->width(image->pixels) 
                        * image->methods->height(image->pixels)));
                
                fprintf(time, "Time per pixel for transformation: "); 
                fprintf(time, "%f nanoseconds/pixel\n", timePerPixel);
                fclose(time);
        }
}

/***************************main*****************************************
*
* Function that parses through the commands provided by the client, opens a 
* provided file, and begins the image transformation.
* 
* Parameters: int argc: the number of arguments on the command line 
              char *argv[]: the arguments provided by the client
*
* Return: EXIT_SUCCESS if the program ran to completion. 
*
* Expects: the image file given on the command line is valid and the correct
*          number and types of arguments are given. 
*      
* Notes: if the image file provided is not valid, throws a checked runtime 
*        error. If the arguments given are not valid, the program prints an 
*        error message and returns EXIT_FAILURE from the usage function. 
*      
*********************************************************************/
int main(int argc, char *argv[])
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;
        char *otherTrans = NULL;
        char *direction = NULL;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-flip") == 0 ) {
                        if (!(i + 1 < argc)) {
                                usage(argv[0]);
                        }
                        otherTrans = argv[i];
                        direction = argv[++i];
                        if (!(strcmp(direction, "vertical") == 0)){
                                fprintf(stderr, 
                                        "Flip must be vertical\n");
                                usage(argv[0]);
                            }
                } else if (strcmp(argv[i], "-time") == 0) {
                        if (!(i + 1 < argc)) {      /* no time file */
                                usage(argv[0]);
                        }
                        time_file_name = argv[++i];
                
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        direction = "transpose";
                        otherTrans = "-flip";
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }

        /* Open the file from command line or standard input */
        FILE *picFile = NULL;

        if (i != argc) {
                picFile = fopen(argv[i], "r");
        }
        else if (i == argc){
                picFile = stdin;
        }
        
        assert(picFile != NULL);

        /* Begin the transformation */
        start_transform(picFile, rotation, time_file_name, map, methods, 
                otherTrans, direction);

        fclose(picFile);

        return EXIT_SUCCESS; 
}
        