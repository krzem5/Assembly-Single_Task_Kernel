#ifndef _COMMON_COMPRESSOR_COMPRESSOR_H_
#define _COMMON_COMPRESSOR_COMPRESSOR_H_ 1
#include <common/types.h>
#if BUILD_MODULE
#include <kernel/writer/writer.h>
#else
#include <stdio.h>
#endif



#define COMPRESSOR_COMPRESSION_LEVEL_NONE 0
#define COMPRESSOR_COMPRESSION_LEVEL_FAST 1
#define COMPRESSOR_COMPRESSION_LEVEL_FULL 2



typedef struct _COMPRESSOR_OUTPUT{
#if BUILD_MODULE
	writer_t* writer;
#else
	FILE* file;
#endif
} compressor_output_t;



void compressor_compress(const u8* data,u32 data_length,u32 compression_level,compressor_output_t* out);



void compressor_decompress(const void* data,u32 data_length,void* out);



#endif
