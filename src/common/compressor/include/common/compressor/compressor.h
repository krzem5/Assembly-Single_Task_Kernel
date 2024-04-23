#ifndef _COMMON_COMPRESSOR_COMPRESSOR_H_
#define _COMMON_COMPRESSOR_COMPRESSOR_H_ 1
#include <common/types.h>



#define COMPRESSOR_COMPRESSION_LEVEL_NONE 0
#define COMPRESSOR_COMPRESSION_LEVEL_FAST 1
#define COMPRESSOR_COMPRESSION_LEVEL_FULL 2



u32 compressor_get_max_compressed_size(u32 data_length);



u32 compressor_compress(const u8* data,u32 data_length,u32 compression_level,u8* out);



void compressor_decompress(const void* data,u32 data_length,void* out);



#endif
