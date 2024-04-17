#ifndef _COMPRESSOR_COMPRESSOR_H_
#define _COMPRESSOR_COMPRESSOR_H_ 1
#include <stdint.h>
#include <stdio.h>



#define COMPRESSOR_COMPRESSION_LEVEL_NONE 0
#define COMPRESSOR_COMPRESSION_LEVEL_FAST 1
#define COMPRESSOR_COMPRESSION_LEVEL_FULL 2



_Bool compressor_compress(FILE* in,uint32_t compression_level,FILE* out);



#endif
