/*
 * componentIMG.h
 * By Isabelle Bain
 * March 9, 2022
 *
 * Interface for a PPM image <-> component video image transformation, with 
 * an intermediary floating-point RGB image transformation.
 *
 * Every function in this interface uses the (x, y) coordinate system, which is
 * the same as the (col, row) or (width, height) system.
 */

#ifndef COMPONENTIMG_INCLUDED
#define COMPONENTIMG_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <assert.h>
#include "except.h"
#include "a2methods.h"

/* cv_img
 * Struct containing all information associated with a component video image:
 * width, height, the pixel map, and the methods suite. Pixel map is a 2D
 * blocked array with element type 'struct cv_ypbpr'. Methods suite is used 
 * to operate on the 'pixels' field.
 *
 * Based on the struct Pmn_ppm from pnm.h.
 */
typedef struct cv_img {
        unsigned width, height;
        A2Methods_UArray2 pixels;
        const struct A2Methods_T *methods; 
} *cv_img;

/* cv_ypbpr
 * Struct containing the Y/PB/PR data of a component video pixel.
 * Based on the struct Pmn_rgb from pnm.h.
 */
typedef struct cv_ypbpr {
        float y, pb, pr;
} *cv_ypbpr;

/* float_img
 * Struct containing all information associated with a floating-point version 
 * of a PPM image: width, height, denominator, the pixel map, and the methods 
 * suite. Denominator represents the denominator of the PPM the floating-point
 * image was derived from. Pixel map is a 2D blocked array with element type 
 * 'struct float_rgb'. Methods suite is used to operate on the 'pixels' field.
 *
 * Based on the struct Pmn_ppm from pnm.h.
 */
typedef struct float_img {
        unsigned width, height, denominator;
        A2Methods_UArray2 pixels;
        const struct A2Methods_T *methods;
} *float_img;

/* cv_ypbpr
 * Struct containing the floating-point RGB data of an image pixel.
 * Based on the struct Pmn_rgb from pnm.h.
 */
typedef struct float_rgb {
        float red, green, blue;
} *float_rgb;


/******************************************************************************\
*                   Compression: PPM to Component Video Image                  *
\******************************************************************************/

/* takes in a pnm_ppm and converts it to a cv_img that gets returned */
cv_img ppm_to_cv(Pnm_ppm ppm, A2Methods_T methods);

/* 
 * to be called with a mapping function - called on an empty float_img that gets
 * populated by transforming the pnm_ppm's pixel data that gets passed in
 */
void apply_rgb_to_float(int col, int row, A2Methods_UArray2 floatIMG, 
                        void *pixel, void *ppm);

/* 
 * to be called with a mapping function - called on an empty cv_img that gets
 * populated by transforming the float_img's pixel data that gets passed in   
 */
void apply_float_to_cv(int col, int row, A2Methods_UArray2 component, 
                       void *pixel, void *floatIMG);

/* takes in single floating-point rbg pixel and turns it into a cv pixel */
cv_ypbpr floatpix_to_cvpix(float_rgb pixel);


/******************************************************************************\
*                 Decompression: Component Video to PPM Image                  *
\******************************************************************************/

/* takes in a cv_img and converts it to a Pnm_ppm that gets returned */
Pnm_ppm cv_to_ppm(cv_img cv, A2Methods_T methods);

/* 
 * to be called with a mapping function - called on an empty Pnm_ppm that gets
 * populated by transforming the cv_img's pixel data that gets passed in
 */
void apply_cv_to_float(int col, int row, A2Methods_UArray2 ppm, void *pixel,
                       void *cv);

/* 
 * to be called with a mapping function - called on an empty Pnm_ppm that gets
 * populated by transforming the float_img's pixel data that gets passed in
 */
void apply_float_to_rgb(int col, int row, A2Methods_UArray2 ppm, void *pixel,
                        void *floatIMG);

/* takes in single cv pixel and turns it into a floating-point rbg pixel */
float_rgb cvpix_to_floatpix(cv_ypbpr pixel);

/******************************************************************************\
*                     Memory Deallocation: Free Functions                      *
\******************************************************************************/

/* frees a given component video image, including its pixel map */
void cv_imgfree(cv_img *cv);

/* frees a given floating-point image, including its pixel map */
void float_imgfree(float_img *floatIMG);


#endif
