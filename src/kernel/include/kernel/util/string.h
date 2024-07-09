#ifndef _KERNEL_UTIL_STRING_H_
#define _KERNEL_UTIL_STRING_H_ 1
#include <kernel/types.h>



bool str_equal(const char* a,const char* b);



bool str_starts_with(const char* str,const char* prefix);



void str_copy(const char* src,char* dst,u64 max_length);



void str_copy_from_padded(const char* src,char* dst,u64 length);



void str_copy_byte_swap_from_padded(const u16* src,char* dst,u64 length);



#endif
