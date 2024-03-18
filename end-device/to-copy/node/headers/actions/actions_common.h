#ifndef ACTIONS_COMMON_H
#define ACTIONS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../../loramac/LoRaMacCrypto.h"
#include "../../loramac/LoRaMacParser.h"
#include "../../loramac/LoRaMacSerializer.h"
#include "../../loramac/utilities.h"

static void hex2bin(const char* in, size_t len, unsigned char* out) {

  static const unsigned char TBL[] = {
     0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  58,  59,  60,  61,
    62,  63,  64,  10,  11,  12,  13,  14,  15,  71,  72,  73,  74,  75,
    76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  10,  11,  12,  13,  14,  15
  };

  static const unsigned char *LOOKUP = TBL - 48;

  const char* end = in + len;

  while(in < end) *(out++) = LOOKUP[*(in++)] << 4 | LOOKUP[*(in++)];

}

/*
  in: 0xaa 0xbb 0xcc 0x11 0x22
  out:  aa   bb   cc   11   22
*/
static char* bin2hex(uint8_t *bin_arr,uint8_t size){
  char *outstr = malloc(size * 2 + 1);
  if(!outstr){
    exit(-1);
  }
  char *p = outstr;
  for(size_t i = 0; i < size; i++){
    p += sprintf(p, "%.2x", bin_arr[i]);
  }
  return outstr;
}

#endif