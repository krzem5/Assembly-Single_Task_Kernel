#ifndef _KERNEL_FORMAT_FORMAT_H_
#define _KERNEL_FORMAT_FORMAT_H_ 1
#include <kernel/types.h>



u32 format_string(char* buffer,u32 length,const char* template,...);



u32 format_string_va(char* buffer,u32 length,const char* template,__builtin_va_list* va);



#endif
