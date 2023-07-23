#ifndef _KERNEL_PRINT_PRINT_H_
#define _KERNEL_PRINT_PRINT_H_ 1
#include <kernel/types.h>



void print_format(const char* template,...);



void print_string(const char* str,u64 length);



void _putchar_nolock(char c);



#endif
