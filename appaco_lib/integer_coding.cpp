#include "integer_coding.h"

// length in char of an integer compacty represented
int length_encode_int(uint32_t tocode)
{
    int length=1;

    while (tocode>=128)
    {
            tocode>>=7;
        length++;
    }
    return length;
}


// encodes an integer compacty in a char table
int all_encode_int(uint32_t tocode,unsigned char *tmp)
{
  uint32_t lowmask = 127;
  int length=1;

  while (tocode>=128) {
    *(tmp++) = ((unsigned char)1 << 7) | (unsigned char)(tocode & lowmask);
    tocode>>=7;
    length++;
  }
  *tmp=tocode;

  return length;
}

// encodes an integer compacty in a char table
void encode_int(uint32_t tocode,unsigned char *tmp)
{
    uint32_t lowmask = 127;
    while (tocode>=128)
    {
        *(tmp++) = ((unsigned char)1 << 7) | (unsigned char)(tocode & lowmask);
        tocode>>=7;
    }
    *tmp=tocode;
}


// decodes an integer compacty form a char table
uint32_t length_decode_int(int* pos_code, unsigned char *tmp)
{
    unsigned char mask = 127;
    unsigned char testmask = 128;
    uint32_t tocode = 0;
    int shift=0;
    int length=1;
    /// unsigned char stmp[10]; // we use this variable in print ffor debbuging

    while (*tmp & testmask) {
      tocode|=(uint32_t)(*(tmp) & mask) << shift;

       tmp++;
      shift+=7;
      (*pos_code)++;
      length++;
    }
    tocode|=(uint32_t)(*tmp & mask) << shift;
    (*pos_code)++;

    return tocode;
}


// decodes an integer compacty in a char table
uint32_t decode_int(unsigned char *tmp)
{
    unsigned char mask = 127;
    unsigned char testmask = 128;
    uint32_t tocode = 0;
    int shift=0;

    while (*tmp & testmask)
    {
      tocode|=((uint32_t)(*(tmp++) & mask) << shift);
      shift+=7;
    }

    tocode|=(uint32_t)(*tmp & mask) << shift;

    return tocode;
}
