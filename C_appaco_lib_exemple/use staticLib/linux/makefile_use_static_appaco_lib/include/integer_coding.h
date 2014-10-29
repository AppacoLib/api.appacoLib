#ifndef INTEGER_CODING_H
#define INTEGER_CODING_H


/*****************************************************************/
/****  Integer coding/decoding in a series of char(s)       ******/
/*****************************************************************/


#include <stdint.h>
#include <stdio.h>


// length in char of an integer compacty represented
int length_encode_int(uint32_t tocode);

// encodes an integer compacty in a char table
void encode_int(uint32_t tocode,unsigned char *tmp);

// encodes an integer compacty in a char table
int all_encode_int(uint32_t tocode,unsigned char *tmp);

// decodes an integer compacty in a char table
uint32_t length_decode_int(int* pos_code, unsigned char *tmp);

// decodes an integer compacty in a char table
uint32_t decode_int(unsigned char *tmp);

#endif // INTEGER_CODING_H
