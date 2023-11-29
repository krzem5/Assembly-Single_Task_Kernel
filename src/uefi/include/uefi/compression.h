#ifndef _UEFI_COMPRESSION_H_
#define _UEFI_COMPRESSION_H_ 1
#include <stdint.h>



void _decompress_raw(const uint8_t* data,uint32_t data_length,uint8_t* out);



#endif
