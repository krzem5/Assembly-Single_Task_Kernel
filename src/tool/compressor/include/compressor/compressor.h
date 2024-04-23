#ifndef _COMPRESSOR_COMPRESSOR_H_
#define _COMPRESSOR_COMPRESSOR_H_ 1
#include <stdint.h>



#define COMPRESSOR_COMPRESSION_LEVEL_NONE 0
#define COMPRESSOR_COMPRESSION_LEVEL_FAST 1
#define COMPRESSOR_COMPRESSION_LEVEL_FULL 2



uint32_t compressor_get_max_compressed_size(uint32_t data_length);



uint32_t compressor_compress(const uint8_t* data,uint32_t data_length,uint32_t compression_level,uint8_t* out);



void compressor_decompress(const void* data,uint32_t data_length,void* out);



#endif
