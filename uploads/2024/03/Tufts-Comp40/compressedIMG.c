/*
 * compressedIMG.c
 * By Isabelle Bain
 * March 9, 2022
 *
 * Implementation of the compressedIMG interface, which transforms a quantized
 * image to a compressed image or vice versa. Compressed image is represented 
 * as a 2D array of 32-bit words - functions from the bitpack.h interafce are 
 * used to pack and unpack these words. Includes functions that can read in 
 * a compressed image from a file pointer, and write a compressed image to 
 * standard output.
 */

#include "compressedIMG.h"

#define BLOCK_SIZE 2
#define START_LSB 24
#define BYTE_SIZE 8

/*****************************************************************************\
*                   Compression: Quantized to Compressed Image                *
\*****************************************************************************/

/* quantized_to_compressed
 *     Purpose:  transforms a given quantized image to a compressed
 *               representation - takes in a quantized_img and populates a 
 *               compressed_img with each pixel containing the corresponding
 *               quantized pixel's information bitpacked into a 32-bit word
 *  Parameters:  quant – the quantized image to transform
 *               methods - the methods suite used to map the compressed image
 *     Returns:  a struct containing the compressed image
 * Error cases:  checked runtime error if the given quantized image or methods
 *               is null, or if allocating memory fails for the compressed 
 *               image
 */
compressed_img quantized_to_compressed(quantized_img quant, 
                                                        A2Methods_T methods)
{
    assert(quant != NULL);
    assert(methods != NULL);
    
    /* create a compressed image, populate the struct with relevant data */
    compressed_img compressedIMG = malloc(sizeof(struct compressed_img));
    assert(compressedIMG != NULL);
    compressedIMG->methods = methods;
    compressedIMG->width = quant->width;
    compressedIMG->height = quant->height;
    
    /* create the pixel map, populate it with the quantized image's data */
    compressedIMG->pixels = methods->new_with_blocksize(compressedIMG->width, 
                            compressedIMG->height, sizeof(uint64_t), 1);
    assert(compressedIMG->pixels != NULL);
    methods->map_block_major(compressedIMG->pixels,
                             apply_quantized_to_compressed, quant);
    
    return compressedIMG;
}

/* apply_quantized_to_compressed
 *     Purpose:  apply function that populates a given compressed pixel by 
 *               packing the given quantized image's corresponding pixel 
 *               information into a 32-bit word
 *  Parameters:  col, row – the column and row of the current compressed pixel
 *               compressed - the pixel map of the compressed image
 *               pixel - a pointer to the current compressed pixel
 *               quantizedIMG - a pointer to the quantized image to transform
 *     Returns:  nothing
 * Error cases:  checked runtime error if the given pixel map is null or the 
 *               quantized image or its corresponding pixel/methods are null
 */
void apply_quantized_to_compressed(int col, int row, A2Methods_UArray2 
                                compressed, void *pixel, void *quantizedIMG)
{
    (void) pixel;
    assert(compressed != NULL);
    assert(quantizedIMG != NULL);
    
    quantized_img copyQUANT = (quantized_img)quantizedIMG;
    A2Methods_T methods = (A2Methods_T) copyQUANT->methods;
    assert(methods != NULL);
    assert(copyQUANT->pixels != NULL);

    quantized_pix pix = methods->at(copyQUANT->pixels, col, row);
    assert(pix != NULL);

    uint64_t word = quantizedpix_to_compressedpix(pix);
    
    uint64_t *wordPix = methods->at(compressed, col, row);
    assert(wordPix != NULL); 
    
    *wordPix = word;
}

/* quantizedpix_to_compressedpix
 *     Purpose:  bitpacks a given quantized pixel into a 32-bit word
 *  Parameters:  pixel – a struct containing a quantized pixel's data
 *     Returns:  a 32-bit word packed with the quantized pixel's data
 * Error cases:  checked runtime error if the given quantized pixel is null
 */
uint64_t quantizedpix_to_compressedpix(quantized_pix pixel)
{
    assert(pixel != NULL);
    uint64_t word = 0;
    
    word = Bitpack_newu(word, 6, 26, pixel->a);
    word = Bitpack_news(word, 6, 20, pixel->b);
    word = Bitpack_news(word, 6, 14, pixel->c);
    word = Bitpack_news(word, 6, 8, pixel->d);
    word = Bitpack_newu(word, 4, 4, pixel->pb);
    word = Bitpack_newu(word, 4, 0, pixel->pr);
    
    return word;
}

/* compressed_write
 *     Purpose:  writes a given compressed image to standard output
 *  Parameters:  image – a struct containing a compressed image and its data
 *     Returns:  nothing, but the image gets printed to standard output
 * Error cases:  checked runtime error if the image or its methods/map are null
 */
void compressed_write(compressed_img image)
{
     assert(image != NULL);
     assert(image->methods != NULL);
     assert(image->pixels != NULL);
     
     unsigned og_width = image->width * 2;
     unsigned og_height = image->height * 2;
     
     printf("COMP40 Compressed image format 2\n%u %u\n", og_width, og_height);
                                             
     image->methods->map_block_major(image->pixels, apply_write, NULL);
}

/* apply_write
 *     Purpose:  apply function that prints out a given pixel of a compressed 
 *               image by printing out 4 8-bit characters that represent a 
 *               32-bit word containing bitpacked pixel data
 *  Parameters:  col, row – the column and row of the current compressed pixel
 *               compressed - the pixel map of the compressed image
 *               pixel - a pointer to the current compressed pixel
 *               closure - a closure pointer that isn't needed nor used
 * Error cases:  checked runtime error if the given pixel is null
 */
void apply_write(int col, int row, A2Methods_UArray2 compressed, void *pixel, 
                            void *closure)
{
    (void) col, (void) row, (void) compressed, (void) closure;
    assert(pixel != NULL);
    
    uint64_t word = *(uint64_t *)pixel;
    unsigned lsb = START_LSB;
    for (int i = 0; i < 4; i++) {
        uint64_t value = Bitpack_getu(word, BYTE_SIZE, lsb);
        putchar(value);
        lsb -= BYTE_SIZE;
    }
}

/*****************************************************************************\
*                 Decompression: Compressed to Quantized Image                *
\*****************************************************************************/

/* compressed_read
 *     Purpose:  reads a given compressed image into a compressed_img struct
 *  Parameters:  input – a pointer to the compressed image file
 *               methods - the methods suite used to map the compressed image
 *     Returns:  a struct containing the compressed image
 * Error cases:  checked runtime error if the file pointer or methods are null, 
 *               or if allocation of the compressed image's memory fails
 */
compressed_img compressed_read(FILE *input, A2Methods_T methods)
{
    assert(input != NULL);
    assert(methods != NULL);
    
    /* read in the header information */
    unsigned height, width;
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", &width,
                                                                      &height);
    assert(read == 2);
    int c = getc(input);
    assert(c == '\n');
    assert(width != 0 && height != 0);
    
    /* create a compressed image, populate the struct with relevant data */
    compressed_img compressedIMG = malloc(sizeof(struct compressed_img));
    assert(compressedIMG != NULL);
    compressedIMG->methods = methods;
    compressedIMG->width = width / 2;
    compressedIMG->height = height / 2;
    
    /* create the pixel map, populate it with the file's data */
    compressedIMG->pixels = methods->new_with_blocksize(compressedIMG->width, 
                            compressedIMG->height, sizeof(uint64_t), 1);
    assert(compressedIMG->pixels != NULL);
    methods->map_block_major(compressedIMG->pixels, apply_read, input);
       
    return compressedIMG;
}

/* apply_read
 *     Purpose:  apply function that reads in a given pixel of a compressed 
 *               image by packing 4 characters into a 32-bit word
 *  Parameters:  col, row – the column and row of the current compressed pixel
 *               compressed - the pixel map of the compressed image
 *               pixel - a pointer to the current compressed pixel
 *               input - a pointer to the compressed image file
 * Error cases:  checked runtime error if the given pixel map, pixel, or image 
 *               file is null
 */
void apply_read(int col, int row, A2Methods_UArray2 compressed, void *pixel, 
                void *input)
{
    (void) row, (void) col;
    assert(compressed != NULL);
    assert(input != NULL);
    assert(pixel != NULL);
    
    FILE *in = (FILE *)input;
    
    uint64_t word = 0;
    unsigned lsb = START_LSB;
    for (int i = 0; i < 4; i++) {
        uint64_t eightbits = getc(in);
        word = Bitpack_newu(word, BYTE_SIZE, lsb, eightbits);
        lsb -= BYTE_SIZE;
    }
    
    *(uint64_t *)pixel = word;
}

/* compressed_to_quantized
 *     Purpose:  transforms a given compressed image to a quantized
 *               representation - takes in a compressed_img and populates a 
 *               quantized_img with each pixel containing the corresponding
 *               compressed pixel's unpacked data
 *  Parameters:  comp – the compressed image to transform
 *               methods - the methods suite used to map the quantized image
 *     Returns:  a struct containing the quantized image
 * Error cases:  checked runtime error if the given compressed image or methods
 *               is null, or if allocating memory fails for the quantized image
 */
quantized_img compressed_to_quantized(compressed_img comp, A2Methods_T methods)
{
    assert(comp != NULL);
    assert(methods != NULL);
    
    /* create a quantized image, populate the struct with relevant data */
    quantized_img quantIMG = malloc(sizeof(struct quantized_img));
    assert(quantIMG != NULL);
    quantIMG->methods = methods;
    quantIMG->width = comp->width;
    quantIMG->height = comp->height;
    
    /* create the pixel map, populate it with the compressed image's data */
    quantIMG->pixels = methods->new_with_blocksize(comp->width, comp->height,
                                     sizeof(struct quantized_pix), BLOCK_SIZE);
    assert(quantIMG->pixels != NULL);
    methods->map_block_major(quantIMG->pixels,
                             apply_compressed_to_quantized, comp);
                             
    return quantIMG;
}

/* apply_compressed_to_quantized
 *     Purpose:  apply function that populates a given quantized pixel by 
 *               unpacking the given compressed image's corresponding pixel, 
 *               which is a 32-bit word
 *  Parameters:  col, row – the column and row of the current quantized pixel
 *               compressed - the pixel map of the quantized image
 *               pixel - a pointer to the current quantized pixel
 *               compressedIMG - a pointer to the compressed image to transform
 *     Returns:  nothing
 * Error cases:  checked runtime error if the given pixel map is null or the 
 *               compressed image or its corresponding pixel/methods are null
 */
void apply_compressed_to_quantized(int col, int row, A2Methods_UArray2 quant,
                                   void *pixel, void *compressedIMG)
{
    (void) pixel;
    assert(quant != NULL);
    assert(compressedIMG != NULL);
    
    compressed_img copyCOMPRESSED = (compressed_img)compressedIMG;
    A2Methods_T methods = (A2Methods_T) copyCOMPRESSED->methods;
    assert(methods != NULL);
    assert(copyCOMPRESSED->pixels != NULL);

    uint64_t *word = methods->at(copyCOMPRESSED->pixels, col, row);
    assert(word != NULL);

    quantized_pix quantPix = compressedpix_to_quantizedpix(*word);
    assert(quantPix != NULL);
    
    quantized_pix pix = methods->at(quant, col, row);
    assert(pix != NULL);
    
    pix->a = quantPix->a;
    pix->b = quantPix->b;
    pix->c = quantPix->c;
    pix->d = quantPix->d;
    pix->pb = quantPix->pb;
    pix->pr = quantPix->pr;
    
    free(quantPix);
}

/* compressedpix_to_quantizedpix
 *     Purpose:  unpacks a given 32-bit word into a quantized pixel
 *  Parameters:  word – a 32-bit word containing the data of a quantized pixel
 *     Returns:  a struct (quantized_pix) containing a quantized pixel
 * Error cases:  checked runtime error if memory allocation of pixel fails
 */
quantized_pix compressedpix_to_quantizedpix(uint64_t word)
{
    quantized_pix newPix = malloc(sizeof(struct quantized_pix));
    assert(newPix != NULL);
    
    uint64_t a = Bitpack_getu(word, 6, 26);
    int64_t b = Bitpack_gets(word, 6, 20);
    int64_t c = Bitpack_gets(word, 6, 14);
    int64_t d = Bitpack_gets(word, 6, 8);
    uint64_t pb = Bitpack_getu(word, 4, 4);
    uint64_t pr = Bitpack_getu(word, 4, 0);
    
    newPix->a = a;
    newPix->b = b;
    newPix->c = c;
    newPix->d = d;
    newPix->pr = pr;
    newPix->pb = pb;

    return newPix;
}

/*****************************************************************************\
*                     Memory Deallocation: Free Functions                     *
\*****************************************************************************/

/* compressed_imgfree
 *     Purpose:  clears and deallocates memory from a given compressed image -
 *               including the pixel map within the struct itself
 *  Parameters:  a struct containing the compressed image and its data
 *     Returns:  nothing
 * Error cases:  checked runtime error if the given compressed image is null
 */
void compressed_imgfree(compressed_img *compressed)
{
    assert(compressed != NULL);
    assert((*compressed)->pixels != NULL);
    (*compressed)->methods->free(&(*compressed)->pixels);
}
