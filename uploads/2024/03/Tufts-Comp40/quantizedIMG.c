/*
 * quantizedIMG.c
 * By Isabelle Bain
 * March 9, 2022
 *
 * This interface ultimately contains a compression and decompression aspect,
 * and concerns itself with converting between a component video image and 
 * a quantized pixel image. There is an intermediate step between the two that
 * is a blocked pixel image, which holds all the information of a quantized 
 * image, btu before it has been quantized. Using helper and apply functions,
 * this interfact goes from a cv image to a blocked image to a quantized one, 
 * and also from a quantized image to a blocked image to a component video one
 */

#include "quantizedIMG.h"

#define BLOCK_SIZE 2

/*****************************************************************************\
*            Compression: Component Video Image to Quantized Image            *
\*****************************************************************************/

/* cv_to_quantized */
/* Purpose:     Compression step. takes a component video image and converts it
                it into a quantized image */
/* Parameters:  cv – the cv image to transform (contains dimensions and map)
                methods - the methods suite used to create and map the 
                quantized image */
/* Return:      a struct containing the quantized image pixel array and its 
                information */
/* Error Cases: checked runtime error if the given image or methods suite is 
                null, also if memory allocation fails when creating the blocked
                and component image structs and their pixel arrays */
quantized_img cv_to_quantized(cv_img cv, A2Methods_T methods)
{
    assert(cv != NULL);
    assert(methods != NULL);
    
    /* since each blocked pixel hold a 2x2 block of data, its height and width
    are half that of the original cv image */
    unsigned width = cv->width / 2;
    unsigned height = cv->height / 2;
    
    /* create a blocked image to be populated with the component video image */
    blocked_img blockIMG = malloc(sizeof(struct blocked_img));
    assert(blockIMG != NULL);
    blockIMG->methods = methods;
    blockIMG->width = width;
    blockIMG->height = height;
    blockIMG->pixels = methods->new_with_blocksize(width, height, 
                                    sizeof(struct blocked_pix), BLOCK_SIZE);
    assert(blockIMG->pixels != NULL);
    
    /* create the closure that will be  passed into the apply function, and 
    create the seqquence that it holds and populate its information */
    passIN closure = malloc(sizeof(struct passIN));
    assert(closure != NULL);
    Seq_T pix_block = Seq_new(4);
    closure->block = pix_block;
    closure->pixels = blockIMG->pixels;
    closure->methods = methods;
    
    /* populate the blocked pixel map */
    methods->map_block_major(cv->pixels, apply_cv_to_blocked, closure);
    
    
    /* create a quantized image to be populated with the blocked image */
    quantized_img quantizedIMG = malloc(sizeof(struct quantized_img));
    assert(quantizedIMG != NULL);
    quantizedIMG->methods = methods;
    quantizedIMG->width = width;
    quantizedIMG->height = height;
    
    quantizedIMG->pixels = methods->new_with_blocksize(width, height, 
                                    sizeof(struct quantized_pix), BLOCK_SIZE);
    assert(quantizedIMG->pixels != NULL);
    
    /* populate the blocked pixel map */
    methods->map_block_major(quantizedIMG->pixels, apply_blocked_to_quantized, 
                                                                blockIMG);
    
    /*free all associated memory*/
    free(closure);
    Seq_free(&pix_block);
    blocked_imgfree(&blockIMG);
    free(blockIMG);
    return quantizedIMG;
}


/* apply_cv_to_blocked */
/* Purpose:     Compression step tp be run with a mapping function. takes a 
                cv pixel and converts it into a blcoked pixel, which is then 
                stored in its equivalent location in the provided blocked 
                image pixel array */
/* Parameters:  col – the location along the x axis of the given element, as 
                        a nonnegative integer
                row - the location along the y axis of the given element, 
                        as a nonnegative integer
                cv – a A2Methods_UArray2 that contains the cv pixels to
                     convert
                pixel – a pointer to the current pixel stored at the provided
                        row and col in cv 
                passedIN – a pointer to a struct containing the blocked pixel 
                array to populate, the methods suite, and a sequence used to 
                store the cv pixels before converting into blocks */
/* Return:      a struct containing the blocked image pixel array and its 
                information */
/* Error Cases: checked runtime error if the given pixel array or methods suite
                is null, along with the pixel and strcut passed in. also
                ensures that the methods suite and pixels provided in the cl
                struct are not null */
void apply_cv_to_blocked(int col, int row, A2Methods_UArray2 cv, void *pixel, 
                         void *passedIN)
{
    assert(cv != NULL);
    assert(pixel != NULL);
    assert(passedIN != NULL);

    passIN copyPASS = (passIN)passedIN;
    A2Methods_T methods = (A2Methods_T) copyPASS->methods;
    assert(methods != NULL);
    assert(copyPASS->pixels != NULL);
    
    /* at the start of a new block, calculate the corresponding indices on the
    blocked array */
    int blockCol = (col - 1) / 2;
    int blockRow = (row - 1) / 2;
    
    /* add the current component video pixel to the sequence */
    Seq_addhi(copyPASS->block, pixel);
    
    /* once all 4 component video pixels are enqueued, use them to create a 
    blocked pixel and assign it to the previously calcuted location */
    blocked_pix blockPix;
    if (Seq_length(copyPASS->block) == 4) {
      blockPix = cvpix_to_blockedpix(copyPASS->block);
      
      blocked_pix pix = 
            ((blocked_pix)methods->at(copyPASS->pixels, blockCol, blockRow));
      
      pix->a = blockPix->a;
      pix->b = blockPix->b;
      pix->c = blockPix->c;
      pix->d = blockPix->d;
      pix->pb = blockPix->pb;
      pix->pr = blockPix->pr;
      
      /* freess the blocked pixel now that it is no longer needed */
      free(blockPix);
    }
    
}


/* apply_blocked_to_quantized */
/* Purpose:     compression step tp be run with a mapping function. takes a 
                blocked pixel and converts it into a quantized pixel, which is 
                then stored in its equivalent location in the provided 
                quantized image pixel array */
/* Parameters:  col – the location along the x axis of the given element, as 
                        a nonnegative integer
                row - the location along the y axis of the given element, 
                        as a nonnegative integer
                quant – a A2Methods_UArray2 that contains the quantized pixels 
                     to be set
                pixel – a pointer to the current pixel stored at the provided
                        row and col in quant 
                blcokedIMG – a pointer to a struct containing the blocked pixel
                array to to get the pixels to convert from */
/* Return:      a struct containing the quantized image pixel array and its 
                information */
/* Error Cases: checked runtime error if the given pixel array or methods suite
                is  null, along with the pixel and blocked image passed in. 
                also ensures that the methods suite and pixels provided in the 
                blocked image struct are not null. if any acessed pixels are 
                null, also raises a checked runtime error */
void apply_blocked_to_quantized(int col, int row, A2Methods_UArray2 quant, 
                                                void *pixel, void *blockedIMG)
{
     assert(quant != NULL);
     assert(pixel != NULL);
     assert(blockedIMG != NULL);
     (void) pixel;
     
     blocked_img copyBlock = (blocked_img) blockedIMG;
     A2Methods_T methods = (A2Methods_T) copyBlock->methods;
     assert(methods != NULL);
     assert(copyBlock->pixels != NULL);
     
     /* acesses the blocked pixel we wnat to convert */
     blocked_pix blockPix = methods->at(copyBlock->pixels, col, row);
     assert(blockPix != NULL);
     
     /* converts it into a quantized pixel */
     quantized_pix quantPix = blockedpix_to_quantpix(blockPix);
     assert(quantPix != NULL);
     
     /* acesses the location in the pixel array of the pixel we would like 
     to set */
     quantized_pix pix = methods->at(quant, col, row);
     assert(pix != NULL);
     
     /* sets the acessed pixel's value to be equal to the created pixel's */
     pix->a = quantPix->a;
     pix->b = quantPix->b;
     pix->c = quantPix->c;
     pix->d = quantPix->d;
     pix->pb = quantPix->pb;
     pix->pr = quantPix->pr;
     
     /* freess the quantized pixel now that it is no longer needed */
     free(quantPix);
}


/* cvpix_to_blockedpix */
/* Purpose:     Compression step. takes 4 cv pixels  and turns them into a 
                blocked pixel that gets returned */
/* Parameters:  pixels – a sequence of cv pixels to be converted*/
/* Return:      A single pixel of type blocked_pix */
/* Error Cases: checked runtime error if the given pixel sequence is null or 
                not of length 4, and also if any of the pixels in it are null.
                Also if memory deallocation of the blocked pixel fails */
blocked_pix cvpix_to_blockedpix(Seq_T pixels)
{
    assert(pixels != NULL);
    assert(Seq_length(pixels) == 4);
    
    /* removes each pixel from the sequence an asserts that they exist */
    cv_ypbpr pix1 = (cv_ypbpr)Seq_remlo(pixels);
    cv_ypbpr pix2 = (cv_ypbpr)Seq_remlo(pixels);
    cv_ypbpr pix3 = (cv_ypbpr)Seq_remlo(pixels);
    cv_ypbpr pix4 = (cv_ypbpr)Seq_remlo(pixels);
    assert(pix1 != NULL && pix2 != NULL && pix3 != NULL && pix4 != NULL);
    
    /* creates a new blocked pixel to store the calculated information in*/
    blocked_pix newPix = malloc(sizeof(struct blocked_pix));
    assert(newPix != NULL);
    
    /* sets the created pixel's values based on the calculations done with 
    the cv pixels from the sequence */
    newPix->a = (pix4->y + pix3->y + pix2->y + pix1->y) / 4.0;
    newPix->b = (pix4->y + pix3->y - pix2->y - pix1->y) / 4.0;
    newPix->c = (pix4->y - pix3->y + pix2->y - pix1->y) / 4.0;
    newPix->d = (pix4->y - pix3->y - pix2->y + pix1->y) / 4.0;
    newPix->pb = (pix1->pb + pix2->pb + pix3->pb + pix4->pb) / 4.0;
    newPix->pr = (pix1->pr + pix2->pr + pix3->pr + pix4->pr) / 4.0;
        
    return newPix;
}


/* blockedpix_to_quantpix */
/* Purpose:     Compression step. takes a blocked pixel and converts it into a
                quantized pixel that gets returned*/
/* Parameters:  pixel – a blocked pixel to be converted*/
/* Return:      A single pixel of type blocked_pix */
/* Error Cases: checked runtime error if the given blocked pixel is null or if 
                memory deallocation of the quantized pixel fails */
quantized_pix blockedpix_to_quantpix(blocked_pix pixel)
{
    assert(pixel != NULL);
    
    /* creates a new quantized pixel tp stpre the calculated information in */
    quantized_pix newPix = malloc(sizeof(struct quantized_pix));
    assert(newPix != NULL);
    
    /* sets the new pixel's values based on quantization done on the provided
    blocked pixel */
    newPix->a = (pixel->a) * 63;
    newPix->pb = Arith40_index_of_chroma(pixel->pb);
    newPix->pr = Arith40_index_of_chroma(pixel->pr);
    
    signed b = floor((pixel->b) * 100);
    signed c = floor((pixel->c) * 100);
    signed d = floor((pixel->d) * 100);
    
    /* if the pixel's quantized values are out of the range -15 to 15, sets 
    those vales to the respective edge value */
    newPix->b = (b > 30) ? 30 : ((b < -30) ? -30 : b);
    newPix->c = (c > 30) ? 30 : ((c < -30) ? -30 : c);
    newPix->d = (d > 30) ? 30 : ((d < -30) ? -30 : d);
    
    return newPix;
}

/*****************************************************************************\
*            Deompression: Quantized Image to Component Video Image           *
\*****************************************************************************/

/* quantized_to_cv */
/* Purpose:     Decompression step. takes a quantized image and converts it
                it into a component video image*/
/* Parameters:  quantized – the quantized image to transform 
                (contains dimensions and map)
                methods - the methods suite used to create and map the 
                cv image */
/* Return:      a struct containing the component video image pixel array and 
                its information */
/* Error Cases: checked runtime error if the given image or methods suite is 
                null, also if memory allocation fails when creating the blocked
                and component image structs and their pixel arrays */
cv_img quantized_to_cv(quantized_img quantized, A2Methods_T methods)
{
    
    assert(quantized != NULL);
    assert(methods != NULL);
    
    unsigned width = quantized->width;
    unsigned height = quantized->height;
    
    /* create the blocked image to be populated */
    blocked_img blockIMG = malloc(sizeof(struct blocked_img));
    assert(blockIMG != NULL);
    blockIMG->methods = methods;
    blockIMG->width = width;
    blockIMG->height = height;
    blockIMG->pixels = methods->new_with_blocksize(width, height, 
                                    sizeof(struct blocked_pix), BLOCK_SIZE);
    assert(blockIMG->pixels != NULL);
    
    /* populate the blocked pixel map with the quantized image */
    methods->map_block_major(blockIMG->pixels, apply_quantized_to_block, 
                                                                quantized);
    
    /* create a component video image */
    cv_img componentIMG = malloc(sizeof(struct cv_img));
    assert(componentIMG != NULL);
    componentIMG->methods = methods;
    componentIMG->width = width * 2;
    componentIMG->height = height * 2;
    componentIMG->pixels = methods->new_with_blocksize(width * 2, height * 2, 
                                        sizeof(struct cv_ypbpr), BLOCK_SIZE);
    assert(componentIMG->pixels != NULL);
    
    /* populate the component video pixel map with the blocked image */
    methods->map_block_major(blockIMG->pixels, apply_blocked_to_cv, 
                             componentIMG);
    
    /* free the blocked image now that it is no longer needed */
    blocked_imgfree(&blockIMG);
    free(blockIMG);
    
    return componentIMG;
}

/* apply_blocked_to_cv */
/* Purpose:     Decompression step tp be run with a mapping function. takes a 
                blocked pixel and converts it into a sequence of cv pixels, 
                which are then stored in their equivalent location in the 
                provided quantized image pixel array */
/* Parameters:  col – the location along the x axis of the given element, as 
                        a nonnegative integer
                row - the location along the y axis of the given element, 
                        as a nonnegative integer
                blocked – a A2Methods_UArray2 that contains the blocked pixels 
                     to be used for the conversion
                pixel – a pointer to the current pixel stored at the provided
                        row and col in blocked 
                componentIMG – a pointer to a struct containing the cv pixel
                array to be populated */
/* Return:      a struct containing the cv image pixel array and its 
                information */
/* Error Cases: checked runtime error if the given pixel array or methods suite
                is null, along with the pixel and cv image passed in. 
                also ensures that the methods suite and pixels provided in the 
                component image struct are not null. if any acessed pixels 
                are null, also raises a checked runtime error */
void apply_blocked_to_cv(int col, int row, A2Methods_UArray2 blocked, 
                                            void *pixel, void *componentIMG)
{
    assert(blocked != NULL);
    assert(componentIMG != NULL);
    
    cv_img copyCV = (cv_img)componentIMG;
    A2Methods_T methods = (A2Methods_T) copyCV->methods;
    assert(methods != NULL);
    assert(copyCV->pixels != NULL);

    /* creates a blocked pixel that holds the information to be converted */
    blocked_pix copyPix = (blocked_pix) pixel;
    
    /* using that blocked pixel, create a sequence of 4 cv pixels */
    Seq_T cv_pixels = blockedpix_to_cvpix(copyPix);
    
    /* access the 4 pixels just created and ensure they are not null */
    cv_ypbpr pix1 = (cv_ypbpr)Seq_remlo(cv_pixels);
    cv_ypbpr pix2 = (cv_ypbpr)Seq_remlo(cv_pixels);
    cv_ypbpr pix3 = (cv_ypbpr)Seq_remlo(cv_pixels);
    cv_ypbpr pix4 = (cv_ypbpr)Seq_remlo(cv_pixels);
    assert(pix1 != NULL && pix2 != NULL && pix3 != NULL && pix4 != NULL);
    
    /* access the cv pixel at the first desired location in the cv image pixel
    array and set its information to be that same as the first created pixel*/
    cv_ypbpr copyPix1 = 
    (cv_ypbpr)methods->at(copyCV->pixels, col * 2, row * 2);
    assert(copyPix1 != NULL);
    copyPix1->y = pix1->y;
    copyPix1->pb = pix1->pb;
    copyPix1->pr = pix1->pr;
    
    /* access the cv pixel at the second desired location in the cv image pixel
    array and set its information to be that same as the second created pixel*/
    cv_ypbpr copyPix2 = 
    (cv_ypbpr)methods->at(copyCV->pixels, col * 2, (row * 2) + 1); 
    assert(copyPix2 != NULL);
    copyPix2->y = pix2->y;
    copyPix2->pb = pix2->pb;
    copyPix2->pr = pix2->pr;
    
    /* access the cv pixel at the third desired location in the cv image pixel
    array and set its information to be that same as the third created pixel*/
    cv_ypbpr copyPix3 = 
    (cv_ypbpr)methods->at(copyCV->pixels, (col * 2) + 1, row * 2); 
    assert(copyPix3 != NULL);
    copyPix3->y = pix3->y;
    copyPix3->pb = pix3->pb;
    copyPix3->pr = pix3->pr;
    
    /* access the cv pixel at the fourth desired location in the cv image pixel
    array and set its information to be that same as the fourth created pixel*/
    cv_ypbpr copyPix4 = 
    (cv_ypbpr)methods->at(copyCV->pixels, (col * 2) + 1, (row * 2) + 1); 
    assert(copyPix4 != NULL);
    copyPix4->y = pix4->y;
    copyPix4->pb = pix4->pb;
    copyPix4->pr = pix4->pr;
    
    /* free the created sequence and its 4 cv pixels now that their information
    has been transfered*/
    Seq_free(&cv_pixels);   
    free(pix1);
    free(pix2);
    free(pix3);
    free(pix4);
}


/* blockedpix_to_cvpix */
/* Purpose:     Decompression step. takes a single blocked pixle and turns it
                into a sequence of cv pixels that gets returned */
/* Parameters:  pixel – a blocked pixel*/
/* Return:      A sequence of pixels of type cv_ypbpr */
/* Error Cases: checked runtime error if the given block pixel is null or if 
                memory deallocation of the sequence or any of the 
                individual cv pixels fail. Also if the sequence is not of 
                length 4 after is is created */
Seq_T blockedpix_to_cvpix(blocked_pix block)
{
    assert(block != NULL);
    
    /* create a sequence that will hold the 4 cv pixel to be returned*/
    Seq_T pix_block = Seq_new(4);
    assert(pix_block != NULL);
    
    /* access the information inside the provided blocked pixel*/
    float a = block->a;
    float b = block->b;
    float c = block->c;
    float d = block->d;
    float pr = block->pr;
    float pb = block->pb;
    
    /* create the 4 cv pixels to be set and ensure none of them are null*/
    cv_ypbpr pix1 = malloc(sizeof(struct cv_ypbpr));
    cv_ypbpr pix2 = malloc(sizeof(struct cv_ypbpr));
    cv_ypbpr pix3 = malloc(sizeof(struct cv_ypbpr));
    cv_ypbpr pix4 = malloc(sizeof(struct cv_ypbpr));
    assert(pix1 != NULL && pix2 != NULL && pix3 != NULL && pix4 != NULL);
    
    /* set the first pixel based on calculations done with the extracted 
    information from the blocked pixel*/
    pix1->y = (a - b - c + d);
    pix1->pr = pr;
    pix1->pb = pb;
    
    /* set the second pixel based on calculations done with the extracted 
    information from the blocked pixel*/
    pix2->y = (a - b + c - d);
    pix2->pr = pr;
    pix2->pb = pb;
    
    /* set the third pixel based on calculations done with the extracted 
    information from the blocked pixel*/
    pix3->y = (a + b - c - d);
    pix3->pr = pr;
    pix3->pb = pb;
    
    /* set the fourth pixel based on calculations done with the extracted 
    information from the blocked pixel*/
    pix4->y = (a + b + c + d);
    pix4->pr = pr;
    pix4->pb = pb;
    
    /* adds the 4 pixels to the created sequence */
    Seq_addhi(pix_block, pix1);
    Seq_addhi(pix_block, pix2);
    Seq_addhi(pix_block, pix3);
    Seq_addhi(pix_block, pix4);
    
    /* ensures that the sequence holds all 4 pixels */
    assert(Seq_length(pix_block) == 4);
        
    return pix_block;
}


/* quantpix_to_blockedpix */
/* Purpose:     Decompression step. takes a quantized pixel and converts it 
                into a blocked pixel that gets returned*/
/* Parameters:  pixel – a quantized pixel to be converted*/
/* Return:      A single pixel of type blocked_pix */
/* Error Cases: checked runtime error if the given quantized pixel is null or 
                if  memory deallocation of the blocked pixel fails */
blocked_pix quantpix_to_blockedpix(quantized_pix pixel)
{
    assert(pixel != NULL);
    
    /* create a new blocked pixel to be set */
    blocked_pix newPix = malloc(sizeof(struct blocked_pix));
    assert(newPix != NULL);
    
    /* sets the new pixel's values based on reversing the quantization done 
    on the provided quantized pixel */
    newPix->a = pixel->a / 63.0;
    newPix->b = pixel->b / 100.0;
    newPix->c = pixel->c / 100.0;
    newPix->d = pixel->d / 100.0;
    newPix->pb = Arith40_chroma_of_index(pixel->pb);
    newPix->pr = Arith40_chroma_of_index(pixel->pr);
    
    return newPix;
}

/* apply_quantized_to_blocked */
/* Purpose:     Decompression step to be run with a mapping function. takes a 
                quantized pixel and converts it into a blocked pixel, which is
                then stored in its equivalent location in the provided blocked
                image pixel array */
/* Parameters:  col – the location along the x axis of the given element, as 
                        a nonnegative integer
                row - the location along the y axis of the given element, 
                        as a nonnegative integer
                blocks – a A2Methods_UArray2 that contains the blocked pixels 
                     to be set
                pixel – a pointer to the current pixel stored at the provided
                        row and col in blocks 
                quantizedIMG – a pointer to a struct containing the quantized
                pixel array to be converted */
/* Return:      a struct containing the blocked image pixel array and its 
                information */
/* Error Cases: checked runtime error if the given pixel array or methods suite
                is null, along with the pixel and quantized image passed in. 
                also ensures that the methods suite and pixels provided in the 
                quantized image struct are not null. if any acessed pixels 
                are null, also raises a checked runtime error */
void apply_quantized_to_block(int col, int row, A2Methods_UArray2 blocks, 
                                            void *pixel, void *quantizedIMG)
{
    assert(blocks != NULL);
    assert(quantizedIMG != NULL);
    (void) pixel;
    
    quantized_img copyQuant = (quantized_img) quantizedIMG;
    A2Methods_T methods = (A2Methods_T) copyQuant->methods;
    assert(methods != NULL);
    assert(copyQuant->pixels != NULL);
    
    /* access the quantized pixel to be converted */
    quantized_pix quantPix = methods->at(copyQuant->pixels, col, row);
    assert(quantPix != NULL);
    
    /* create a new blocked pixel using the quantized one */
    blocked_pix blockPix = quantpix_to_blockedpix(quantPix);
    
    /* access the blocked pixel at the location we want to change */
    blocked_pix pix = methods->at(blocks, col, row);
    assert(pix != NULL);
    
    /* set the accessed pixel's data to be equal to the created pixel's data */
    pix->a = blockPix->a;
    pix->b = blockPix->b;
    pix->c = blockPix->c;
    pix->d = blockPix->d;
    pix->pb = blockPix->pb;
    pix->pr = blockPix->pr;
    
    /* free the blocked pixel we created */
    free(blockPix);
}

/*****************************************************************************\
*                     Memory Deallocation: Free Functions                     *
\*****************************************************************************/

/* blocked_imgfree */
/* Purpose:     Freeing step. takes a blocked image and frees the space 
                malloced for each pixel it holds*/
/* Parameters:  blcok – a blocked image */
/* Return:      nothing */
/* Error Cases: checked runtime error if the given blocked image is null or 
                if its pixel array is null */
void blocked_imgfree(blocked_img *block)
{
    assert(block != NULL);
    assert((*block)->pixels != NULL);
    
    /* use the methods suite's free to free all the pixels in block */
    (*block)->methods->free(&(*block)->pixels);
}


/* quantized_imgfree */
/* Purpose:     Freeing step. takes a quanitzed image and frees the space 
                malloced for each pixel it holds*/
/* Parameters:  quantized – a quantized image */
/* Return:      nothing */
/* Error Cases: checked runtime error if the given quantized image is null or 
                if its pixel array is null */
void quantized_imgfree(quantized_img *quantized)
{
    assert(quantized != NULL);
    assert((*quantized)->pixels != NULL);
    
    /* use the methods suite's free to free all the pixels in quantized */
    (*quantized)->methods->free(&(*quantized)->pixels);
}

