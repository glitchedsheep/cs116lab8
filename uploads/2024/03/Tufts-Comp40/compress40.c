/*
 * compress40.c
 * By Isabelle Bain
 * March 9, 2022
 *
 * Contains overall compression and decompression steps. The compression 
 * function, compress40, directly calls our image transformation interfaces to 
 * transform a given file to a PPM -> component video image -> quantized image 
 * -> compressed image before printing out the results to standard output. The 
 * other function, decompress40, does the same but in reverse order: reads in
 * compressed file -> quantized image -> component video -> PPM -> print image.
 */

#include <stdio.h>
#include "compress40.h"
#include "assert.h"

#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"

#include "componentIMG.h"
#include "quantizedIMG.h"
#include "compressedIMG.h"

/* compress40
 *     Purpose:  transforms a given PPM image to a compressed image, printing 
 *               the result to standard output
 *  Parameters:  a file pointer to a PPM
 *     Returns:  nothing, but the compressed image is printed to stdout
 * Error cases:  checked runtime error if methods is null
 */
void compress40(FILE *input)
{
    /* set methods to the UArray2b methods suite so we can use blocked arrays*/
    A2Methods_T methods = uarray2_methods_blocked;
    assert(methods != NULL);
    
    /* read in the given file to a PPM */
    Pnm_ppm image = Pnm_ppmread(input, methods);
    
    /* transform the PPM to a component video representation */
    cv_img component_video = ppm_to_cv(image, methods);
    
    /* transform the component video image to a quantized representation */
    quantized_img quant = cv_to_quantized(component_video, methods);
    
    /* transform the quantized representation to a compressed image */
    compressed_img compressed = quantized_to_compressed(quant, methods);

    /* write the compressed image to standard output */
    compressed_write(compressed);
    
    /* deallocate memory */
    Pnm_ppmfree(&image);
    free(image);

    cv_imgfree(&component_video);
    free(component_video);

    quantized_imgfree(&quant);
    free(quant);
    
    compressed_imgfree(&compressed);
    free(compressed);
}


/* decompress40
 *     Purpose:  transforms a given compressed image to a PPM, printing the
 *               result to standard output
 *  Parameters:  a file pointer to a compressed image
 *     Returns:  nothing, but the PPM image is printed to stdout
 * Error cases:  checked runtime error if methods is null
 */
void decompress40(FILE *input)
{
    
    A2Methods_T methods = uarray2_methods_blocked;
    assert(methods != NULL);
    
    /* read in the compressed image from the user */
    compressed_img image = compressed_read(input, methods);
    
    /* transform the compressed image to a quantized representation */
    quantized_img quant = compressed_to_quantized(image, methods);
    
    /* transform the quantized image to a component video representation */
    cv_img component_video = quantized_to_cv(quant, methods);
    
    /* transform the component video image to a PPM representation */
    Pnm_ppm pnm_image = cv_to_ppm(component_video, methods);
    
    /* write the PPM image to standard output */
    Pnm_ppmwrite(stdout, pnm_image);    
    
    /* deallocate memory */
    Pnm_ppmfree(&pnm_image);
    free(pnm_image);
    
    cv_imgfree(&component_video);
    free(component_video);
    
    quantized_imgfree(&quant);
    free(quant);
    
    compressed_imgfree(&image);
    free(image);
}
