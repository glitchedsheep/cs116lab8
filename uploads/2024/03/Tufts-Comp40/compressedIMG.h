/*
 * compressedIMG.h
 * By Isabelle Bain
 * March 9, 2022
 *
 * Interface for a quantized image <-> compressed image transformation, which 
 * includes functions that read and write the compressed image to standard 
 * output.
 *
 * Every function in this interface uses the (x, y) coordinate system, which is
 * the same as the (col, row) or (width, height) system.
 */

#ifndef COMPRESSEDIMG_INCLUDED
#define COMPRESSEDIMG_INCLUDED
 
#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <assert.h>

#include "quantizedIMG.h"
#include "bitpack.h"

/* compressed_img
 * Struct containing all information associated with a compressed image: width,
 * height, the pixel map, and the methods suite. Pixel map is a 2D blocked 
 * array with each element pointing to a 32-bit word representing a 2x2 block 
 * of the original PPM image. Methods suite is used to operate on the 'pixels'
 * field.
 *
 * Based on the struct Pmn_ppm from pnm.h.
 */
typedef struct compressed_img {
        unsigned width, height;
        A2Methods_UArray2 pixels;
        const struct A2Methods_T *methods;
} *compressed_img;

/******************************************************************************\
*                  Compression: Quantized to Compressed Image                  *
\******************************************************************************/

/* takes in a quantized_img, transforms and returns it as a compressed_img */
compressed_img quantized_to_compressed(quantized_img quan, 
                                                        A2Methods_T methods);

/* 
 * to be called with a mapping function - called on an empty compressed_img 
 * that gets populated by bitpacking the passed quantized_img's pixel data
 */
void apply_quantized_to_compressed(int col, int row, 
                A2Methods_UArray2 compress, void *pixel, void *quantizedIMG);

/* takes in single quantized pixel and turns it into a single 32-bit word */
uint64_t quantizedpix_to_compressedpix(quantized_pix pixel);

/* writes a given compressed image to standard output */
void compressed_write(compressed_img image);

/* 
 * to be called with a mapping function - called on a compressed_img, each 
 * pixel (32-bit word) gets printed out as 4 8-bit characters to standard 
 * output
 */
void apply_write(int col, int row, A2Methods_UArray2 compressed, void *pixel, 
                 void *closure);

/******************************************************************************\
*                 Decompression: Compressed to Quantized Image                 *
\******************************************************************************/

/* reads a given compressed image into a compressed_img */
compressed_img compressed_read(FILE *input, A2Methods_T methods);

/* 
 * to be called with a mapping function - called on an empty compressed_img,
 * each pixel gets populated with a corresponding word from the input file
 */
 void apply_read(int col, int row, A2Methods_UArray2 compressed, void *pixel, 
                 void *input);

/* takes in a compressed_img, transforms and returns it as a quantized_img */
quantized_img compressed_to_quantized(compressed_img comp, 
                                                        A2Methods_T methods);

/* 
 * to be called with a mapping function - called on an empty quantized_img that
 * gets populated by unpacking the passed compressed_img's pixel data
 */
void apply_compressed_to_quantized(int col, int row, A2Methods_UArray2 quant, 
                                        void *pixel, void *compressedIMG);

/* takes in single 32-bit word and converts it to a quantized pixel */
quantized_pix compressedpix_to_quantizedpix(uint64_t word);

/******************************************************************************\
*                     Memory Deallocation: Free Functions                      *
\******************************************************************************/

/* frees a given compressed image, including its pixel map */
void compressed_imgfree(compressed_img *compressed);

#endif
