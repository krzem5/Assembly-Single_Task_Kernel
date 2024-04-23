#ifndef _COMMON_COMPRESSOR_COMPRESSOR_H_
#define _COMMON_COMPRESSOR_COMPRESSOR_H_ 1



#define COMPRESSOR_COMPRESSION_LEVEL_NONE 0
#define COMPRESSOR_COMPRESSION_LEVEL_FAST 1
#define COMPRESSOR_COMPRESSION_LEVEL_FULL 2



unsigned int compressor_get_max_compressed_size(unsigned int data_length);



unsigned int compressor_compress(const unsigned char* data,unsigned int data_length,unsigned int compression_level,unsigned char* out);



void compressor_decompress(const void* data,unsigned int data_length,void* out);



#endif
