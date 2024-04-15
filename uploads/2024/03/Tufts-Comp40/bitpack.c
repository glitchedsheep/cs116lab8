/*
 * bitpack.c
 * By Isabelle Bain
 * March 9, 2022
 *
 * This program allows to user to interface with and manipulate fields within
 * words. It enables the user to create words, access fields within them, check
 * the size of a word or field, and has a smart shift left and right function.
 * Every function has a signed and unsigned version.
 */

#include "bitpack.h"
#include "assert.h"
#include "except.h"

#define WIDTH_MAX 64

Except_T Bitpack_Overflow = { "Overflow packing bits" };

/* shift_leftu
 *     Purpose:  shifts a given unsigned value left by a given number of times
 *  Parameters:  n - the unsigned value to shift, width - the shift size
 *     Returns:  the new shifted value, unsigned
 */
uint64_t shift_leftu(uint64_t n, unsigned width)
{
    /* if width is more than 64, we want to clear the provided word */
    if (width >= WIDTH_MAX) {
        return 0;
    }
    
    /* otherwise, shift it left by the desired amount */
    uint64_t shifted = n << width;
    
    return shifted;
}

/* shift_rightu
 *     Purpose:  shifts a given unsigned value right by a given number of times
 *  Parameters:  n - the unsigned value to shift, width - the shift size
 *     Returns:  the new shifted value, unsigned
 */
uint64_t shift_rightu(uint64_t n, unsigned width)
{
    /* if width is more than 64, we want to clear the provided word */
    if (width >= WIDTH_MAX) {
        return 0;
    }
    
    /* otherwise, shift it right by the desired amount */
    uint64_t shifted = n >> width;
    
    return shifted;
}

/* shift_lefts
 *     Purpose:  shifts a given signed value left by a given number of times
 *  Parameters:  n - the signed value to shift, width - the shift size
 *     Returns:  the new shifted value, signed
 */
int64_t shift_lefts(int64_t n, unsigned width)
{
    /* if width is more than 64, we want to clear the provided word */
    if (width >= WIDTH_MAX) {
        return 0;
    }
    
    /* otherwise, shift it left the desire amount */
    int64_t shifted = n << width;

    return shifted;
}

/* shift_rights
 *     Purpose:  shifts a given signed value right by a given number of times
 *  Parameters:  n - the signed value to shift, width - the shift size
 *     Returns:  the new shifted value, signed
 */
int64_t shift_rights(int64_t n, unsigned width)
{
    /* if width is more than 64, we want to return -1 */
    if (width >= WIDTH_MAX) {
        return -1;
    }
    
    /* otherwise, shift it right by the desired amount */
    int64_t shifted = n >> width;
    
    return shifted;
}

/* Bitpack_fitsu
 *     Purpose:  determines whether a given unsigned value would fit in a given
 *               number of bits
 *  Parameters:  n - the unsigned value, width - the number of bits available
 *     Returns:  true if n can fit in width bits, false if it can't
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    /* if width is over 64, we know anything will fit in it */
    /* if width is zero, the only value it can hold is zero */
    if (width >= WIDTH_MAX) {
      return true;
    } else if (width == 0) { 
      if (n == 0) {
        return true;
      } else {
        return false;
      }
    }
    
    /* shift a mask of 1's left by width -1 and invert it to find the largest
    value that many bits can hold*/
    uint64_t to_shift = ~0;
    to_shift = shift_leftu(to_shift, width);
    uint64_t max_val = ~to_shift;
    
    /* if n is less than or equal to it, n can be stored in width bits */
    return (n <= max_val);
}

/* Bitpack_fitss
 *     Purpose:  determines whether a given signed value would fit in a given 
 *               number of bits
 *  Parameters:  n - the signed value, width - the number of bits available
 *     Returns:  true if n can fit in width bits, false if it can't
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    /* if width is over 64, we know anything will fit in it */
    /* if width is zero, the only value it can hold is zero */
    if (width >= WIDTH_MAX) {
      return true;
    } else if (width == 0) { 
      if (n == 0) {
        return true;
      } else {
        return false;
      }
    }
    
    /* shift a mask of 1's left by width -1 to find the smallest number 
    (largest negative) it can hold */
    /* invert that minumum to find the largets value that many bits can hold*/
    uint64_t to_shift = ~0;
    int64_t min_val = shift_lefts(to_shift, width - 1);
    int64_t max_val = ~min_val;
    
    /* if n is within the range between them, it can be stored in width bits */
    return (n >= min_val && n <= max_val);
}

/* Bitpack_getu
 *     Purpose:  unpacks a desired unsigned value from a specified location in 
 *               a 64-bit word
 *  Parameters:  word - the 64-bit word to unpack
 *               width - the number of bits to retrieve
 *               lsb - the least significant bit of the desired value
 *     Returns:  a 64-bit unsigned word containing the desired value
 * Error cases:  checked runtime error if the width is over 64 bits or if the 
 *               width is longer than the word given the least significant bit
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= WIDTH_MAX);
    assert(width + lsb <= WIDTH_MAX);
    
    /* shift the word left until the desired data is in the left most position
    so that the rest of the info before it gets cleared */
    uint64_t data = shift_leftu(word, (WIDTH_MAX - width - lsb));
    
    /* now shift it right until the data is in the rigth most position so that
    the rest of the data behind it gets cleared and we are left with only the 
    desired data*/
    data = shift_rightu(data, (WIDTH_MAX - width));
    
    return data;
}

/* Bitpack_gets
 *     Purpose:  unpacks a desired signed value from a specified location in a
 *               64-bit word
 *  Parameters:  word - the 64-bit word to unpack
 *               width - the number of bits to retrieve
 *               lsb - the least significant bit of the desired value
 *     Returns:  a 64-bit signed word containing the desired value
 * Error cases:  checked runtime error if the width is over 64 bits or if the 
 *               width is longer than the word given the least significant bit
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= WIDTH_MAX);
    assert(width + lsb <= WIDTH_MAX);

    /* shift the word left until the desired data is in the left most position
    so that the rest of the info before it gets cleared */
    int64_t data = shift_lefts(word, (WIDTH_MAX - width - lsb));

    /* now shift it right until the data is in the rigth most position so that
    the rest of the data behind it gets cleared and we are left with only the 
    desired data*/
    data = shift_rights(data, (WIDTH_MAX - width));

    return data;
}

/* Bitpack_newu
 *     Purpose:  updates a given word by packing a given unsigned value into a
 *               specified location and returning the resulting word
 *  Parameters:  word - the 64-bit word to pack
 *               width - the number of bits available to the value
 *               lsb - the least significant bit of the given value
 *               value - the unsigned value to pack into the word
 *     Returns:  the 64-bit word now packed with the value in the desired spot
 * Error cases:  checked runtime error if the width is over 64 bits or if the 
 *               width is longer than the word given the least significant bit,
 *               exception if the value can't fit into the given width
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
    assert(width <= WIDTH_MAX);
    assert(width + lsb <= WIDTH_MAX);
    
    /* make sure the new value fits in the given space */
    if (Bitpack_fitsu(value, width) == false) {
      RAISE(Bitpack_Overflow);
    }
    
    /* clear any data currently in the designated spot */
    uint64_t clear = ~0;
    clear = shift_leftu(clear, (WIDTH_MAX - width));
    clear = shift_rightu(clear, (WIDTH_MAX - width - lsb));
    clear = ~clear;
    
    word = word & clear;
    
    /* create a mask where value is in the right spot */
    value = shift_leftu(value, lsb);
    
    /* add the desired value to word using the above mask */
    word = word | value;

    return word;
}

/* Bitpack_news
 *     Purpose:  updates a given word by packing a given signed value into a
 *               specified location and returning the resulting word
 *  Parameters:  word - the 64-bit word to pack
 *               width - the number of bits available to the value
 *               lsb - the least significant bit of the given value
 *               value - the signed value to pack into the word
 *     Returns:  the 64-bit word now packed with the value in the desired spot
 * Error cases:  checked runtime error if the width is over 64 bits or if the 
 *               width is longer than the word given the least significant bit,
 *               exception if the value can't fit into the given width
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                      int64_t value)
{
    assert(width <= WIDTH_MAX);
    assert(width + lsb <= WIDTH_MAX);

    /* make sure the new value fits in the given space */
    if (Bitpack_fitss(value, width) == false) {
      RAISE(Bitpack_Overflow);
    }
    
    /* clear any data currently in the designated spot */
    uint64_t clear = ~0;
    clear = shift_leftu(clear, (WIDTH_MAX - width));
    clear = shift_rightu(clear, (WIDTH_MAX - width - lsb));
    clear = ~clear;
    
    word = word & clear;
    
    /* create a mask where value is in the right spot */
    value = shift_lefts(value, lsb);
    clear = ~clear;
    value = value & clear;
    
    /* add the desired value to word using the above mask */
    word = word | value;

    return word;
}
