/* 
 * componentIMG.c
 * By Isabelle Bain
 * March 9, 2022
 *
 * Implementation of the componentIMG interface, which performs a transformation
 * that can turn a ppm image to a component video representation and vice versa
 * using an intermediary floatin-point RGB representation. Includes functions 
 * that deallocate memory for the included component video and floating-point 
 * image structs.
 */

#include "componentIMG.h"

#define BLOCK_SIZE 2

/******************************************************************************\
*                   Compression: PPM to Component Video Image                  *
\******************************************************************************/
 
/* ppm_to_cv
 *     Purpose:  transforms a given PPM image to a component video 
 *               representation - takes in a Pnm_ppm and populates a cv_img
 *               with each pixel containing the corresponding Y/PB/PR values 
 *  Parameters:  ppm – the PPM image to transform (contains dimensions and map)
 *               methods - the methods suite used to create and map the cv image
 *     Returns:  a struct containing the component video image
 * Error cases:  checked runtime error if the given ppm or methods is null, or
 *               if allocating memory fails for the component video image or the
 *               intermediary floating-point image
 */
cv_img ppm_to_cv(Pnm_ppm ppm, A2Methods_T methods)
{
    assert(ppm != NULL);
    assert(methods != NULL);
    
    /* trim edges if needed */
    int width = ((ppm->width) % 2 == 0) ? ppm->width : ppm->width - 1;
    int height = ((ppm->height) % 2 == 0) ? ppm->height : ppm->height - 1;
    
    /* create a floating-point version of the ppm image */
    float_img floatIMG = malloc(sizeof(struct float_img));
    assert(floatIMG != NULL);
    floatIMG->methods = methods;
    floatIMG->width = width;
    floatIMG->height = height;
    floatIMG->denominator = ppm->denominator;
    
    /* create and populate the floating-point pixel map */
    floatIMG->pixels = methods->new_with_blocksize(width, height, 
                                          sizeof(struct float_rgb), BLOCK_SIZE);
    assert(floatIMG->pixels != NULL);
    methods->map_block_major(floatIMG->pixels, apply_rgb_to_float, ppm);
    
    /* create a component video version of the image */
    cv_img componentIMG = malloc(sizeof(struct cv_img));
    assert(componentIMG != NULL);
    componentIMG->methods = methods;
    componentIMG->width = width;
    componentIMG->height = height;
    
    /* create and populate the component video pixel map */
    componentIMG->pixels = methods->new_with_blocksize(width, height, 
                                           sizeof(struct cv_ypbpr), BLOCK_SIZE);
    assert(componentIMG->pixels != NULL);
    methods->map_block_major(componentIMG->pixels, apply_float_to_cv, floatIMG);
    
    /* free the intermediary float image now that we're done with it */
    float_imgfree(&floatIMG);
    free(floatIMG);
    
    return componentIMG;
}

/* apply_rgb_to_float
 *     Purpose:  apply function that populates a given floating-point pixel 
 *               (float_rgb) by dividing the RGB data from the passed ppm by 
 *               its denominator
 *  Parameters:  col, row – the column and row of the current floatIMG pixel
 *               floatIMG - the pixel map of the floating-point image
 *               pixel - a pointer to the current floatIMG pixel
 *               ppm - a pointer to the PPM image we're using the RGB data from
 *     Returns:  nothing
 * Error cases:  checked runtime error if the given pixel map is null, the 
 *               ppm or its corresponding pixel or methods are null, or if the
 *               RGB data from the ppm pixel is greater than the denominator
 */
void apply_rgb_to_float(int col, int row, A2Methods_UArray2 floatIMG, 
                        void *pixel, void *ppm)
{
    (void) pixel;
    assert(floatIMG != NULL);
    assert(ppm != NULL);
    
    Pnm_ppm copyPPM = (Pnm_ppm) ppm;
    A2Methods_T methods = (A2Methods_T) copyPPM->methods;
    assert(methods != NULL);
    
    Pnm_rgb ppmPixel = methods->at(copyPPM->pixels, col, row);
    assert(ppmPixel != NULL);

    assert(ppmPixel->red <= copyPPM->denominator);
    assert(ppmPixel->green <= copyPPM->denominator);
    assert(ppmPixel->blue <= copyPPM->denominator);
    
    float_rgb pixelData = methods->at(floatIMG, col, row);
    assert(pixelData != NULL);
    
    pixelData->red = (float)ppmPixel->red / (float)copyPPM->denominator;
    pixelData->green = (float)ppmPixel->green / (float)copyPPM->denominator;
    pixelData->blue = (float)ppmPixel->blue / (float)copyPPM->denominator;
    
}

/* apply_float_to_cv
 *     Purpose:  apply function that populates a given component video pixel 
 *               (cv_ypbpr) by transforming the RGB color space of the passed 
 *               floating-point image's corresponding pixel
 *  Parameters:  col, row – the column and row of the current cv pixel
 *               component - the pixel map of the component video image
 *               pixel - a pointer to the current component video pixel
 *               floatIMG - a pointer to the floating-point image we're using 
 *               the RGB data from
 *     Returns:  nothing
 * Error cases:  checked runtime error if the cv pixel map is null, the float
 *               image is null, or its corresponding pixel or methods are null
 */
void apply_float_to_cv(int col, int row, A2Methods_UArray2 component, 
                       void *pixel, void *floatIMG)
{
    (void)pixel;
    assert(component != NULL);
    assert(floatIMG != NULL);
    
    float_img copyFloat = (float_img) floatIMG;
    A2Methods_T methods = (A2Methods_T) copyFloat->methods;
    assert(methods != NULL);
    assert(copyFloat->pixels != NULL);
    
    cv_ypbpr pix = floatpix_to_cvpix(methods->at(copyFloat->pixels, col, row));
    
    cv_ypbpr cv = methods->at(component, col, row);
    assert(cv != NULL);
    
    cv->y = pix->y;
    cv->pb = pix->pb;
    cv->pr = pix->pr;
    
    free(pix);
}

/* floatpix_to_cvpix
 *     Purpose:  transforms a given floating-point pixel to a component video 
 *               pixel
 *  Parameters:  pixel – a struct containing a pixel's floating-point RGB data
 *     Returns:  a struct containing the pixel's corresponding Y/PB/PR data
 * Error cases:  checked runtime error if the given floating-point pixel is null
 *               or if memory allocation of the component video pixel fails
 */
cv_ypbpr floatpix_to_cvpix(float_rgb pixel)
{
    assert(pixel != NULL);
    
    float r = pixel->red;
    float g = pixel->green;
    float b = pixel->blue;
    
    cv_ypbpr newPix = malloc(sizeof(struct cv_ypbpr));
    assert(newPix != NULL);
    
    newPix->y = (0.299 * r) + (0.587 * g) + (0.114 * b);
    newPix->pb = (-0.168736 * r) - (0.331264 * g) + (0.5 * b);
    newPix->pr = (0.5 * r) - (0.418688 * g) - (0.081312 * b);
    
    return newPix;
}

/******************************************************************************\
*                 Decompression: Component Video to PPM Image                  *
\******************************************************************************/

/* cv_to_ppm
 *     Purpose:  transforms a given component video image to a PPM 
 *               representation - takes in a cv_img and populates a Pnm_ppm
 *               with each pixel containing the corresponding RGB values 
 *  Parameters:  cv – the cv image to transform (contains dimensions and map)
 *               methods - the methods suite used to create and map the PPM
 *     Returns:  a struct containing the PPM image (Ppm_pnm) and its information
 * Error cases:  checked runtime error if the given cv or methods are null, or
 *               if allocating memory fails for the PPM image or the
 *               intermediary floating-point image
 */
Pnm_ppm cv_to_ppm(cv_img cv, A2Methods_T methods)
{
    assert(cv != NULL);
    assert(methods != NULL);

    /* component video to floating-point transformation */
    float_img floatIMG = malloc(sizeof(struct float_img));
    assert(floatIMG != NULL);
    
    floatIMG->height = cv->height;
    floatIMG->width = cv->width;
    floatIMG->methods = methods;
    floatIMG->denominator = 255;
    floatIMG->pixels = methods->new_with_blocksize(cv->width, cv->height, 
                                        sizeof(struct float_rgb), BLOCK_SIZE);
    assert(floatIMG->pixels != NULL);
    
    methods->map_block_major(floatIMG->pixels, apply_cv_to_float, cv);
    
    /* floating-point to ppm transformation */
    Pnm_ppm ppm = malloc(sizeof(struct Pnm_ppm));
    assert(ppm != NULL);
    
    ppm->height = cv->height;
    ppm->width = cv->width;
    ppm->denominator = 255;
    ppm->methods = methods;
    ppm->pixels = methods->new_with_blocksize(cv->width, cv->height, 
                                            sizeof(struct Pnm_rgb), BLOCK_SIZE);
    assert(ppm->pixels != NULL);
    methods->map_block_major(ppm->pixels, apply_float_to_rgb, floatIMG);
    
    /* free the floating-point image */
    float_imgfree(&floatIMG);
    free(floatIMG);

    return ppm;
    
}

/* apply_cv_to_float
 *     Purpose:  apply function that populates a given floating-point pixel 
 *               (float_img) by transforming the Y/PB/PR color space of the 
 *               passed component video image's corresponding pixel
 *  Parameters:  col, row – the column and row of the current float image pixel
 *               floatIMG - the pixel map of the floating-point image
 *               pixel - a pointer to the current floating-point pixel
 *               cv - a pointer to the component video image we're using 
 *               the Y/PB/PR data from
 *     Returns:  nothing
 * Error cases:  checked runtime error if the float pixel map is null, the cv
 *               image is null, or its corresponding pixel or methods are null
 */
void apply_cv_to_float(int col, int row, A2Methods_UArray2 floatIMG, 
                       void *pixel, void *cv)
{
    assert(floatIMG != NULL);
    assert(cv != NULL);
    
    cv_img copyCV = (cv_img) cv;
    A2Methods_UArray2 pixels = copyCV->pixels;
    A2Methods_T methods = (A2Methods_T) copyCV->methods;
    assert(pixels != NULL);
    assert(methods != NULL);
    
    float_rgb pix = cvpix_to_floatpix(methods->at(pixels, col, row));
    
    float_rgb rgb = methods->at(floatIMG, col, row);
    assert(rgb != NULL);

    rgb->red = pix->red;
    rgb->green = pix->green;
    rgb->blue = pix->blue;
    
    (void)pixel;
    
    free(pix);
}

/* apply_float_to_rgb
 *     Purpose:  apply function that populates a given PPM pixel (Pnm_rgb) by 
 *               multiplying the floating-point image's corresponding pixel's
 *               RGB data by the denominator (this program uses 255)
 *  Parameters:  col, row – the column and row of the current PPM pixel
 *               ppm - the pixel map of the PPM image
 *               pixel - a pointer to the current PPM pixel
 *               floatIMG - a pointer to the floating-point image we're using 
 *               the RGB data from
 *     Returns:  nothing
 * Error cases:  checked runtime error if the ppm pixel/map is null, the float
 *               image is null, or its corresponding pixel or methods are null
 */
void apply_float_to_rgb(int col, int row, A2Methods_UArray2 ppm, void *pixel, 
                        void *floatIMG)
{
    assert(floatIMG != NULL);
    assert(ppm != NULL);
    assert(pixel != NULL);
    
    float_img copyFloat = (float_img)floatIMG;
    A2Methods_UArray2 pixels = copyFloat->pixels;
    A2Methods_T methods = (A2Methods_T) copyFloat->methods;
    assert(pixels != NULL);
    assert(methods != NULL);
    
    float_rgb floatPixel = methods->at(pixels, col, row);
    assert(floatPixel != NULL);
    
    unsigned r = floatPixel->red * copyFloat->denominator;
    unsigned g = floatPixel->green * copyFloat->denominator;
    unsigned b = floatPixel->blue * copyFloat->denominator;

    assert(r <= (unsigned)copyFloat->denominator);
    assert(g <= (unsigned)copyFloat->denominator);
    assert(b <= (unsigned)copyFloat->denominator);
    
    ((Pnm_rgb)pixel)->red = r;
    ((Pnm_rgb)pixel)->green = g;
    ((Pnm_rgb)pixel)->blue = b;
}

/* cvpix_to_floatpix
 *     Purpose:  transforms a given component video pixel to a floating-point
 *               RGB pixel
 *  Parameters:  pixel – a struct containing a pixel's Y/PB/PR data
 *     Returns:  a struct containing the pixel's corresponding RGB data in 
 *               floating-point form
 * Error cases:  checked runtime error if the given cv pixel is null or if 
 *               memory allocation of the floating-point pixel fails
 */
float_rgb cvpix_to_floatpix(cv_ypbpr pixel)
{
    assert(pixel != NULL);
    
    float y = pixel->y;
    float pb = pixel->pb;
    float pr = pixel->pr;
        
    float_rgb pix = malloc(sizeof(struct float_rgb));
    assert(pix != NULL);
    
    pix->red = (1.0 * y) + (0.0 * pb) + (1.402 * pr);
    pix->red = (pix->red < 0) ? 0 : ((pix->red > 1) ? 1 : pix->red);

    pix->green = (1.0 * y) - (0.344136 * pb) - (0.714136 * pr);
    pix->green = (pix->green < 0) ? 0 : ((pix->green > 1) ? 1 : pix->green);

    pix->blue = (1.0 * y) + (1.772 * pb) + (0.0 * pr);
    pix->blue = (pix->blue < 0) ? 0 : ((pix->blue > 1) ? 1 : pix->blue);
    
    return pix;
}

/******************************************************************************\
*                     Memory Deallocation: Free Functions                      *
\******************************************************************************/

/* cv_imgfree
 *     Purpose:  clears and deallocates memory from a given component video
 *               image - including the pixel map within the struct itself
 *  Parameters:  a struct containing the component video image and its data
 *     Returns:  nothing
 * Error cases:  checked runtime error if the given cv image is null
 */
void cv_imgfree(cv_img *cv)
{
    assert(cv != NULL);
    assert((*cv)->pixels != NULL);
    (*cv)->methods->free(&(*cv)->pixels);
}

/* float_imgfree
 *     Purpose:  clears and deallocates memory from a given floating-point RGB
 *               image - including the pixel map within the struct itself
 *  Parameters:  a struct containing the floating-point RGB image and its data
 *     Returns:  nothing
 * Error cases:  checked runtime error if the given floating-point image is null
 */
void float_imgfree(float_img *floatIMG)
{
    assert(floatIMG != NULL);
    assert((*floatIMG)->pixels != NULL);
    (*floatIMG)->methods->free(&(*floatIMG)->pixels);
}
