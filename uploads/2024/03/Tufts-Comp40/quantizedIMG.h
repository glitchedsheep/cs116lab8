/*
 * quantizedIMG.h
 * By Isabelle Bain
 * March 9, 2022
 *
 * Interface for a cv image <-> quantized image transformation
 *
 * functions in this interface use only the 'new', 'free',
 * 'cell', and 'map_default' methods
 *
 * every function in this interface uses the (x, y) coordinate system,
 * which is the same as the (col, row) or (width, height) system.
 *
 */

#ifndef BLOCKEDIMG_INCLUDED
#define BLOCKEDIMG_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <assert.h>
#include <seq.h>
#include <arith40.h>
#include <math.h>

#include "except.h"
#include "componentIMG.h"
#include <a2methods.h>

/* blocked_img
* Struct containing all information associated with a blocked image: width,
* height, the pixel map, and the methods suite. Pixel map is a 2D blocked 
* array with each element pointing to a pre quantization pixel struct 
* representing a 2x2 block of the original PPM image. Methods suite is used to
* operate on the 'pixels' field.
*
* Based on the struct Pmn_ppm from pnm.h.
*/
typedef struct blocked_img {
        unsigned width, height;
        A2Methods_UArray2 pixels;
        const struct A2Methods_T *methods;
} *blocked_img;

/* blocked image pixel - represents a 2x2 pixel of the original image and the
vales that we extract from them and store for further calculations */
typedef struct blocked_pix {
    float a, b, c, d, pb, pr;
} *blocked_pix;


/* quantized_img
* Struct containing all information associated with a blocked image: width,
* height, the pixel map, and the methods suite. Pixel map is a 2D blocked 
* array with each element pointing to a post quantization pixel struct 
* representing a 2x2 block of the original PPM image. Methods suite is used to
* operate on the 'pixels' field.
*
* Based on the struct Pmn_ppm from pnm.h.
*/
typedef struct quantized_img {
        unsigned width, height;
        A2Methods_UArray2 pixels;
        const struct A2Methods_T *methods; 
} *quantized_img;

/* quantized image pixel - represents a 2x2 pixel of the original image and the
vales that we extract from them after they have been quantized */
typedef struct quantized_pix {
        unsigned a, pb, pr;
        signed b, c, d;
} *quantized_pix;


/* passIN
* Struct used as the closure in apply_cv_to_blocked. Contains a pixel array as
* well as a sequence used to hold pixels representing each 2x2 block, and the
* methods suite. Methods suite is used to operate on the 'pixels' field.
*/
typedef struct passIN {
        Seq_T block;
        A2Methods_UArray2 pixels;
const struct A2Methods_T *methods;
} *passIN;

/*****************************************************************************\
*            Compression: Component Video Image to Quantized Image            *
\*****************************************************************************/

/* takes in a cv_img and converts it to a blocked_img that gets returned*/
quantized_img cv_to_quantized(cv_img cv, A2Methods_T methods);

/* takes in a sequence of 4 cv_ypbpr pixels and turns it into a blocked pixel*/
blocked_pix cvpix_to_blockedpix(Seq_T pixels);

/*  takes in a blcoked and turns it into a quantized pixel*/
quantized_pix blockedpix_to_quantpix(blocked_pix pixel);

/*  to be called with a mapping function. calleed on an empty blocked_img that
gets populated using the cv_image passed in*/
void apply_cv_to_blocked(int col, int row, A2Methods_UArray2 component, 
                                                void *pixel, void *floatIMG);

/* to be called with a mapping function. called on an empty pnm_ppm that gets 
populated using the cv_img passed in*/
void apply_blocked_to_quantized(int col, int row, A2Methods_UArray2 quant, 
                                            void *pixel, void *blockedIMG);

/*****************************************************************************\
*            Deompression: Quantized Image to Component Video Image           *
\*****************************************************************************/

/* takes in a blocked_img and converts it to a cv_image that gets returned */
cv_img quantized_to_cv(quantized_img quantized, A2Methods_T methods);

/*  takes in a blocked pixel and turns it into a sequence of 4 cv_ypbpr 
pixels */
Seq_T blockedpix_to_cvpix(blocked_pix pixel);

/*  takes in single quantized pixel and turns it into a quantized pixel*/
blocked_pix quantpix_to_blockedpix(quantized_pix pixel);

/* to be called with a mapping function. called on an empty cv_img that gets 
populated using the blocked_img passed in */
void apply_blocked_to_cv(int col, int row, A2Methods_UArray2 blocked, 
                                            void *pixel, void *componentIMG);

/* to be called with a mapping function. called on an empty blocked image that 
gets populated using the quantized_img passed in*/
void apply_quantized_to_block(int col, int row, A2Methods_UArray2 blocks, 
                                            void *pixel, void *quantizedIMG);

/*****************************************************************************\
*                     Memory Deallocation: Free Functions                     *
\*****************************************************************************/

/* Free pixmap pointed to by blockedIMG. Does not free methods. */
void blocked_imgfree(blocked_img *blockedIMG);

/* Free pixmap pointed to by quantized. Does not free methods. */
void quantized_imgfree(quantized_img *quantized);


#endif